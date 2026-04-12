#include "card.h"
#include "hal/board.h"
#include "leds.h"
#include  "mcal/gpio.h"
#include "hardware.h"
#include "hal/IRQN_Ports.h"
#include "mcal/SysTick.h"
#include <string.h>

//#define DEBUG

#define CARD_MAX      80
#define CARD_MAX_CHARS     40

#define SS 0b01011 // Start Sentinel ';'
#define ES 0b11111 // End Sentinel '?'
#define FS 0b01101 // Field Separator '='

uint32_t debugCounter =0 ;
char myString[500] = "";

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



// Variables internas
static volatile truchar_t bitBuffer[CARD_MAX];
static volatile int32_t bitCount = 0;
volatile bool cardDataReady = false;            // True cuando se terminar de recibir los bits -> Activa el procesamiento processBuffer()
volatile uint32_t lastBitTime = 0;              // Dato para determinar timeout en la recepcion de los datos de una tarjeta
volatile uint32_t timeoutCounter = 0;                    // Suma cada vez que entra al pisr. Compara con lastBitTime
static volatile bool receiving = false;         // True cuando detecta el flanco de enable de la tarjeta
volatile static bool cardIsReadyFlag = false;
char id[8];
static uint32_t lastCard = 0;
volatile static bool firstSSBit = 0;
static bool flagES = false;

// LOCAL FUNCTIONS
static void processIdData(void);
static void checkTimeout(void);
static uint8_t get_char_from_bits(truchar_t t);
static uint8_t make_char(uint8_t value4bits);
static void cleanBuffer(void);
static uint32_t calculateIdNumber(void);

// HANDLERS & CALLBACKS
void timerForTimeout (void);
void cardHandler(void);

// ==========================================================
// INIT
// ==========================================================

// Inicializa los pines vinculados al lector de tarjetas
// Devuelve true si inicializa correctamente
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
    if (flags & (1 << 0))
    {
        cleanFlags(PIN_ENABLE_DATA);

        // lógica para CARD ENABLE DATA

        // TODO Si solo se mete cuando detecta el flanco y no vuelve a entrar hasta la siguiente vez
        // que reciba una tarjeta -> ANDA BIEN

        // Sino, hay que meter otro flag o desactivar esta interrupción hasta que se lea la tarjeta
        bitCount = 0;
        receiving = true;
        firstSSBit = 0;
        cardIsReadyFlag = false;
        // Reset de los timers cuando detecta una nueva lectura de la tarjeta
        timeoutCounter = 0;
        lastBitTime = 0;

        cleanBuffer();
    }

	// CARD	 CLOCK (PTC5)
	if (flags & (1 << 5))
	{
		cleanFlags(PIN_CARD_CLOCK);

		// lógica para CARD CLOCK


        if (!receiving)
            return;

		lastBitTime = timeoutCounter;             // Actualiza el tiempo del último dato
 		bool bit = !gpioRead(PIN_CARD_DATA);
		uint8_t idx = (uint8_t)(bitCount % 5);
		bitBuffer[(uint8_t)(bitCount/5)].fields.truchar.raw |= (bit << idx);

		if (bit && !firstSSBit) {
			firstSSBit = true;

		}
		if (flagES)
		{
			cardIsReadyFlag = true;
			receiving = false;
			bitCount=0;
			firstSSBit = 0;
			flagES = false;
		}
		if (bitBuffer[(uint8_t)((bitCount)/5)].fields.truchar.raw == ES || bitCount > 200)
		{
			flagES = true;
		}


		if (firstSSBit) {
			bitCount++;
		}
		#ifdef DEBUG
		if (bit)
		{

			strcat(myString, "1");
		}
		else
		{
			strcat(myString, "0");
		}
		debugCounter++;
		if (debugCounter>=200)
		{
			debugCounter--;
			debugCounter++;
		}
		#endif

	}


}



// ==========================================================
// PROCESAMIENTO
// ==========================================================

