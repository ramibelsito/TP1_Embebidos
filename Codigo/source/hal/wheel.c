#include "mcal/gpio.h"

#include "mcal/board.h"
#include "hal/wheel.h"
#include "mcal/SysTick.h"

#define WHEEL_READ_TIME	1

#define BUFFER_SIZE		100 			// Number of elements of encoderData

#define TWO_BIT_MASK 	0x03

uint8_t clickCounter = 0;
bool wheelInputFlag;

uint8_t validPrev = 2;


uint8_t dir = LEFTTURN;

enum
{
	FIRSTC,
	SECONDC,
	THIRDC
};


typedef union {
    uint8_t raw;
    struct {
        uint8_t rcha :1;
        uint8_t rchb :1;
        uint8_t rchd :1;
        uint8_t free :5;
    };
} encoder_t;

encoder_t encoderData[BUFFER_SIZE];

uint32_t idx = 0;		// Global idx to move through encoderData
uint8_t move = 0;




void wheelReadGPIO(void);




bool wheelInit()
{
	// Inicializar dos puertos para leer los canales A y B de la ruedita
	if(!gpioInit(PIN_RCHA)) {
		return false;		// Si no se inicializa bien, devuelve un false
	}
	if(!gpioInit(PIN_RCHB)) {
		return false;		// Si no se inicializa bien, devuelve un false
	}
	if(!gpioInit(PIN_RCHD)) {
		return false;		// Si no se inicializa bien, devuelve un false
	}
	gpioMode(PIN_RCHA, INPUT);
	gpioMode(PIN_RCHB, INPUT);
	gpioMode(PIN_RCHD, INPUT);


	pisr_register(wheelReadGPIO, WHEEL_READ_TIME);		// Read wheel data every WHEEL_READ_TIME (ms)
	return true;
}

void wheelReadGPIO(void) {
	encoderData[idx].rcha = gpioRead(PIN_RCHA);
	encoderData[idx].rchb = gpioRead(PIN_RCHB);
	encoderData[idx].rchd = gpioRead(PIN_RCHD);
	encoderData[idx].free = 0;

	idx++;
	if(idx > BUFFER_SIZE - 1) {			// idx lives between 0 and 19
		idx = idx % BUFFER_SIZE;
	}
	clickCounter += 1;
}




uint32_t readWheel(void)
{
	static uint8_t clickState = 0;



    static uint8_t prev = 0;
    static int8_t acc = 0;

    uint8_t curr = (encoderData[idx].raw & TWO_BIT_MASK);

    static const int8_t table[4][4] = {
 // curr  00, 01, 10, 11	prev
        { 0, -1, +1,  0}, // 00
        {+1,  0,  0, -1}, // 01
        {-1,  0,  0, +1}, // 10
        { 0, +1, -1,  0} //  11
    };

    int8_t delta = table[prev][curr];

    uint8_t diff = prev ^ curr;

    // detectar salto doble
    if(diff == 0x03) {
        if(acc > 0) delta = +2;
        else if(acc < 0) delta = -2;
        else delta = 0; // ambiguo
    }

    acc += delta;
    prev = curr;

    if(acc >= 3) {
        acc = 0;
        return RIGHTTURN;
				}
    else if(acc <= -3) {
        acc = 0;
        return LEFTTURN;
    }

    if (!encoderData[idx].rchd && !clickState)
    {
    	clickState = 1;
    	clickCounter = 0;
				}
    else if (!encoderData[idx].rchd && clickState == 1 && clickCounter > 300)
    {
    	clickState = 0;
    	return CLICKHOLD;
    }
    else if (encoderData[idx].rchd && clickState == 1 && clickCounter < 100)
    {
    	clickCounter = 0;
    	clickState = 2;
				}
    else if (encoderData[idx].rchd && clickState == 2 && clickCounter > 200)
    {
    	clickState = 0;
    	return CLICK;
				}
    else if (!encoderData[idx].rchd && clickState == 2 && clickCounter < 200)
    {
    	clickState = 3;
				}
    else if (encoderData[idx].rchd && clickState == 3)
    {
    	clickState = 0;
    	return DOUBLECLICK;
	}
	else {
		return IDLE;
	}
}



