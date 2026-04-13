#include "card.h"
#include "hal/board.h"
#include "leds.h"
#include  "mcal/gpio.h"
#include "hardware.h"
#include "hal/IRQN_Ports.h"
#include "mcal/SysTick.h"
#include <string.h>

//#define DEBUG

#define CARD_MAX      	   	 80
#define CARD_MAX_CHARS    	 40
#define SS 0b01011 // Start Sentinel ';' 
#define ES 0b11111 // End Sentinel '?'    
#define FS 0b01101 // Field Separator '=' 

#ifdef DEBUG
uint32_t debugCounter =0 ;
char myString[500] = "";
#endif // DEBUG



// Variables internas
static volatile uint8_t bitBuffer[CARD_MAX];

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
static uint32_t ESIdx = 0;

// LOCAL FUNCTIONS
static void processIdData(void);
static void checkTimeout(void);
static uint8_t get_char_from_bits(uint8_t r);
static uint8_t make_char(uint8_t value4bits);
static void cleanBufferAndId(void);
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
    pisr_register(timerForTimeout, 10);

    return true;
}



// ==========================================================
// HANDLER & CALLBACKS
// ==========================================================

// Aumenta su valor a medida que entra el systick. Sirve de referencia para veificar si se pasa un tiempo límite de espera
void timerForTimeout (void) {
	toggleInterruptFlag();
    timeoutCounter++;

    if(receiving) {
		checkTimeout();
    }
    toggleInterruptFlag();
}

