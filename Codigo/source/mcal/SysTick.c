#include "SysTick.h"
#include "hardware.h"
#include "hal/IRQN_Ports.h"
//#include "gpio.h"

//#include "hal/board.h"
/**
 * @brief Initialize SysTic driver
 * @param funcallback Function to be call every SysTick
 * @return Initialization and registration succeed
 */


static pisr_t interruptions[PISR_CANT];	// Initialize maximum

static pisr_t interruptions[PISR_CANT];
static uint32_t isr_count = 0;
static uint32_t pisr_registered = 0;

__ISR__ SysTick_Handler(void)
{
	toggleInterruptFlag( 1);
    isr_count++;

    for (uint32_t i = 0; i < pisr_registered; i++) {
        if ((isr_count % interruptions[i].period) == 0) {
            interruptions[i].callback();
        }
    }
    toggleInterruptFlag( 0);
}

bool pisr_register(pisr_callback_t fun, uint32_t period)
{
    if (pisr_registered >= PISR_CANT || period == 0)
        return false;

    interruptions[pisr_registered].callback = fun;
    interruptions[pisr_registered].period   = period;
    pisr_registered++;

    return true;
}

bool SysTick_Init(uint32_t tick_hz)
{
    uint32_t reload = SystemCoreClock / tick_hz;

    SysTick->CTRL = 0;
    SysTick->LOAD = reload - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;

    return true;
}
