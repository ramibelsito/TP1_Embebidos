#include "mcal/gpio.h"


#define PIN_RCHA	// Purto del canal A de la ruedita
#define PIN_RCHB	// Puerto del canal B de la ruedita
#define PIN_RCHC 	// Puerto del canal C de la ruedita


#define WHEEL_READ_TIME	500

#define BUFFER_SIZE		20 			// Number of elements of encoderData

#define TWO_BIT_MASK 	0x03

bool wheelInputFlag;

uint8_t validPrev;
enum {LEFT, RIGHT};

bool dir = LEFT;


typedef union {
    uint8_t raw;
    struct {
        uint8_t rcha :1;
        uint8_t rchb :1;
        uint8_t rchd :1;
        uint8_t free :5;
    };
} encoder_t;

encoder_t encoderData[BUFFER_SIZE];

uint32_t index = 0;		// Global index to move through encoderData
uint16_t move = 0;









bool wheelInit()
{
	// Inicializar dos puertos para leer los canales A y B de la ruedita
	if(!gpioInit(PIN_RCHA)) {
		return false;		// Si no se inicializa bien, devuelve un false
	}
	if(!gpioInit(PIN_RCHB)) {
		return false;		// Si no se inicializa bien, devuelve un false
	}
	if(!gpioInit(PIN_RCHC)) {
		return false;		// Si no se inicializa bien, devuelve un false
	}
	gpioMode(PIN_RCHA, INPUT);
	gpioMode(PIN_RCHB, INPUT);
	gpioMode(PIN_RCHC, INPUT);

	gpioRegister(wheelReadGPIO, WHEEL_READ_TIME);		// Read wheel data every WHEEL_READ_TIME (ms)

}

bool wheelReadGPIO(void) {
	encoderData[index].rcha = gpioRead(PIN_RCHA);
	encoderData[index].rchb = gpioRead(PIN_RCHB);
	encoderData[index].rchd = gpioRead(PIN_RCHD);
	encoderData[index].free = 0;

	index++;
	if(index > BUFFER_SIZE - 1) {			// Index lives between 0 and 19
		index = index % BUFFER_SIZE;
	}
}


uint32_t readWheel()
{
	// Compares last 2 bits to know if there is a new value
	if(((validPrev & TWO_BIT_MASK) != (encoderData[index] & TWO_BIT_MASK))) {
		switch(validPrev & TWO_BIT_MASK) {
			case 0x00:
				if((encoderData[index] & TWO_BIT_MASK) == 0x02) {
					move++;
					dir = RIGHT;
				}
				else if((encoderData[index] & TWO_BIT_MASK) == 0x01) {
					move++;
					dir = LEFT;
				}
				else if((encoderData[index] & TWO_BIT_MASK) == 0x03) {
					move = 2;
				}
				break;
			case 0x01:
				if(encoderData[index] & TWO_BIT_MASK) {
					move++;
				}
				else if(dir == LEFT && (encoderData[index] & TWO_BIT_MASK) == 0x02) {
					move = move+2;
				}
				else if(encoderData[index] == 0x00) {
					move = 0;
				}
				break;
			case 0x02:
				if((encoderData[index] & TWO_BIT_MASK) == 0x00) {
					move++;
				}
				else if(dir == LEFT && (encoderData[index] & TWO_BIT_MASK) == 0x01) {
					move = 4;
				}
				else if(dir == RIGHT && (encoderData[index] & TWO_BIT_MASK) == 0x03) {
					move++;
				}
				break;
			case 0x03:
				if((encoderData[index] & TWO_BIT_MASK) == 0x02) {
					dir = LEFT;
					move++;
				}
				else if((encoderData[index] & TWO_BIT_MASK) == 0x01) {
					dir = RIGHT;
					move++;
				}
				else if((encoderData[index] & TWO_BIT_MASK) == 0x00) {
					move = 2;
				}
				break;
		}
		validPrev = (encoderData[index] & 0x03);
	}

	if(move>3) {
		return ((dir == RIGHT)? RIGHTTURN : LEFTTURN);
	}
	else {
		return IDLE;
	}
}