// Handler para las interrupciones de la tarjeta
void cardHandler(void)
{
	toggleInterruptFlag();
    uint32_t flags = PORTC->ISFR;

    // ============================
    // CARD ENABLE
    // ============================
    if (flags & (1 << 0))
    {

        cleanFlags(PIN_ENABLE_DATA);

        receiving       = true;
        firstSSBit      = false;
        flagES          = false;
        ESIdx           = 0;

        bitCount        = 0;
        lastBitTime     = 0;
        timeoutCounter  = 0;

        cleanBufferAndId();

        return;
    }

    // ============================
    // CARD CLOCK
    // ============================
    if (flags & (1 << 5))
    {
        cleanFlags(PIN_CARD_CLOCK);

        if (!receiving)
            return;

        lastBitTime = timeoutCounter;

        bool bit = !gpioRead(PIN_CARD_DATA);

        // ============================================================
        // SHIFT REGISTER para sincronización (5 bits)
        // ============================================================
        static uint8_t shiftReg = 0;
        static uint8_t shiftCount = 0;

        shiftReg = (shiftReg >> 1) | (bit << 4);  // LSB first
        if (shiftCount < 5)
            shiftCount++;

        // ============================================================
        // BUSCAR START SENTINEL (SS) en cualquier alineación
        // ============================================================
        if (!firstSSBit)
        {
            if (shiftCount == 5 && shiftReg == SS)
            {
                firstSSBit = true;

                // Guardar SS como primer caracter
                bitBuffer[0] = shiftReg;

                bitCount = 5;   // ya tengo 1 char completo
            }
            return;
        }

        // ============================================================
        // YA SINCRONIZADO → ARMAR CARACTERES DE 5 BITS
        // ============================================================
        uint8_t idx = bitCount / 5;
        uint8_t pos = bitCount % 5;

        if (pos == 0)
            bitBuffer[idx] = 0;

        bitBuffer[idx] |= (bit << pos);
        bitCount++;

        // ============================================================
        // DETECTAR END SENTINEL (solo cuando el char está completo)
        // ============================================================
        if (pos == 4 && bitBuffer[idx] == ES)
        {
            flagES = true;
            ESIdx = idx;
        }

        // ============================================================
        // ES + LRC
        // ============================================================
        if (flagES && idx > ESIdx + 1)
        {
            receiving = false;
            cardIsReadyFlag = true;

            // Reset parcial
            bitCount       = 0;
            firstSSBit     = false;
            flagES         = false;
            shiftReg       = 0;
            shiftCount     = 0;

            lastBitTime    = 0;
            timeoutCounter = 0;

            return;
        }

        // ============================================================
        // PROTECCIÓN
        // ============================================================
        if (bitCount > 200)
        {
            receiving = false;
            bitCount  = 0;
            firstSSBit = false;
            flagES     = false;

            shiftReg   = 0;
            shiftCount = 0;

            cleanBufferAndId();
            return;
        }
    }
    toggleInterruptFlag();
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
    
	//ledToggle(BLUE);
    while ( bitBuffer[i] != ES)
    {
        if (bitBuffer[i] == SS)
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
                                        
    while ( SSPos+i < ESPos ) {
        count = ((bitBuffer[SSPos+i] >> 0) & 1)
                    + ((bitBuffer[SSPos+i] >> 1) & 1)
                    + ((bitBuffer[SSPos+i] >> 2) & 1)
                    + ((bitBuffer[SSPos+i] >> 3) & 1);

        // printf("Dígito %u: Count = %u, Paridad = %u \n", i,count,bitBuffer[SSPos+i].fields.truchar.bit.b0);
        
        if (!((count % 2 == 0 && ((bitBuffer[SSPos+i] >> 4) & 1) == 1) ||
            (count % 2 != 0 && ((bitBuffer[SSPos+i] >> 4) & 1) == 0))) {
            
            return;         // Detecta error --> Sale de la función
        }
        i++;
    }
    //ledToggle(BLUE);

    // Corroborar Paridad Vertical (LRC) 

    i = 0;
    count = 0;
    uint8_t j = 0;
    //uint8_t lrc = get_char_from_bits(bitBuffer[LRCPos]);

    // LRC tiene una longitud de 4 bits
    while (i < 4) {

        while ((SSPos + j) < LRCPos ) {
            count ^= (bitBuffer[SSPos+j] >> i) & 1;         // Si cuenta numeros pares de 1 --> count = 0, sino count = 1.
            j++;
        }

        if (count != ((bitBuffer[LRCPos] >> i) & 1)) {
            return;                      					// Detecta error --> Sale de la función
        }
        i++;
        j = 0;
        count = 0;
    }


    // Traducir a caracteres
    // Va a agregarlos a id

    uint8_t value4bits;
    for (int j = 0 ; j < sizeof(id)/sizeof(id[0]) ; j++) {
        value4bits = get_char_from_bits(bitBuffer[SSPos+j+1]);
        id[j] = make_char(value4bits);
    }
}

// calcula el char para los chunks de 5 bits por caracter leídos
static uint8_t get_char_from_bits(uint8_t r) {
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
        return '?';   // error o caracter inválido

    return '0' + value4bits;
}


// Limpia el buffer para la próxima lectura. De esta forma nos aseguramos que no tenga data basura de antes
// Se llama a esta función luego de procesar el id de la tarjeta leída
static void cleanBufferAndId(void) {
    for (int i = 0 ; i < sizeof(bitBuffer)/sizeof(bitBuffer[0]) ; i++) {
        bitBuffer[i] = 0;
    }
    for (int i = 0 ; i < sizeof(id)/sizeof(id[0]) ; i++) {
		id[i] = 0;
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
            // Si supera el tiempo de timeout deja de leer los datos y limpia las variables

            receiving 		= false;
            cardIsReadyFlag = false;
            bitCount       	= 0;
            firstSSBit     	= false;
            flagES         	= false;

            lastBitTime    	= 0;
            timeoutCounter 	= 0;

            cleanBufferAndId();
        }
    }
}


// ==========================================================
// API
// ==========================================================

// cardAvailable solo me ayuda si quiero hacer un flujo mas claro y que no dependa solo de datos, sino de estados
// Puedo llamar de una a processCardData(), pero aporta mas claridad, no mucho mas
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
		lastCard = calculateIdNumber();     // Actualiza lastCard con el valor de id de la tarjeta procesada

	}
}




uint32_t cardRead(char *pId)
{
	for (int j = 0; j < 8; j++)
	{
		pId[j] = id[j];
	}
	uint32_t cardHolder = lastCard;
	cleanBufferAndId();
	lastCard = 0;
	return cardHolder;              // Devuelve el último Id calculado
}
