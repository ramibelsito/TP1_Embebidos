#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define IS_DIGIT(x) ((x) >= '0' && (x) <= '9')

uint8_t clampInc(uint8_t current, uint8_t max);
uint8_t clampDec(uint8_t current, uint8_t min);
uint8_t min(uint8_t a, uint8_t b);
uint8_t max(uint8_t a, uint8_t b);

#endif
