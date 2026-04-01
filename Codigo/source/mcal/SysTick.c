#include "SysTick.h"
#include "hardware.h"
#include "gpio.h"
#include "board.h"
/**
 * @brief Initialize SysTic driver
 * @param funcallback Function to be call every SysTick
 * @return Initialization and registration succeed
 */

extern volatile uint32_t delay;

void funcallback(void);

__ISR__ SysTick_Handler(void)
{
	// Cuando se prende el led se arranca a medir, en la practica deberia
	// llevarse a cabo en otro pin.
	//gpioWrite(PIN_LED_GREEN, LOW);
	delay = 1;
	//gpioWrite(PIN_LED_GREEN, HIGH);

}

void funcallback (void)
{
	return;
}


bool SysTick_Init (void (*funcallback)(void) ) {
	SysTick->CTRL = 0x00;
	SysTick->LOAD = 100000000L -1; //12499999L; // <= 1000 ms @ 100Mhz
	SysTick->VAL  = 0x00;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	return 0;
}
