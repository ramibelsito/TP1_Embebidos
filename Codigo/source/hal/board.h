/***************************************************************************//**
  @file     board.h
  @brief    Board management
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** BOARD defines **********************************************************/

// On Board User LEDs
#define PIN_LED_RED     PORTNUM2PIN(PB,22)// ???
#define PIN_LED_GREEN   PORTNUM2PIN(PE,26)// ???
#define PIN_LED_BLUE    PORTNUM2PIN(PB,21) // PTB21

#define LED_ACTIVE      LOW


// On Board User Switches
#define PIN_SW2         // ???
#define PIN_SW3         PORTNUM2PIN(PA,4) // PTA4

#define SW_ACTIVE       LOW // ???
#define SW_INPUT_TYPE   BOOL // ???

#define PIN_RCHA		PORTNUM2PIN(PA,2)
#define PIN_RCHB		PORTNUM2PIN(PC,2)
#define PIN_RCHD		PORTNUM2PIN(PC,3)

/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
