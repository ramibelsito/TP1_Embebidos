#include "id_input.h"
#include "app/utils.h"
#include "hal/display.h"
#include "hal/wheel.h"
#include <stdint.h>

typedef struct id_input_t {
  char buf[ID_LEN];
  uint8_t bufIdx;
  char display[DIGITS];
} id_input_t;

static id_input_t input = {.buf = "00000000", .bufIdx = 0, .display = "0000"};
static IdInputState state = ID_EDIT;

void handleIdEdit(char* id, wheel_input_t wheelResult);
void handleIdSelectDigit(char* id, wheel_input_t wheelResult);

IdInputState handleIdInput(char* id, wheel_input_t wheelResult) {
  for (uint8_t i = 0; i < DIGITS; ++i) {
    bool blink = (state == ID_EDIT) && (i == (input.bufIdx % DIGITS));
    writeCharacter(input.display[i], i, blink);
  }

  if (state == ID_EDIT) {
    handleIdEdit(id, wheelResult);
  } else if (state == ID_SELECT_DIGIT) {
    handleIdSelectDigit(id, wheelResult);
  }

  for (int8_t i = DIGITS - 1, j = max(DIGITS - 1, input.bufIdx); i >= 0; --i, --j) {
    input.display[i] = input.buf[j];
  }

  return state;
}

void handleIdEdit(char* id, wheel_input_t wheelResult) {
  switch (wheelResult) {
  case RIGHTTURN:
    input.bufIdx = clampInc(input.bufIdx, ID_LEN - 1);
    state = ID_EDIT;
    break;
  case LEFTTURN:
    input.bufIdx = clampDec(input.bufIdx, 0);
    state = ID_EDIT;
    break;
  case CLICK:
    state = ID_SELECT_DIGIT;
    break;
  case DOUBLECLICK:
    for (uint8_t i = 0; i < ID_LEN; ++i) id[i] = input.buf[i];
    state = ID_CONFIRMED;
    break;
  case CLICKHOLD:
    state = ID_CANCELLED;
    break;
  default:
    break;
  }
}

void handleIdSelectDigit(char* id, wheel_input_t wheelResult) {
  switch (wheelResult) {
  case RIGHTTURN:
    input.buf[input.bufIdx] = clampInc(input.buf[input.bufIdx], '9');
    break;
  case LEFTTURN:
    input.buf[input.bufIdx] = clampDec(input.buf[input.bufIdx], '0');
    break;
  case CLICK:
    state = ID_EDIT;
    break;
  default:
    break;
  }
}
