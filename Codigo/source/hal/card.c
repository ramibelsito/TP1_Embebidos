#include "card.h"
#include "hal/board.h"
#include "leds.h"
#include  "mcal/gpio.h"
#include "hardware.h"
#include "hal/IRQN_Ports.h"

#define CARD_MAX_BITS      320
#define CARD_MAX_CHARS     40
#define CARD_TIMEOUT_MS    50   

// Se puede acceder al char completo como x.fields.truchar.raw o podes acceder bit a bit como x.fields.truchar.bit.b0
// Hay que agregar al union una forma de acceder solo al bit paridad y solo a los 4 bits de caracter
typedef union
{
    uint8_t raw;
    struct
    {
        union
        {
            uint8_t raw : 5;
            struct
            {
                uint8_t b0 : 1;
                uint8_t b1 : 1;
                uint8_t b2 : 1;
                uint8_t b3 : 1;
                uint8_t b4 : 1;
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
char id[8];
uint32_t timeoutCounter = 0;                    // Suma cada vez que entra al pisr. Compara con lastBitTime

static volatile bool receiving = false;         // True cuando detecta el flanco de enable de la tarjeta

bool cardReadFlag = false;                      // True cuando ya se tiene el ID (ya termino de procesar processBuffer())
static uint32_t lastCard = 0;


// LOCAL FUNCTIONS
static void processBuffer(void);
static int decodeF2F(uint8_t *in, uint16_t len, uint8_t *out);
static bool checkLRC(uint8_t *decoded, int totalChars);
static void checkTimeout(void);

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

void timerForTimeout (void) {
    timeoutCounter++;
}


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
void processIdData(truchar_t bitBuffer[])
{
    bool SSFound = false;
    bool FSFound = false;
    uint8_t SSPos = 0;
    uint8_t FSPos = 0;
    uint8_t i = 0;
    
    while ( bitBuffer[i].raw != ES)
    {
        if (bitBuffer[i].raw == SS)
        {
            SSFound = true;
            SSPos = i;
        }
        if (bitBuffer[i].raw == FS)
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
    // Corroborar Paridad Horizontal --> si sale mal retornar un string vacio

    // Corroborar Paridad Vertical (LRC) --> si sale mal retornar un string vacio 

    // Traducir a caracteres --> si sale mal retornar un string vacio
    // Va a agregarlos a id --> id[0] = bitBuffer[SSPos+1].fields.truchar.char hasta id[7] = bitBuffer[SSPos+8].fields.truchar.char




}
/*
static void processBuffer(void) {
    uint8_t decoded[CARD_MAX_BITS];
    int decodedLen = decodeF2F((uint8_t*)bitBuffer, bitCount, decoded);

    if (decodedLen <= 0)
        return;

    // Validación de longitud
    if (decodedLen % 5 != 0)
        return;

    int totalChars = decodedLen / 5;

    if (totalChars < 2) // mínimo: ; ... ? + LRC
        return;

    // Validar LRC
    if (!checkLRC(decoded, totalChars))
        return;

    char chars[CARD_MAX_CHARS];
    int charCount = 0;

    // Decodificación + PARIDAD
    for (int i = 0; i < decodedLen; i += 5)
    {
        uint8_t val = 0;
        uint8_t ones = 0;

        for (int j = 0; j < 4; j++)
        {
            uint8_t bit = decoded[i + j];
            val |= (bit << j);
            if (bit) ones++;
        }

        uint8_t parity = decoded[i + 4];
        if (parity) ones++;

        // Paridad impar
        if ((ones % 2) == 0)
        {
            return; // Error
        }

        char c = val + '0';

        // Validación de caracteres
        if (!(c >= '0' && c <= '9') &&
            c != ';' && c != '?' && c != '=')
        {
            return;
        }

        if (charCount >= CARD_MAX_CHARS)
            return;

        chars[charCount++] = c;
    }

    // Buscar frame válido
    int start = -1, end = -1;

    for (int i = 0; i < charCount; i++)
    {
        if (chars[i] == ';' && start == -1)
            start = i;

        if (chars[i] == '?')
        {
            end = i;
            break;
        }
    }

    if (start == -1 || end == -1 || end <= start)
        return;

    // Extraer ID
    uint32_t id = 0;

    for (int i = start + 1; i < end; i++)
    {
        if (chars[i] >= '0' && chars[i] <= '9')
        {
            id = id * 10 + (chars[i] - '0');
        }
        else if (chars[i] == '=')
        {
            break; // fin del PAN
        }
        else
        {
            return; // carácter inválido dentro del ID
        }
    }

    if (id != 0)
    {
        lastCard = id;
        cardReadFlag = true;
    }
}

// ==========================================================
// DECODIFICACIÓN F2F 
// ==========================================================

static int decodeF2F(uint8_t *in, uint16_t len, uint8_t *out)
{
    if (len < 2)
        return 0;

    int outIndex = 0;

    for (int i = 1; i < len; i++)
    {
        // Detecta transición
        uint8_t transition = (in[i] != in[i - 1]);

        // Transición -> 1
        // No transición -> 0
        out[outIndex++] = transition ? 1 : 0;
    }

    return outIndex;
}


static bool checkLRC(uint8_t *decoded, int totalChars) {
    uint8_t lrc[5] = {0};

    // XOR columna por columna
    for (int i = 0; i < totalChars - 1; i++) // excluye LRC
    {
        for (int j = 0; j < 5; j++)
        {
            lrc[j] ^= decoded[i * 5 + j];
        }
    }

    // comparar con LRC recibido (último char)
    for (int j = 0; j < 5; j++)
    {
        if (lrc[j] != decoded[(totalChars - 1) * 5 + j])
            return false;
    }

    return true;
}

// ==========================================================
// TIMEOUT
// ==========================================================

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
        processBuffer();
        cardDataReady = false;
    }
}



uint32_t cardRead(void)
{
    if (!cardReadFlag)
        return 0;

    cardReadFlag = false;           // Limpiar el flag

    processBuffer();                // Procesa los datos y calcula el Id

    return lastCard;                // Devuelve el último Id calculado
}

*/