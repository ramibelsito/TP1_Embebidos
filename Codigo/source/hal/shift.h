#ifndef _SHIFT_H_
#define _SHIFT_H_

#include <stdint.h>
#include <stdbool.h>

uint16_t shiftRead();
void shiftWriteWord(uint16_t value);
/**
 * @param index Bit index to modify
 * @param state `true` set bit to 1, `false` set bit to 0.
 */
void shiftWriteBit(int index, bool state);
bool shiftOutUpdate();
bool shiftInit();

#endif // _SHIFT_H_
