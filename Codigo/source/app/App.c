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

// FOR INIT
#include "hal/board.h"
#include "hal/card.h"
#include "hal/display.h"
#include "hal/leds.h"
#include "hal/system.h"
#include "hal/timers.h"
#include "hal/wheel.h"
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

typedef struct app_t {
  AppState state;
  char id[ID_LEN];
  char pass[PASS_LEN];
  bool firstRun;
  bool fullPass;
  bool passVerified;
  int8_t idxDataset;
  timer_t openLockTimer;
  timer_t changePassTimer;
  timer_t invalidIdTimer;
  timer_t badPassTimer;
} app_t;

void resetState(app_t* app);

void App_Init(void) {
  initSystem(1000);
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
  initTimers();
#ifdef ADMIN
  if (initUserSystem()) {
    ledOn(RED); // fallo al inicializar userDataset
  }
#endif // ADMIN
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void) {
  static app_t app = {
      .state = INITIAL,
      .id = {0},
      .pass = {0},
      .passVerified = false,
      .firstRun = true,
      .fullPass = false,
      .idxDataset = 0,
      .openLockTimer = {.startMillis = 0, .durationMillis = 5000, .started = false},
      .changePassTimer = {.startMillis = 0, .durationMillis = 5000, .started = false},
      .invalidIdTimer = {.startMillis = 0, .durationMillis = 5000, .started = false},
      .badPassTimer = {.startMillis = 0, .durationMillis = 5000, .started = false},
  };
  // static AppState appState = INITIAL;
  // static char id[ID_LEN] = {0};
  // static char pass[PASS_LEN] = {0};
  // static bool firstRun = true;
  // static bool fullPass = 0; // es 1 si son 5 caracteres
  // static int8_t idxDataset = 0;
  // // can't use `timerCreate` in static initialization so I need to create manually, idk how to
  // // it better.
  // static timer_t openLockTimer = {.startMillis = 0, .durationMillis = 5000, .started = false};

  // TODO: use `wheelInputFlag`
  wheel_input_t result = readWheel();
  switch (app.state) {
  case INITIAL:
    if (app.firstRun) {
      writeString("CLICK TO LOG IN - DOUBLE CLICK FOR INTENSITY");
      app.firstRun = false;
      ledOn(BLUE);
    }
    if (result == CLICK) {
      app.state = INPUT_ID;
      initIdInput();
    } else if (result == DOUBLECLICK) {
      app.state = DISPLAY_INTENSITY;
      cleanDisplay();
      writeString("0000");
    }
    break;
  case INPUT_ID: {
    IdInputState idInputState = handleIdInput(app.id, result);
    if (idInputState == ID_CANCELLED) {
      resetState(&app);
    } else if (idInputState == ID_CONFIRMED) {
      if (searchId(app.id, &app.idxDataset)) {
        app.state = INPUT_PASS;
        initPassInput();
      } else {
        if (!app.invalidIdTimer.started) {
          timerStart(&app.invalidIdTimer);
          writeString("INVALID ID");
        } else if (timerFinished(&app.invalidIdTimer)) {
          resetState(&app);
        }
      }
    }
    break;
  }
  case INPUT_PASS: {
    PassInputState passInputState = handlePassInput(app.pass, result, &app.fullPass);
    if (passInputState == PASS_CANCELLED) {
      resetState(&app);
      ledOn(RED);
    } else if (passInputState == PASS_CONFIRMED) {
      if (app.passVerified || checkPass(&app.idxDataset, app.pass, app.fullPass)) {
        if (!app.openLockTimer.started) {
          app.passVerified = true;
          timerStart(&app.openLockTimer);
          turnOnDisplayLed(0);
          turnOnDisplayLed(2);
          writeString("DC TO CHANGE PASS");
        } else if (timerFinished(&app.openLockTimer)) {
          turnOffDisplayLed(0);
          turnOffDisplayLed(2);
          resetState(&app);
        }
      } else {
        if (!app.badPassTimer.started) {
          timerStart(&app.badPassTimer);
          writeString("BAD PASS");
        } else if (timerFinished(&app.badPassTimer)) {
          resetState(&app);
        }
      }
    } else if (passInputState == PASS_CHANGE) {
      user_t* user = &userDataset[app.idxDataset];
      user->fullPass = app.fullPass;
      changePass(user, app.pass);
      if (!app.changePassTimer.started) {
        timerStart(&app.changePassTimer);
        turnOffDisplayLed(0);
        turnOffDisplayLed(2);
        turnOnDisplayLed(1);
        writeString("PASS CHANGED");
      } else if (timerFinished(&app.changePassTimer)) {
        turnOffDisplayLed(1);
        resetState(&app);
      }
    }
    break;
  }
  case DISPLAY_INTENSITY: {
    IntensityState intensityState = handleDisplayIntensity(result);
    if (intensityState == INTENSITY_CONFIRM) {
      resetState(&app);
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

void resetState(app_t* app) {
  app->state = INITIAL;
  app->firstRun = true;
  app->passVerified = false;
  app->idxDataset = 0;
  app->openLockTimer.started = false;
  app->changePassTimer.started = false;
  app->invalidIdTimer.started = false;
  app->badPassTimer.started = false;
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
