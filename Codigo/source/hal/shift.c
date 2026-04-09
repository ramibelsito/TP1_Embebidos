#include "hal/shift.h"
#include "mcal/gpio.h"
#include <stdint.h>

#define PIN_SHIFT PORTNUM2PIN(PB, 23)
#define PIN_CLK PORTNUM2PIN(PA, 1)    // serial clock
#define PIN_SETOUT PORTNUM2PIN(PB, 9) // latch clock

#define OUTCANT 16

static uint16_t shiftRegister;

uint16_t shiftRead() {
  return shiftRegister;
}

void shiftWriteWord(uint16_t value) {
  shiftRegister = value;
}

void shiftWriteBit(int index, bool state) {
  if (state) shiftRegister |= (1 << index);
  else shiftRegister &= ~(1 << index);
}

bool shiftOutUpdate() {
  // TODO: averiguar si se puede hacer más eficiente con SPI.
  for (int i = 15; i >= 0; i--) {
    gpioWrite(PIN_SHIFT, (shiftRegister >> i) & 1);
    gpioWrite(PIN_CLK, 1);
    gpioWrite(PIN_CLK, 0);
  }

  gpioWrite(PIN_SETOUT, 1);
  gpioWrite(PIN_SETOUT, 0);

  // TODO: should handle errors?
  return true;
}

bool shiftInit() {
  gpioInit(PIN_SHIFT);
  gpioMode(PIN_SHIFT, OUTPUT);
  gpioInit(PIN_CLK);
  gpioMode(PIN_CLK, OUTPUT);
  gpioWrite(PIN_CLK, 0);
  gpioInit(PIN_SETOUT);
  gpioMode(PIN_SETOUT, OUTPUT);
  gpioWrite(PIN_SETOUT, 0);

  // TODO: should handle errors?
  return true;
}
