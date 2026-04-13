/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Nicolás Magliola
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "app/action.h"

#include "app/display_intensity.h"
#include "app/id_input.h"
#include "app/utils.h"
#include "hal/timers.h"

// FOR INIT
#include "hal/board.h"
#include "hal/card.h"
#include "hal/display.h"
#include "hal/leds.h"
#include "hal/wheel.h"
#include "mcal/SysTick.h"
#include "mcal/gpio.h"
#include <stdbool.h>
#include <stdint.h>

//#define DEBUG
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

void App_Init(void) {
  SysTick_Init(1000);
  initDisplay();
  if (ledsInit(WHITE)) {
	 ledOff(WHITE);
	 return;
	}

  if (wheelInit() == false) {
	  ledOn(RED);
	}
  if (!cardInit()) {
	  ledOn(RED);
	}
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
#ifdef DEBUG
  cleanDisplay();
  writeCharacter('5', 3, 1);
  writeCharacter('0', 2, 0);
  writeCharacter('2', 1, 1);
  writeCharacter('7', 0, 0);
  cleanDisplay();
  writeString("HELLO WORLD");
  cleanDisplay();
  writeSegments(0b10000000, 0);

#endif //DEBUG
}

typedef enum {
  INITIAL,
  INPUT_ID,
  INPUT_PASS,
  DISPLAY_INTENSITY,
} AppState;

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void) {
#ifndef DEBUG
  static AppState appState = INITIAL;
  static char id[ID_LEN] = {0};
  static char pass[PASS_LEN] = {0};

  /*if (!gpioRead(PIN_ENABLE_DATA)) {
    // ledOn(GREEN);
  }*/

  wheel_input_t result = readWheel();
  switch (appState) {
  case INITIAL:
    writeString("CLICK TO LOG IN - DOUBLE CLICK FOR INTENSITY");
    if (result == CLICK) {
      appState = INPUT_ID;
      cleanDisplay();
    } else if (result == DOUBLECLICK) {
      appState = DISPLAY_INTENSITY;
      writeString("0000");
    }
    break;
  case INPUT_ID: {
    IdInputState idInputState = handleIdInput(id, result);
    if (idInputState == ID_CANCELLED) appState = INITIAL;
    else if (idInputState == ID_CONFIRMED) appState = INPUT_PASS;
    break;
  }
  case INPUT_PASS: {
    // IdInputState idInputState = handleIdInput(id, result);
    // if (idInputState == ID_CANCELLED) appState = INITIAL;
    // if (idInputState == ID_CONFIRMED) appState = INPUT_PASS;
    break;
  }
  case DISPLAY_INTENSITY:
    handleDisplayIntensity(result);
    break;
  }
#endif // NOT DEBUG
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
