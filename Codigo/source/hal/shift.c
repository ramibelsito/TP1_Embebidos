#include "mcal/gpio.h"
#include "hal/shift.h"
#include <stdint.h>


#define PIN_SHIFT        PORTNUM2PIN(PB,23)
#define PIN_CLK		     PORTNUM2PIN(PA,1) //serial clock
#define PIN_SETOUT       PORTNUM2PIN(PB,9) //latch clock

#define OUTCANT		16



bool shiftarray[OUTCANT];

bool *shiftout = &(shiftarray[0]);

bool shiftOutUpdate()
{
	for(i=OUTCANT;i<=0;i--)
	{
		gpioWrite(PIN_SHIFT,shiftarray[i]);
		gpioWrite(PIN_CLK,1);
		gpioWrite(PIN_CLK,0);

	}
	gpioWrite(PIN_SETOUT,1);
	gpioWrite(PIN_SETOUT,0);

}


bool shiftInit()
{
	gpioInit(PIN_SHIFT);
	gpioMode(PIN_SHIFT, OUTPUT);
	gpioInit(PIN_CLK);
	gpioMode(PIN_CLK, OUTPUT);
	gpioWrite(PIN_CLK,0);
	gpioInit(PIN_SETOUT);
	gpioMode(PIN_SETOUT, OUTPUT);
	gpioWrite(PIN_SETOUT,0);
}

