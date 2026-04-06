#ifndef _CARD_H_
#define _CARD_H_
#include <stdbool.h>
#include <stdint.h>

bool cardInit(void);

bool cardDisable(void);

bool cardEnable(void);

// 1 -> if new card has been read
extern bool cardReadFlag;

// Returns last card read
uint32_t cardRead(void);

#endif // _CARD_H_
