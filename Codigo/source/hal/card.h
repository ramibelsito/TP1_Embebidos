#ifndef _CARD_H_
#define _CARD_H_
#include <stdbool.h>
#include <stdint.h>



// Initializes the card reader driver
bool cardInit(void);

// Returns if cardData is ready
bool cardAvailable(void);

// Calls processBuffer() to process all buffer data when app needs it
void processCardData(void);

// Returns last card read
uint32_t cardRead(void);

#endif // _CARD_H_
