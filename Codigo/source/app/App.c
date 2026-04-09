/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "app/action.h"

#include "hal/timers.h"

// FOR INIT
#include "hal/display.h"
#include "hal/wheel.h"
#include "hal/card.h"
#include "hal/leds.h"
#include "mcal/gpio.h"
#include "hal/board.h"
#include "mcal/SysTick.h"
#include "hal/card.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*

void sensor_read(void) {
	if(gpioRead(PIN_SW2) == SW_ACTIVE) {
		test_var = !test_var;
	}
}
*/

void App_Init (void)
{
	/*if(gpioInit(PIN_LED_RED)) {
		gpioMode(PIN_LED_RED, OUTPUT);
		gpioWrite(PIN_LED_RED, HIGH);
	}

	if(gpioInit(PIN_SW2)) {
		gpioMode(PIN_SW2, INPUT);
	}


	SysTick_Init(1000); // 1ms tick

	pisr_register(led_toggle, 500);     // cada 500 ms
	pisr_register(sensor_read, 300);    // cada 100 ms

	//testInterruptSW2(PIN_SW2);
	init_nvic();*/
	SysTick_Init(100);
	if (ledsInit(WHITE))
	{
		ledOff(WHITE);
		return;
	}

	if (wheelInit() == false)
	{
		ledOn(RED);
	}
	if (!cardInit()){
		ledOn(RED);
	}
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	if(!gpioRead(PIN_ENABLE_DATA))
	{
		//ledOn(GREEN);
	}
	/*
	uint32_t result = readWheel();
	switch (result)
	{
	case RIGHTTURN:
		ledOn(GREEN);
		break;
	case LEFTTURN:
		ledOn(RED);
		break;
	case IDLE:
		vez = 0;
		break;
	case CLICK:
		ledOff(WHITE);
		break;
	case DOUBLECLICK:
		ledOn(PINK);
		break;
	case CLICKHOLD:
		ledOn(CYAN);
		break;
	default:
		ledOn(WHITE);
		break;

	}*/

}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/




/*******************************************************************************
 ******************************************************************************/
