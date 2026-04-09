#ifndef __LEDS_H__
#define __LEDS_H__

#include <stdint.h>
#include <stdbool.h>

enum
{
	RED,
	GREEN,
	BLUE,
	CYAN,
	PINK,
	YELLOW,
	WHITE
};
// returns NULL if correct
bool ledsInit(uint8_t color);
// returns NULL if correct
bool ledOn(uint8_t color);
// returns NULL if correct
bool ledOff(uint8_t color);
// returns NULL if correct
bool ledToggle(uint8_t color);

#endif // __LEDS_H__
