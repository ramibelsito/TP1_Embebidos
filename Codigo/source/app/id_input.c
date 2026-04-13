#include "id_input.h"
#include "app/utils.h"
#include "hal/display.h"
#include "hal/wheel.h"
#include <stdint.h>
#include "hal/card.h"
#include "hal/leds.h"

typedef struct id_input_t {
  char buf[ID_LEN];
  uint8_t bufIdx;
  char display[DIGITS];
} id_input_t;

static id_input_t input = {.buf = "00000000", .bufIdx = 0, .display = "0000"};
static IdInputState state = ID_EDIT;

static void handleIdEdit(char* id, wheel_input_t wheelResult);
static void handleIdSelectDigit(wheel_input_t wheelResult);
static void updateDisplay();

void initIdInput() {
  input = (id_input_t){.buf = "00000000", .bufIdx = 0, .display = "0000"};
  state = ID_EDIT;
  cleanDisplay();
}

IdInputState handleIdInput(char* id, wheel_input_t wheelResult) {
	if (cardAvailable())
	 {
		ledOn(BLUE);
		 processCardData();
		 cardRead(id);
		 return ID_CONFIRMED;
	 }
	for (uint8_t i = 0; i < DIGITS; ++i) {
	bool blink = (state == ID_EDIT) && (i == min(DIGITS - 1, input.bufIdx));
	writeCharacter(input.display[i], i, blink);
	enableDot(i, blink);
	}

	if (state == ID_EDIT) {
	handleIdEdit(id, wheelResult);
	} else if (state == ID_SELECT_DIGIT) {
	handleIdSelectDigit(wheelResult);
	}

	return state;
}

static void handleIdEdit(char* id, wheel_input_t wheelResult) {
  switch (wheelResult) {
  case RIGHTTURN:
    input.bufIdx = clampInc(input.bufIdx, ID_LEN - 1);
    state = ID_EDIT;
    updateDisplay();
    break;
  case LEFTTURN:
    input.bufIdx = clampDec(input.bufIdx, 0);
    state = ID_EDIT;
    updateDisplay();
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

static void handleIdSelectDigit(wheel_input_t wheelResult) {
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
    state = ID_EDIT;
    break;
  default:
    break;
  }
}

static void updateDisplay() {
  for (int8_t i = DIGITS - 1, j = max(DIGITS - 1, input.bufIdx); i >= 0; --i, --j) {
    input.display[i] = input.buf[j];
  }
}
