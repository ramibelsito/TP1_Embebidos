#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdbool.h>
#include <stdint.h>


typedef struct TIMER
{
	uint32_t timerID;
	uint32_t timerTarget; // Seconds
	bool timerRing;
} timer_t;

timer_t createTimer(uint32_t timerTarget);

bool sleep(uint32_t seconds);

bool timerCheck(timer_t timer);

#endif // _TIMER_H_
