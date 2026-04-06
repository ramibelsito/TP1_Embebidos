#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdbool.h>
#include <stdint.h>

bool writeSegment(uint8_t segment, uint8_t digit, uint32_t dutyPercentage);

// Writes 1 number in the specified digit
bool writeNumber(uint8_t number, uint8_t digit, uint32_t dutyPercentage);

// Writes number ands slides number if needed
bool displayNumber(uint32_t number, uint32_t dutyPercentage);

bool cleanDisplay(void);

bool cleanSegment(uint8_t segment, uint8_t digit);

bool cleanNumber(uint8_t digit);

// Writes string
// bool displayStr(str * word);

#endif // _DISPLAY_H_



