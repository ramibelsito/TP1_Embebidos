#include "card.h"
#include "hal/board.h"
#include "leds.h"
#include  "mcal/gpio.h"
#include "hardware.h"
#include "hal/IRQN_Ports.h"
#include "mcal/SysTick.h"

#define CARD_MAX_BITS      320
#define CARD_MAX_CHARS     40
#define CARD_TIMEOUT_MS    50   

// Se puede acceder al char completo como x.fields.truchar.raw o podes acceder bit a bit como x.fields.truchar.bit.b0
// Hay que agregar al union una forma de acceder solo al bit paridad y solo a los 4 bits de caracter
typedef union
{
    uint8_t raw;
    struct {
        union {
            uint8_t raw : 5;
            struct {
                uint8_t b4 : 1;         // LSB
                uint8_t b3 : 1;
                uint8_t b2 : 1;
                uint8_t b1 : 1;
                uint8_t b0 : 1;         // MSB
            } bit;
        } truchar;
        uint8_t free : 3;
    } fields;
} truchar_t;


#define SS 0x1F // Start Sentinel ';' -> 0b11111
#define ES 0x1E // End Sentinel '?' -> 0b11110
#define FS 0x1D // Field Separator '=' -> 0b11101

// Variables internas
static volatile truchar_t bitBuffer[CARD_MAX_CHARS];
static volatile uint16_t bitCount = 0;
volatile bool cardDataReady = false;            // True cuando se terminar de recibir los bits -> Activa el procesamiento processBuffer()
volatile uint32_t lastBitTime = 0;              // Dato para determinar timeout en la recepcion de los datos de una tarjeta
uint32_t timeoutCounter = 0;                    // Suma cada vez que entra al pisr. Compara con lastBitTime
static volatile bool receiving = false;         // True cuando detecta el flanco de enable de la tarjeta
static bool cardReadFlag = false;                      // True cuando ya se tiene el ID (ya termino de procesar processBuffer())






char id[8];
static uint32_t lastCard = 0;


// LOCAL FUNCTIONS
void processIdData(void);
static void checkTimeout(void);
uint8_t get_char_from_bits(truchar_t t);
static uint32_t calculateIdNumber(void);

// HANDLERS & CALLBACKS
void timerForTimeout (void);
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

    // Habilito interrupción periodica para el timeout
    pisr_register(timerForTimeout, 1);	

    return true;
}


// ==========================================================
// HANDLER & CALLBACKS
// ==========================================================

// Aumenta su valor a medida que entra el systick. Sirve de referencia para veificar si se pasa un tiempo límite de espera
void timerForTimeout (void) {
    timeoutCounter++;
}

// Handler para las interrupciones de la tarjeta
void cardHandler(void)
{   
	uint32_t flags = PORTC->ISFR;

    // CARD ENABLE DATA (PTC1)
    if (flags & (1 << 1))
    {
        cleanFlags(PIN_ENABLE_DATA);

        // lógica para CARD ENABLE DATA

        // TODO Si solo se mete cuando detecta el flanco y no vuelve a entrar hasta la siguiente vez
        // que reciba una tarjeta -> ANDA BIEN

        // Sino, hay que meter otro flag o desactivar esta interrupción hasta que se lea la tarjeta
        bitCount = 0;
        receiving = true;
        cardReadFlag = false;

        // Reset de los timers cuando detecta una nueva lectura de la tarjeta
        timeoutCounter = 0;
        lastBitTime = 0;
    }

	// CARD	 CLOCK (PTC5)
	if (flags & (1 << 5))
	{
		cleanFlags(PIN_CARD_CLOCK);

		// lógica para CARD CLOCK
		
        if (!receiving)
            return;

        if (bitCount < CARD_MAX_BITS)
        {
            lastBitTime = timeoutCounter;             // Actualiza el tiempo del último dato
            bool bit = gpioRead(PIN_CARD_DATA);
            uint8_t idx = (uint8_t)(bitCount % 5);
            switch (idx)
            {
                case 0:
                    bitBuffer[(uint8_t)(bitCount/5)].fields.truchar.bit.b0 = bit;
                    break;
                case 1:
                    bitBuffer[(uint8_t)(bitCount/5)].fields.truchar.bit.b1 = bit;
                    break;
                case 2:
                    bitBuffer[(uint8_t)(bitCount/5)].fields.truchar.bit.b2 = bit;
                    break;
                case 3:
                    bitBuffer[(uint8_t)(bitCount/5)].fields.truchar.bit.b3 = bit;
                    break;
                case 4:
                    bitBuffer[(uint8_t)(bitCount/5)].fields.truchar.bit.b4 = bit;
                    break;
            }
            bitCount++;
        }
        else
        {
            receiving = false;
            cardDataReady = true;        // Ya se terminaron de leer los datos
        }
	}

    checkTimeout();             // Si supera el tiempo de timeout, deja de recibir datos (receiving=0)

}

// ==========================================================
// PROCESAMIENTO
// ==========================================================

