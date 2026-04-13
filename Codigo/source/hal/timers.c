#include "hal/timers.h"
#include "mcal/SysTick.h"
#include <stdint.h>

uint64_t ticks = 0;

static void updateTick();

void initTimers() {
  pisr_register(updateTick, 5);
}

timer_t timerCreate(uint32_t durationMillis) {
  return (timer_t){.startMillis = 0, .durationMillis = durationMillis, .started = false};
}

void timerStart(timer_t* timer) {
  timer->startMillis = ticks;
  timer->started = true;
}

bool timerFinished(timer_t* timer) {
  return (ticks - timer->startMillis) >= timer->durationMillis;
}

static void updateTick() {
  ++ticks;
}
