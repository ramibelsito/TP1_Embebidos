#ifndef ID_INPUT_H
#define ID_INPUT_H

#include "hal/display.h"
#include "hal/wheel.h"
#include <stdint.h>

#define ID_LEN 8
#define PASS_LEN 5

typedef enum { ID_EDIT, ID_SELECT_DIGIT, ID_CONFIRMED, ID_CANCELLED } IdInputState;

IdInputState handleIdInput(char* id, wheel_input_t wheelResult);

#endif
