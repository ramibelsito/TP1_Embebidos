#include "mcal/gpio.h"

#include "board.h"
#include "hal/wheel.h"
#include "mcal/SysTick.h"
#include "hal/leds.h"
#define WHEEL_READ_TIME	10

#define BUFFER_SIZE		100 			// Number of elements of encoderData

#define TWO_BIT_MASK 	0x03

#define CLICKHOLDTIME 250
#define DOUBLECLICKTIME 200
#define CLICKENABLETIME 100

enum
{
	FIRSTC,
	SECONDC,
	THIRDC
};

enum 
{
    NOTCLICK,
    INCLICK,
    X2CLICK,
    IN2CLICK
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

// Variables Globales (de verdad)
bool wheelInputFlag;

// variables globales del archivo
static encoder_t encoderData[BUFFER_SIZE];
static uint32_t idx = 0;		// Global idx to move through encoderData
static uint32_t timeCounter = 0;



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
	timeCounter += 1;

}




wheel_input_t readWheel(void)
{
    static uint32_t clickState = 0;
    clickState = !encoderData[idx].rchd;
    static uint32_t pressState = IDLE;

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
    if (timeCounter < CLICKENABLETIME)
    {

    }
    else
	{
		switch (pressState)
		{
			default:
			case NOTCLICK:
				if (clickState == 1)
				{
					pressState = INCLICK;
					timeCounter = 0;
				}
				else {
					pressState = NOTCLICK;
				}
				break;
			case INCLICK:
				if (clickState == 1)
				{
					pressState = INCLICK;
				}
				else
				{
					if (timeCounter >= CLICKHOLDTIME)
					{
						pressState = NOTCLICK;
						timeCounter = 0;
						return CLICKHOLD;
					}
					else
					{
						pressState = X2CLICK;
						timeCounter = 0;
					}
				}
				break;
			case X2CLICK:
				if (clickState == 0 && timeCounter <= DOUBLECLICKTIME)
				{
					pressState = X2CLICK;
				}
				else if (clickState == 0 && timeCounter > DOUBLECLICKTIME)
				{

					pressState = NOTCLICK;
					timeCounter = 0;
					return CLICK;
				}
				else if (clickState == 1 && timeCounter <= DOUBLECLICKTIME)
				{
					pressState = IN2CLICK;
					timeCounter = 0;
				}
				break;
			case IN2CLICK:
				if (clickState == 1)
				{
					pressState = IN2CLICK;
				}
				else
				{
					pressState = NOTCLICK;
					timeCounter = 0;
					return DOUBLECLICK;
				}
				break;
		}
	}

	return IDLE;

}



