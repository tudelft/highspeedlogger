#ifndef DELAY_H
#define DELAY_H

//#include "core_cm4.h"

static __IO uint32_t TimingDelay;

//void Delay(__IO uint32_t nCount);

void Delay(uint32_t nCount) {
  TimingDelay = nCount;
  while(TimingDelay--) { }
}

void TimingDelay_Decrement(void) {
  if (TimingDelay != 0x00)   {
    TimingDelay--;
  }
}



#endif
