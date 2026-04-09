#include "hal/display.h"
#include "hal/shift.h"
#include "mcal/SysTick.h"
#include <stdbool.h>
#include <stdint.h>

#define DIGITS 4
#define MAX_STR_LEN 50

// NOTE: Asuming the shift registers 0-7 are for the display segments and 8-9 for display digit.
#define SEG_MASK 0x00FF     // 0b0000'0000'1111'1111
#define DIGIT_MASK 0x300    // 0b0000'0011'0000'0000
#define DISPLAY_MASK 0x03FF // 0b0000'0011'1111'1111

static uint8_t digits[DIGITS] = {0};
static uint8_t displayString[MAX_STR_LEN] = {0};
static uint8_t dutyPercentage = 100;

void updateDisplay();
void updateDisplayRegisters(uint8_t segments, uint8_t digit);

void initDisplay() {
  // SysTick_Init(1000);
  pisr_register(updateDisplay, 10);
  // need another callback that blinks and slides.
}

void setDutyPercentage(uint8_t percentage) {
  if (0 < percentage && percentage <= 25) dutyPercentage = percentage;
}

bool writeSegments(uint8_t segments, uint8_t digit) {
  if (digit > 3) return false;
  digits[digit] = segments;
  return true;
}

static uint8_t number2segments[10] = {
    0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
    0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111,
};

bool writeNumber(uint8_t number, uint8_t digit) {
  return writeSegments(number2segments[number], digit);
}

// Writes number ands slides number if needed
bool writeString(const char* string, uint8_t slideDelay) {
  for (int i = 0; string[i] != 0 && i < MAX_STR_LEN; ++i) {
    /* displayString */
  }
}

void cleanDisplay() {
  for (int i = 0; i < DIGITS; ++i) digits[i] = 0;
}

// Private functions

void updateDisplay() {
  static uint8_t currentDigit = 0;
  static uint8_t duty = 0;

  uint8_t segments = -(duty < dutyPercentage) & digits[currentDigit];
  updateDisplayRegisters(segments, currentDigit);
  duty = (duty + 1) % 100;
  currentDigit = (currentDigit + 1) % DIGITS;
}

void updateDisplayRegisters(uint8_t segments, uint8_t digit) {
  uint16_t shiftRegister = shiftRead();
  shiftRegister = (shiftRegister & ~DISPLAY_MASK) | (segments & SEG_MASK) | ((digit << 8) & DIGIT_MASK);
  shiftWriteWord(shiftRegister);
  shiftOutUpdate();
}
