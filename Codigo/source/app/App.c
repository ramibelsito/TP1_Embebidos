/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "hal/board.h"
#include "mcal/gpio.h"
#include "mcal/SysTick.h"

volatile uint32_t delay = 0;

static bool test_var = 0;

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

//static void delayLoop(uint32_t veces);


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/




void led_toggle(void) {
	if(!test_var) {
		gpioToggle(PIN_LED_RED);
	}
}

void sensor_read(void) {
	if(gpioRead(PIN_SW2) == SW_ACTIVE) {
		test_var = !test_var;
	}
}

void sleepFunction(void);

uint32_t balizas = 0;

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{/*
	SysTick_Init(test);

	gpioMode(PIN_LED_GREEN, OUTPUT);
    gpioMode(PIN_LED_RED, OUTPUT);
    gpioMode(PIN_LED_BLUE, OUTPUT);

    gpioWrite(PIN_LED_GREEN, HIGH);
    gpioWrite(PIN_LED_RED, HIGH);
    gpioWrite(PIN_LED_BLUE, HIGH);


    gpioMode(PIN_SW2, INPUT);
*/
	if(gpioInit(PIN_LED_RED)) {
		gpioMode(PIN_LED_RED, OUTPUT);
		gpioWrite(PIN_LED_RED, HIGH);
	}

	if(gpioInit(PIN_SW2)) {
		gpioMode(PIN_SW2, INPUT);
	}


	SysTick_Init(1000); // 1ms tick

	pisr_register(led_toggle, 500);     // cada 500 ms
	pisr_register(sensor_read, 100);    // cada 100 ms

	//testInterruptSW2(PIN_SW2);
	init_nvic();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
/*
	if (gpioRead(PIN_SW2) == SW_ACTIVE)
	{
		sleepFunction();
		balizas = !balizas;

	}

	switch (balizas)
	{
		case 1:
			if (delay)
			{
				gpioToggle(PIN_LED_GREEN);
				gpioToggle(PIN_LED_RED);
				delay = 0;
			}
			break;
		default:
		case 0:
			gpioWrite(PIN_LED_RED, HIGH);
			gpioWrite(PIN_LED_GREEN, HIGH);
			break;

	}*/

}
/*
void sleepFunction(void)
{
	uint32_t sleep = 4000000UL;
	if (!sleep)
	{
		sleep = 4000000UL;
	}
	sleep--;
	return;
}*/
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/




/*******************************************************************************
 ******************************************************************************/
