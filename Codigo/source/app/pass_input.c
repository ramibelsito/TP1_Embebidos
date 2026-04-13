#include "pass_input.h"
#include "app/utils.h"
#include "hal/display.h"
#include "hal/wheel.h"
#include <stdint.h>

typedef struct pass_input_t {
  char buf[PASS_LEN];
  uint8_t bufIdx;
  uint8_t bufLen;
  char display[DIGITS];
  char hiddenBuf[PASS_LEN];
} pass_input_t;

static pass_input_t input = {.buf = {0}, .bufIdx = 0, .bufLen = 0, .display = "-   ", .hiddenBuf = "-_-_-"};
static PassInputState state = PASS_EDIT;
static bool passChange = false;

static void handlePassEdit(char* pass, wheel_input_t wheelResult, bool* fullPass);
static void handlePassSelectDigit(wheel_input_t wheelResult);
static void updateDisplay();

void initPassInput() {
  input = (pass_input_t){.buf = {0}, .bufIdx = 0, .display = "-   ", .hiddenBuf = "-_-_-"};
  state = PASS_EDIT;
  passChange = false;
  cleanDisplay();
}

PassInputState handlePassInput(char* pass, wheel_input_t wheelResult, bool* fullPass) {
  for (uint8_t i = 0; i < DIGITS; ++i) {
    bool isCursor = (i == min(DIGITS - 1, input.bufIdx));
    bool blink = (state == PASS_EDIT) && isCursor;
    writeCharacter(input.display[i], i, blink);
    bool dot = (state == PASS_SELECT_DIGIT) && isCursor;
    enableDot(i, dot);
  }

  if (state == PASS_EDIT) {
    handlePassEdit(pass, wheelResult, fullPass);
  } else if (state == PASS_SELECT_DIGIT) {
    handlePassSelectDigit(wheelResult);
  } else if (state == PASS_CONFIRMED) {
    if (wheelResult == DOUBLECLICK) {
      state = PASS_CHANGE;
      passChange = true;
    }
  }

  return state;
}

static void handlePassEdit(char* pass, wheel_input_t wheelResult, bool* fullPass) {
  switch (wheelResult) {
  case RIGHTTURN:
    input.bufIdx = input.bufLen == PASS_LEN ? PASS_LEN - 1 : clampInc(input.bufIdx, input.bufLen);
    state = passChange ? PASS_CHANGE_EDIT : PASS_EDIT;
    updateDisplay();
    break;
  case LEFTTURN:
    input.bufIdx = clampDec(input.bufIdx, 0);
    state = passChange ? PASS_CHANGE_EDIT : PASS_EDIT;
    updateDisplay();
    break;
  case CLICK:
    if (!IS_DIGIT(input.buf[input.bufIdx])) {
      input.buf[input.bufIdx] = '0';
    }
    state = passChange ? PASS_CHANGE_SELECT_DIGIT : PASS_SELECT_DIGIT;
    updateDisplay();
    break;
  case DOUBLECLICK:
    if (input.bufLen >= 4) {
      if (input.bufLen == 4) {
        *fullPass = false;
      } else {
        *fullPass = true;
      }
      uint8_t i = 0;
      for (; i < input.bufLen; ++i) pass[i] = input.buf[i];
      for (; i < PASS_LEN; ++i) pass[i] = 0;
      state = passChange ? PASS_CHANGE_CONFIRMED : PASS_CONFIRMED;
    }
    break;
  case CLICKHOLD:
    state = passChange ? PASS_CHANGE_CANCELLED : PASS_CANCELLED;
    break;
  default:
    break;
  }
}

static void handlePassSelectDigit(wheel_input_t wheelResult) {
  switch (wheelResult) {
  case RIGHTTURN:
    input.buf[input.bufIdx] = clampInc(input.buf[input.bufIdx], '9');
    updateDisplay();
    break;
  case LEFTTURN:
    input.buf[input.bufIdx] = clampDec(input.buf[input.bufIdx], '0');
    updateDisplay();
    break;
  case CLICK:
    if (input.bufIdx == input.bufLen && input.bufLen < PASS_LEN) ++input.bufLen;
    state = passChange ? PASS_CHANGE_EDIT : PASS_EDIT;
    updateDisplay();
    break;
  default:
    break;
  }
}

static void updateDisplay() {
  if (input.bufLen < DIGITS) {
    for (int8_t i = 0; i <= input.bufLen; ++i) {
      input.display[i] = input.hiddenBuf[i];
    }
  } else {
    for (int8_t i = DIGITS - 1, j = max(DIGITS - 1, input.bufIdx); i >= 0; --i, --j) {
      input.display[i] = input.hiddenBuf[j];
    }
  }

  if (state == PASS_SELECT_DIGIT || state == PASS_CHANGE_SELECT_DIGIT) {
    input.display[min(DIGITS - 1, input.bufIdx)] = input.buf[input.bufIdx];
  }
}