// Encuentra el StartSentinel, el EndSentinel y traduce a los caracteres correspondientes los bits intermedios. Corroborar paridad.
// Setea el string id
static void processIdData(void)
{

    bool SSFound = false;
    uint8_t SSPos = 0;
    uint8_t ESPos = 0;
    uint8_t LRCPos = 0;

    uint8_t i = 0;

	ledToggle(BLUE);
    while ( bitBuffer[i].fields.truchar.raw != ES)
    {
        if (bitBuffer[i].fields.truchar.raw == SS)
        {

            SSFound = true;
            SSPos = i;
        }

        if (SSFound)
        {
            i++; // Sale apuntando a LRC
        }
    }
    ESPos = i;
    LRCPos = ESPos+1;       // LRC viene siempre despues de ES


    // Corroborar Paridad Horizontal
    i = 1;
    uint8_t count = 0;

    while ( bitBuffer[SSPos+i].fields.truchar.raw != ES ) {
        count = bitBuffer[SSPos+i].fields.truchar.bit.b1
                    + bitBuffer[SSPos+i].fields.truchar.bit.b2
                    + bitBuffer[SSPos+i].fields.truchar.bit.b3
                    + bitBuffer[SSPos+i].fields.truchar.bit.b4;

        // printf("Dígito %u: Count = %u, Paridad = %u \n", i,count,bitBuffer[SSPos+i].fields.truchar.bit.b0);

        if (!((count % 2 == 0 && bitBuffer[SSPos+i].fields.truchar.bit.b0 == 1) ||
            (count % 2 != 0 && bitBuffer[SSPos+i].fields.truchar.bit.b0 == 0))) {

            return;         // Detecta error --> Sale de la función
        }
        i++;
    }

    // Corroborar Paridad Vertical (LRC)

    i = 0;
    count = 0;
    uint8_t j = 0;
    //uint8_t lrc = get_char_from_bits(bitBuffer[LRCPos]);

    // LRC tiene una longitud de 4 bits
    while (i < 4) {
        // printf("\n i: %u, j: %u, Count: %u \n", i, j, count);

        while ((SSPos + j) < LRCPos ) {
            // printf("%u,", bitBuffer[SSPos+j].fields.truchar.raw & (1 << i));
            count ^= (bitBuffer[SSPos+j].fields.truchar.raw >> i) & 1;         // Si vale uno se suma el contador de unos.
            j++;
        }

        if (count != ((bitBuffer[LRCPos].fields.truchar.raw >> i) & 1)) {
        	return;                      // Detecta error --> Sale de la función
        }
        i++;
        j = 0;
        count = 0;
    }

    /*uint32_t contb4 = 0;
    uint32_t contb3 = 0;
    uint32_t contb2 = 0;
    uint32_t contb1 = 0;
    for (int i = 0; i < LRCPos; i+=5)
    {
    	if (bitBuffer[i].fields.truchar.bit.b4)
    	{
    		++contb4;
    	}
    	if (bitBuffer[i].fields.truchar.bit.b3)
    	{
    		++contb3;
    	}
    	if (bitBuffer[i].fields.truchar.bit.b2)
		{
			++contb2;
		}
    	if (bitBuffer[i].fields.truchar.bit.b1)
		{
			^+contb1;
		}
    }
    if (contb1%2 != bitBuffer[LRCPos].fields.truchar.bit.b1)
    {
    	return;
    }
    if (contb2%2 != bitBuffer[LRCPos].fields.truchar.bit.b2)
	{
		return;
	}
    if (contb3%2 != bitBuffer[LRCPos].fields.truchar.bit.b3)
	{
		return;
	}
    if (contb4%2 != bitBuffer[LRCPos].fields.truchar.bit.b4)
	{
		return;
	}*/

    // Traducir a caracteres --> si sale mal retornar un string vacio
    // Va a agregarlos a id --> id[0] = bitBuffer[SSPos+1].fields.truchar.char hasta id[7] = bitBuffer[SSPos+8].fields.truchar.char

    uint8_t value4bits;
    for (int j = 0 ; j < sizeof(id)/sizeof(id[0]) ; j++) {
        value4bits = get_char_from_bits(bitBuffer[SSPos+j+1]);
        id[j] = make_char(value4bits);
    }
}

// calcula el char para los chunks de 5 bits por caracter leídos
static uint8_t get_char_from_bits(truchar_t t) {
    uint8_t r = t.fields.truchar.raw;  // b0...b4 (b0 más significativo === Paridad)
    uint8_t val = 0;

    val |= ((r >> 0) & 0x01) << 0;
    val |= ((r >> 1) & 0x01) << 1;
    val |= ((r >> 2) & 0x01) << 2;
    val |= ((r >> 3) & 0x01) << 3;

    return val;   // char formado por b1..b4 (4 bits)
}

// Convierte un número de 4 bits en char
static uint8_t make_char(uint8_t value4bits)
{
    // value4 debe estar entre 0 y 9
    if (value4bits > 9)
        return '?';   // error o carácter inválido

    return '0' + value4bits;
}


// Limpia el buffer para la próxima lectura. De esta forma nos aseguramos que no tenga data basura de antes
// Se llama a esta función luego de procesar el id de la tarjeta leída
static void cleanBuffer(void) {
    for (int i = 0 ; i < sizeof(bitBuffer)/sizeof(bitBuffer[0]) ; i++) {
        bitBuffer[i].fields.truchar.raw = 0x00;
    }
    return;
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
            cleanBuffer();
        }
    }
}


// ==========================================================
// API
// ==========================================================

// cardAvailable solo me ayuda si quiero hacer un flujo mas claro y que no dependa solo de datos, sino de estados
// Puedo llamar de una a cardGet(), pero aporta mas claridad, no mucho mas
bool cardAvailable(void) {
    return cardIsReadyFlag;
}

// processCardData tiene acceso desde la App, y permite vincular la app con la
//función que hace todos los cálculos para procesar el buffer de datos

void processCardData(void) {

	if (cardIsReadyFlag)
	{

		processIdData();
		cardIsReadyFlag = 0;
		timeoutCounter = 0;
		lastBitTime = 0;
		bitCount = 0;
		// Limpiar el buffer
		cleanBuffer();

	}
	lastCard = calculateIdNumber();     // Actualiza lastCard con el valor de id de la tarjeta procesada

    return;
}




uint32_t cardRead(void)
{
	uint32_t cardHolder = lastCard;
	for (int i = 0; i<(sizeof(id)/sizeof(id[0])); i++)
	{
		id[i] = 0; // chars de id
	}
	lastCard = 0; // int de id
    return cardHolder;                // Devuelve el último Id calculado
}
