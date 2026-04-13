#include "hal/display.h"
#include "hal/shift.h"
#include "mcal/SysTick.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_STR_LEN 50
// Whitespace padding after string finishes cycling.
// For example if cycling string "12345" if we had no padding at some
// point the cycle would look like "3451", with 2 padding it would look
// like "345 ", "45  ", "5  1", "  12", " 123", "1234", etc
#define STR_PADDING 2

// TODO: Remember to fix masks if we change physical connections.
// NOTE: Asuming the shift registers 0-7 are for the display segments, 8-9 for display digit and 10-11 for leds.
// #define DISPLAY_MASK 0x03FF // 0b0000'0011'1111'1111
#define SEG_MASK 0x00FF  // 0b0000'0000'1111'1111
#define DIGIT_MASK 0x300 // 0b0000'0011'0000'0000
#define LEDS_MASK 0xC00  // 0b0000'1100'0000'0000
#define DIGIT_SHIFT 8
#define LEDS_SHIFT 10

#define DISPLAY_UPDATE_RATE 2
#define DISPLAY_SLIDE_AND_BLINK_RATE 2000

static uint8_t digits[DIGITS] = {0};
static uint8_t displayString[MAX_STR_LEN + STR_PADDING] = {0};
static uint8_t displayStringLen = 0;
static uint8_t dutyPercentage = 100;

static bool digitBlink[DIGITS] = {0};
static uint8_t blinkBuf[DIGITS] = {0};

static bool leds[LED_COUNT] = {0};

static void updateDisplay();
static void updateDisplayRegisters(uint8_t segments, uint8_t digit, uint8_t led);
static void updateSlideAndBlink();

void initDisplay() {
  pisr_register(updateDisplay, DISPLAY_UPDATE_RATE);
  pisr_register(updateSlideAndBlink, DISPLAY_SLIDE_AND_BLINK_RATE);
  shiftInit();
}

// TODO: replace for `increseDutyPercentage` and `decreaseDutyPercentage`.
uint8_t getDutyPercentage() {
  return dutyPercentage;
}
void setDutyPercentage(uint8_t percentage) {
  if (0 < percentage && percentage <= 100) dutyPercentage = percentage;
}

void turnOnDisplayLed(uint8_t idx) {
  if (idx >= LED_COUNT) return;
  leds[idx] = true;
}

void turnOffDisplayLed(uint8_t idx) {
  if (idx >= LED_COUNT) return;
  leds[idx] = false;
}

void enableDot(uint8_t digit, bool value) {
  if (value) {
    digits[digit] |= 0b10000000;
  } else {
    digits[digit] &= 0b01111111;
  }
}

bool writeSegments(uint8_t segments, uint8_t digit) {
  if (digit >= DIGITS) return false;
  digits[digit] = segments;
  return true;
}

static uint8_t char2segments[] = {
    [0] = 0b00000000,

    ['0'] = 0b00111111, ['1'] = 0b00000110, ['2'] = 0b01011011, ['3'] = 0b01001111, ['4'] = 0b01100110,
    ['5'] = 0b01101101, ['6'] = 0b01111101, ['7'] = 0b00000111, ['8'] = 0b01111111, ['9'] = 0b01101111,

    ['A'] = 0b01110111, ['B'] = 0b01111100, ['C'] = 0b00111001, ['D'] = 0b01011110, ['E'] = 0b01111001,
    ['F'] = 0b01110001, ['G'] = 0b00111101, ['H'] = 0b01110110, ['I'] = 0b00000110, ['J'] = 0b00011110,
    ['K'] = 0b01110110, ['L'] = 0b00111000, ['M'] = 0b00010101, ['N'] = 0b00110111, ['O'] = 0b00111111,
    ['P'] = 0b01110011, ['Q'] = 0b01100111, ['R'] = 0b00110001, ['S'] = 0b01101101, ['T'] = 0b01111000,
    ['U'] = 0b00111110, ['V'] = 0b00111110, ['W'] = 0b00101010, ['X'] = 0b01110110, ['Y'] = 0b01101110,
    ['Z'] = 0b01011011,

    [' '] = 0b00000000, ['-'] = 0b01000000, ['_'] = 0b00001000, ['.'] = 0b10000000,
};

bool writeCharacter(char character, uint8_t digit, bool blink) {
  digitBlink[digit] = blink;
  return writeSegments(char2segments[character], digit);
}

// Writes number ands slides number if needed
void writeString(const char* string) {
  uint8_t i = 0;
  for (; i < MAX_STR_LEN && string[i] != 0; ++i) {
    displayString[i] = char2segments[(uint8_t)string[i]];
  }
  displayStringLen = i;

  for (uint8_t j = i; j < i + STR_PADDING; ++j) {
    displayString[j] = char2segments[' '];
  }

  for (uint8_t j = 0; j < DIGITS; ++j) {
    digits[j] = displayString[j];
  }
}

void cleanDisplay() {
  for (uint8_t i = 0; i < DIGITS; ++i) {
    digits[i] = 0;
    digitBlink[i] = false;
  }
  displayStringLen = 0;

  for (uint8_t i = 0; i < LED_COUNT; ++i) leds[i] = false;
}

// Private functions

static void updateDisplay() {
  static uint8_t currentDigit = 0;
  static uint8_t duty[DIGITS] = {0};
  static uint8_t currentLed = 0;

  uint8_t xsegments = 0;
  if (digitBlink[currentDigit]) {
    xsegments = digits[currentDigit] & blinkBuf[currentDigit];
  } else {
    xsegments = digits[currentDigit];
  }
  uint8_t segments = -(duty[currentDigit] < dutyPercentage) & xsegments;
  updateDisplayRegisters(segments, currentDigit, currentLed);
  duty[currentDigit] = (duty[currentDigit] + 10) % 100;
  currentDigit = (currentDigit + 1) % DIGITS; // cicleo por los digitos
  currentLed = (currentLed + 1) % LED_COUNT;
}

static void updateDisplayRegisters(uint8_t segments, uint8_t digit, uint8_t led) {
  uint16_t shiftRegister = 0;
  shiftRegister = (segments & SEG_MASK) | ((digit << DIGIT_SHIFT) & DIGIT_MASK);
  if (leds[led]) shiftRegister |= ((led + 1) << LEDS_SHIFT) & LEDS_MASK;

  shiftWriteWord(shiftRegister);
  shiftOutUpdate();
}

static void updateSlideAndBlink() {
  static uint8_t slideIdx = 0;

  if (displayStringLen == 0) {
    for (int i = 0; i < DIGITS; ++i) {
      if (digitBlink[i] && !blinkBuf[i]) {
        blinkBuf[i] = digits[i];
      } else if (digitBlink[i] && blinkBuf[i]) {
        blinkBuf[i] = 0;
      }
    }
  } else if (displayStringLen <= DIGITS) {
    for (uint8_t i = 0; i < displayStringLen; ++i) {
      digits[i] = displayString[i];
    }
  } else {
    uint8_t lenWithPadding = displayStringLen + STR_PADDING;
    for (uint8_t i = slideIdx, j = 0; j < DIGITS; i = (i + 1) % lenWithPadding, ++j) {
      digits[j] = displayString[i];
    }

    slideIdx = (slideIdx + 1) % lenWithPadding;
  }
}
