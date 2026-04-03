#ifndef _WHEEL_H_
#define _WHEEL_H_

enum wheelInput
{
	IDLE,
	RIGHTTURN,
	LEFTTURN,
	CLICK,
	DOUBLECLICK,
	CLICKHOLD
};

bool wheelInit(void);

// 1 if there has been an input
extern bool wheelInputFlag;

// Returns input type from wheel
uint32_t readWheel(void);

#endif // _WHEEL_H_
