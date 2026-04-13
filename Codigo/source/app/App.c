/***************************************************************************/ /**
   @file     App.c
   @brief    Application functions
   @author   Nicolás Magliola
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "app/display_intensity.h"
#include "app/id_input.h"
#include "app/pass_input.h"
#include "app/user.h"
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

// #define DEBUG
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
typedef enum {
  INITIAL,
  INPUT_ID,
  INPUT_PASS,
  DISPLAY_INTENSITY,
} AppState;

void resetState(AppState* appState, bool* firstRun);

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
#ifdef ADMIN
  if (initUserSystem()) {
    ledOn(RED); // fallo al inicializar userDataset
  }
#endif // ADMIN
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void) {

  static AppState appState = INITIAL;
  static char id[ID_LEN] = {0};
  static char pass[PASS_LEN] = {0};
  static bool firstRun = true;
  static bool fullPass = 0; // es 1 si son 5 caracteres
  static int8_t idxDataset = 0;
  // can't use `timerCreate` in static initialization so I need to create manually, idk how to
  // it better.
  static timer_t openLockTimer = {.startMillis = 0, .durationMillis = 5000, .started = false};

  // TODO: use `wheelInputFlag`
  wheel_input_t result = readWheel();
  switch (appState) {
  case INITIAL:
    if (firstRun) {
      writeString("CLICK TO LOG IN - DOUBLE CLICK FOR INTENSITY");
      firstRun = false;
    }
    if (result == CLICK) {
      appState = INPUT_ID;
      initIdInput();
    } else if (result == DOUBLECLICK) {
      appState = DISPLAY_INTENSITY;
      cleanDisplay();
      writeString("0000");
    }
    break;
  case INPUT_ID: {
    IdInputState idInputState = handleIdInput(id, result);
    if (idInputState == ID_CANCELLED) {
      resetState(&appState, &firstRun);
    } else if (idInputState == ID_CONFIRMED) {
      if (searchId(id, &idxDataset)) {
        appState = INPUT_PASS;
        initPassInput();
      } else {
        writeString("WRONG USER");
        resetState(&appState, &firstRun);
      }
    }
    break;
  }
  case INPUT_PASS: {
    PassInputState passInputState = handlePassInput(pass, result, &fullPass);
    if (passInputState == PASS_CANCELLED) {
      resetState(&appState, &firstRun);
      ledOn(RED);
    }
    if (passInputState == PASS_CONFIRMED) {
      if (checkPass(&idxDataset, pass, fullPass)) {
        if (!openLockTimer.started) {
          timerStart(&openLockTimer);
          turnOnDisplayLed(0);
          turnOnDisplayLed(2);
        } else {
          if (timerFinished(&openLockTimer)) {
            turnOffDisplayLed(0);
            turnOffDisplayLed(2);
            turnOnDisplayLed(0);
            resetState(&appState, &firstRun);
          }
        }
      } else {
        cleanDisplay();
        writeString("BAD PASS");
      }
    }
    break;
  }
  case DISPLAY_INTENSITY: {
    IntensityState intensityState = handleDisplayIntensity(result);
    if (intensityState == INTENSITY_CONFIRM) {
      resetState(&appState, &firstRun);
    }
    break;
  }
  }
  /*char algo[]= "hola";
  if (!gpioRead(PIN_ENABLE_DATA))
  {
          //ledToggle(RED);
  }
  uint32_t idReal = 60612683;
  uint32_t id;
  if (cardAvailable())
   {
          //ledOn(BLUE);
           processCardData();
           id = cardRead(algo);
           if (id == idReal)
          {
                  ledOn(GREEN);
                  id = 0;
          }
          else
          {
                  ledOn(RED);
          }
   }*/
}

void resetState(AppState* appState, bool* firstRun) {
  *appState = INITIAL;
  *firstRun = true;
  return;
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
