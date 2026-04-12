#include "app/display_intensity.h"
#include "hal/display.h"
#include "hal/wheel.h"

IntensityState handleDisplayIntensity(wheel_input_t wheelResult) {
  switch (wheelResult) {
  case RIGHTTURN:
    setDutyPercentage(getDutyPercentage() + 1);
    return INTENSITY_EDIT;
  case LEFTTURN:
    setDutyPercentage(getDutyPercentage() - 1);
    return INTENSITY_EDIT;
  case DOUBLECLICK:
    return INTENSITY_CONFIRM;
  default:
    return INTENSITY_EDIT;
  }
}
