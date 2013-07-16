#include "leds.h"

void LED_LowLevel_Init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = LED1 | LED2 | LED3 | LED4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_SetBits(GPIOB, LED1 | LED2 | LED3 | LED4);
}

void LED_On(uint16_t led) {
  GPIO_ResetBits(GPIOB, led);
}
void LED_Off(uint16_t led) {
  GPIO_SetBits(GPIOB, led);
}
