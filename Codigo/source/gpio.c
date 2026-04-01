#include "gpio.h"
#include "hardware.h"
/*
typedef enum
{
	PORT_mAnalog,
	PORT_mGPIO,
	PORT_mAlt2,
	PORT_mAlt3,
	PORT_mAlt4,
	PORT_mAlt5,
	PORT_mAlt6,
	PORT_mAlt7,

} PORTMux_t;

static PORT_Type* const pPorts[5] = PORT_BASE_PTRS;
static GPIO_Type* const pGpios[5] = GPIO_BASE_PTRS;
*/
 /*
 * @brief Configures the specified pin to behave either as an input or an output
 * @param pin the pin whose mode you wish to set (according PORTNUM2PIN)
 * @param mode INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
 */
/*
void gpioMode (pin_t pin, uint8_t mode) {
	uint8_t port = PIN2PORT(pin);
	uint8_t number = PIN2NUM(pin);

	pPorts[port]->PCR[number] = 0x0; //Clear all bits
	pPorts[port]->PCR[number] |= PORT_PCR_MUX(PORT_mGPIO); 		//Set MUX to GPIO

	pGpios[port]->PDDR = (pGpios[port]->PDDR & ~(1 << number)) | (mode << number);
}
*/
/**
 * @brief Write a HIGH or a LOW value to a digital pin
 * @param pin the pin to write (according PORTNUM2PIN)
 * @param val Desired value (HIGH or LOW)
 */
/*
void gpioWrite (pin_t pin, bool value) {
	uint8_t port = PIN2PORT(pin);
	uint8_t number = PIN2NUM(pin);

	if(value) {
		pGpios[port]->PSOR = (uint32_t)(1 << number);
	}
	else {
		pGpios[port]->PCOR = (uint32_t)(1 << number);
	}
}
*/
/**
 * @brief Toggle the value of a digital pin (HIGH<->LOW)
 * @param pin the pin to toggle (according PORTNUM2PIN)
 */
/*
void gpioToggle (pin_t pin) {
	uint8_t port = PIN2PORT(pin);
	uint8_t number = PIN2NUM(pin);

	pGpios[port]->PTOR = (uint32_t)(1 << number);
}
*/
/**
 * @brief Reads the value from a specified digital pin, either HIGH or LOW.
 * @param pin the pin to read (according PORTNUM2PIN)
 * @return HIGH or LOW
 */
/*
bool gpioRead (pin_t pin) {
	uint8_t port = PIN2PORT(pin);
	uint8_t number = PIN2NUM(pin);

	bool result = (pGpios[port]->PDIR >> number) & 1;

	return result;
}
*/

void init_pins(void) {

    /* ------------------- LED ROJO (PTB22) ------------------- */
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;   // habilita reloj al PORTB
    PORTB->PCR[22] = PORT_PCR_MUX(1);     // función GPIO
    PTB->PDDR |= (1 << 22);               // pin como salida
    PTB->PCOR = (1 << 22);                // LED OFF

    /* ------------------- SW2 (PTC6) ------------------------- */
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;   // habilita reloj al PORTC
    PORTC->PCR[6] = PORT_PCR_MUX(1)       // GPIO
                  | PORT_PCR_PE_MASK      // enable pull
                  | PORT_PCR_PS_MASK      // pull-up
                  | PORT_PCR_IRQC(0b1010); // interrupción por falling edge

    PTC->PDDR &= ~(1 << 6);               // pin como entrada
}

void init_nvic(void) {
    NVIC_EnableIRQ(PORTC_IRQn);        // habilita interrupciones del PORTC
    NVIC_SetPriority(PORTC_IRQn, 3);   // prioridad cualquiera
}

/* ---------------- INTERRUPT HANDLER ---------------- */
void PORTC_IRQHandler(void) {

    // limpia el flag de interrupción del pin C6
    PORTC->ISFR = (1 << 6);

    // toggle del LED rojo
    PTB->PTOR = (1 << 22);
}
