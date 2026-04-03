#ifndef _DISPLAY_H_
#define _DISPLAY_H_
bool writeSegment(uint4_t segment, uint2_t digit, uint32_t dutyPercentage);

// Writes 1 number in the specified digit
bool writeNumber(uint4_t number, uint2_t digit, uint32_t dutyPercentage);

// Writes number ands slides number if needed
bool displayNumber(uint32_t number, uint32_t dutyPercentage);

bool cleanDisplay(void);

bool cleanSegment(uint4_t segment, uint2_t digit);

bool cleanNumber(uint2_t digit);

// Writes string
// bool displayStr(str * word);

#endif // _DISPLAY_H_



