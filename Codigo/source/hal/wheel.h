#ifndef _WHEEL_H_
#define _WHEEL_H_
#include <stdbool.h>
#include <stdint.h>

typedef enum { IDLE, RIGHTTURN, LEFTTURN, CLICK, DOUBLECLICK, CLICKHOLD } wheel_input_t;

bool wheelInit(void);

// 1 if there has been an input
extern bool wheelInputFlag;

// Returns input type from wheel
wheel_input_t readWheel(void);

#endif // _WHEEL_H_
