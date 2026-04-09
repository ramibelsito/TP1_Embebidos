#include "card.h"
#include "hal/board.h"
#include "leds.h"
#include  "mcal/gpio.h"
#include "hardware.h"
#include "hal/IRQN_Ports.h"

#define CARD_MAX_BITS      256
#define CARD_MAX_CHARS     64


// Variables internas
static volatile uint8_t bitBuffer[CARD_MAX_BITS];
static volatile uint16_t bitCount = 0;

static volatile bool receiving = false;

bool cardReadFlag = false;
static uint32_t lastCard = 0;

// Forward
static void processBuffer(void);
static int decodeF2F(uint8_t *in, uint16_t len, uint8_t *out);
void cardHandler(void);

// ==========================================================
// INIT
// ==========================================================

bool cardInit(void)
{
    if (!gpioInit(PIN_CARD_CLOCK)) {
        return false;
    }
    if (!gpioInit(PIN_CARD_DATA)) {
        return false;
    }
    if (!gpioInit(PIN_ENABLE_DATA)) {
        return false;
    }
    
    gpioMode(PIN_CARD_CLOCK, INPUT);
    gpioMode(PIN_CARD_DATA, INPUT);
    gpioMode(PIN_ENABLE_DATA, INPUT);
    

    // Pull-up en las líneas que están idle en 1
    gpioPullUp(PIN_CARD_CLOCK);
    gpioPullUp(PIN_ENABLE_DATA);
    gpioPullUp(PIN_CARD_DATA);   // opcional, depende del hardware, pero no molesta si la línea está open collector/open drain

    // Interrupciones por falling edge
    fallingEdgeIRQC(PIN_CARD_CLOCK);   // PTC5
    fallingEdgeIRQC(PIN_ENABLE_DATA);  // PTC1

    // Habilitar interrupción del puerto C
    cleanFlags(PIN_CARD_CLOCK);
    cleanFlags(PIN_ENABLE_DATA);
    init_nvic(PC);
    setCallbacks(PC, cardHandler, 0);

    return true;
}


void cardHandler(void)
{
	uint32_t flags = PORTC->ISFR;
	// CARD	 CLOCK (PTC5)
	if (flags & (1 << 5))
	{
		cleanFlags(PIN_CARD_CLOCK);

		// lógica para CARD CLOCK
		ledToggle(RED);
	}

	// CARD ENABLE DATA (PTC1)
	if (flags & (1 << 1))
	{
		cleanFlags(PIN_ENABLE_DATA);

		// lógica para CARD ENABLE DATA
		//ledToggle(GREEN);
	}
}
// ==========================================================
// ENABLE 
// ==========================================================
/*
 *

bool cardEnable(void)
{
    bitCount = 0;
    receiving = true;
    cardReadFlag = false;
    
    //PORT_SetPinInterruptConfig(PORTC, CARD_CLK_PIN, kPORT_InterruptFallingEdge);

    return true;
}

bool cardDisable(void)
{
    PORT_SetPinInterruptConfig(PORTC, CARD_CLK_PIN, kPORT_InterruptOrDMADisabled);

    receiving = false;

    return true;
}

// ==========================================================
// INTERRUPT HANDLER
// ==========================================================

void PORTC_IRQHandler(void)
{
    if (PORT_GetPinsInterruptFlags(PORTC) & (1 << CARD_CLK_PIN))
    {
        PORT_ClearPinsInterruptFlags(PORTC, (1 << CARD_CLK_PIN));

        if (!receiving)
            return;

        if (bitCount < CARD_MAX_BITS)
        {
            uint8_t bit = GPIO_PinRead(CARD_DATA_GPIO, CARD_DATA_PIN);
            bitBuffer[bitCount++] = bit;
        }
        else
        {
            // Buffer lleno → procesar
            receiving = false;
            processBuffer();
        }
    }
}*/

// ==========================================================
// PROCESAMIENTO
// ==========================================================

static void processBuffer(void)
{
    uint8_t decoded[CARD_MAX_BITS];
    int decodedLen = decodeF2F((uint8_t*)bitBuffer, bitCount, decoded);

    if (decodedLen <= 0)
        return;

    // Track 2 → 5 bits por caracter
    char chars[CARD_MAX_CHARS];
    int charCount = 0;

    for (int i = 0; i + 4 < decodedLen; i += 5)
    {
        uint8_t val = 0;

        for (int j = 0; j < 4; j++)
        {
            val |= (decoded[i + j] << j);
        }

        chars[charCount++] = val + '0';
    }

    // Buscar ID (entre ; y ?)
    uint32_t id = 0;
    bool started = false;

    for (int i = 0; i < charCount; i++)
    {
        if (chars[i] == ';')
        {
            started = true;
            continue;
        }

        if (chars[i] == '?')
            break;

        if (started && chars[i] >= '0' && chars[i] <= '9')
        {
            id = id * 10 + (chars[i] - '0');
        }
    }

    if (id != 0)
    {
        lastCard = id;
        cardReadFlag = true;
    }
}

// ==========================================================
// DECODIFICACIÓN F2F (simplificada)
// ==========================================================

static int decodeF2F(uint8_t *in, uint16_t len, uint8_t *out)
{
    int outIndex = 0;

    for (int i = 1; i < len; i++)
    {
        // transición → bit 1
        if (in[i] != in[i - 1])
        {
            out[outIndex++] = 1;
        }
        else
        {
            out[outIndex++] = 0;
        }
    }

    return outIndex;
}

// ==========================================================
// API
// ==========================================================

uint32_t cardRead(void)
{
    cardReadFlag = false;
    return lastCard;
}
