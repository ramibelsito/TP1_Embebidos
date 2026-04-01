/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "board.h"
#include "gpio.h"
#include "SysTick.h"

volatile uint32_t delay = 0;

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
void test(void)
{
	return;
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
	init_pins();
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
