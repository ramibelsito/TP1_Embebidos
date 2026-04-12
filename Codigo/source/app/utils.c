#include "utils.h"
#include <stdint.h>

uint8_t clampInc(uint8_t current, uint8_t max) {
  return current < max ? current + 1 : max;
}

uint8_t clampDec(uint8_t current, uint8_t min) {
  return current > min ? current - 1 : min;
}

uint8_t min(uint8_t a, uint8_t b) {
  return a <= b ? a : b;
}

uint8_t max(uint8_t a, uint8_t b) {
  return a >= b ? a : b;
}
