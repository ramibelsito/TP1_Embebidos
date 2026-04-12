#ifndef DISPLAY_INTENSITY_H
#define DISPLAY_INTENSITY_H

#include "hal/wheel.h"

typedef enum { INTENSITY_EDIT, INTENSITY_CONFIRM } IntensityState;

IntensityState handleDisplayIntensity(wheel_input_t wheelResult);

#endif
