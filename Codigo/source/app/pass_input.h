#ifndef PASS_INPUT_H
#define PASS_INPUT_H

#include "hal/display.h"
#include "hal/wheel.h"
#include <stdint.h>

#define PASS_LEN 5

typedef enum {
  PASS_EDIT,
  PASS_SELECT_DIGIT,
  PASS_CONFIRMED,
  PASS_CANCELLED,
  PASS_CHANGE,
  PASS_CHANGE_EDIT,
  PASS_CHANGE_SELECT_DIGIT,
  PASS_CHANGE_CONFIRMED,
  PASS_CHANGE_CANCELLED,
} PassInputState;

void initPassInput();
PassInputState handlePassInput(char* pass, wheel_input_t wheelResult, bool* fullPass);

#endif