// Encuentra el StartSentinel, el EndSentinel y traduce a los caracteres correspondientes los bits intermedios. Corroborar paridad.
// Setea el string id
void processIdData(void)
{
    bool SSFound = false;
    bool FSFound = false;
    uint8_t SSPos = 0;
    uint8_t FSPos = 0;
    uint8_t i = 0;
    
    while ( bitBuffer[i].fields.truchar.raw != ES)
    {
        if (bitBuffer[i].fields.truchar.raw == SS)
        {
            SSFound = true;
            SSPos = i;
        }
        if (bitBuffer[i].fields.truchar.raw == FS)
        {
            FSFound = true;
            FSPos = i;
        }
        if (SSFound)
        {
            
            i++; // Sale apuntando a LRC
        }
    }
    uint8_t lrcPos = i;
    // Corroborar Paridad Horizontal
    i = 1;
    uint8_t count = 0;


    while ( bitBuffer[SSPos+i].fields.truchar.raw != ES ) {
        count = bitBuffer[SSPos+i].fields.truchar.bit.b1
                    + bitBuffer[SSPos+i].fields.truchar.bit.b2
                    + bitBuffer[SSPos+i].fields.truchar.bit.b3
                    + bitBuffer[SSPos+i].fields.truchar.bit.b4;

        if (!(count % 2 == 0 && bitBuffer[SSPos+i].fields.truchar.bit.b0 == 1)) {
            return;                                                              // Detecta error --> Sale de la función
        }
        i++;
    }

    // Corroborar Paridad Vertical (LRC) 
    i = 0;
    count = 0;
    uint8_t j = 0;
    // LRC tiene una longitud de 4 bits
    while (i < 4) {
        while ( bitBuffer[SSPos+j].fields.truchar.raw != ES ) {
            count += bitBuffer[SSPos+j].fields.truchar.raw & (1 << i);         // Si vale uno se suma el contador de unos.
            j++;
        }
        if (!(count % 2 == 0 && (bitBuffer[lrcPos].fields.truchar.raw & (1 << i)) == 1)) {
            return;                                                              // Detecta error --> Sale de la función
        }
        i++;
        j = 0;
        count = 0;
    }

    // Traducir a caracteres --> si sale mal retornar un string vacio
    // Va a agregarlos a id --> id[0] = bitBuffer[SSPos+1].fields.truchar.char hasta id[7] = bitBuffer[SSPos+8].fields.truchar.char
    for (int j = 0 ; j < sizeof(id)/sizeof(id[0]) ; j++) {
        id[j] = get_char_from_bits(bitBuffer[SSPos+j]);
    }
}

// calcula el char para los chunks de 5 bits por caracter leídos
uint8_t get_char_from_bits(truchar_t t) {
    uint8_t r = t.fields.truchar.raw;  // b0...b4 (b0 más significativo === Paridad)
    uint8_t val = 0;

    val |= ((r >> 0) & 0x01) << 0;  
    val |= ((r >> 1) & 0x01) << 1;  
    val |= ((r >> 2) & 0x01) << 2;  
    val |= ((r >> 3) & 0x01) << 3;

    return val;   // char formado por b1..b4 (4 bits)
}



// Limpia el buffer para la próxima lectura. De esta forma nos aseguramos que no tenga data basura de antes
// Se llama a esta función uego de procesar el id de la tarjeta leída
void cleanBuffer(void) {
    for (int i = 0 ; sizeof(bitBuffer)/sizeof(bitBuffer[0]) ; i++) {
        bitBuffer[i].raw = 0x00;
    }
}

// Calcula el valor numérico del id leído de la tarjeta
static uint32_t calculateIdNumber(void) {
    uint32_t value = 0;

    for (int i = 0 ; i < 8 ; i++)
    {
        if (id[i] < '0' || id[i] > '9')
        {
            return 0; // error: caracter no numérico
        }

        value = value * 10 + (id[i] - '0');
    }

    return value;
}

// ==========================================================
// TIMEOUT
// ==========================================================

// Verifica si sucedio la condición de timeout
static void checkTimeout(void)
{
    if (receiving)
    {
        if ((timeoutCounter - lastBitTime) > CARD_TIMEOUT_MS)
        {
            // Si supera el tiempo de timeout deja de leer los datos
            receiving = false;
            bitCount = 0;
        }
    }
}


// ==========================================================
// API
// ==========================================================

// cardAvailable solo me ayuda si quiero hacer un flujo mas claro y que no dependa solo de datos, sino de estados
// Puedo llamar de una a cardGet(), pero aporta mas claridad, no mucho mas
bool cardAvailable(void) {
    return cardReadFlag;
}

// processCardData tiene acceso desde la App, y permite vincular la app con la 
//función que hace todos los cálculos para procesar el buffer de datos

void processCardData(void) {

    if (cardDataReady)
    {
        processIdData();
        cardDataReady = false;
    }
    lastCard = calculateIdNumber();     // Actualiza lastCard con el valor de id de la tarjeta procesada

    cardReadFlag = true;                // Ya se proceso la data de la tarjeta
}


uint32_t cardRead(void)
{
    if (!cardReadFlag)
        return 0;

    cardReadFlag = false;           // Limpiar el flag

    return lastCard;                // Devuelve el último Id calculado
}
