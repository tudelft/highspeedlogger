#ifndef PANIC_H_
#define PANIC_H_

#include "leds.h"

// Blocking panic
static inline void PANIC(char* message, uint32_t delay, uint16_t led) {
  Debug(FTL,message);
  while (1) {
      LED_On(led);
      Delay(delay);
      LED_Off(led);
      Delay(delay);
  }
}

#endif /* PANIC_H_ */
