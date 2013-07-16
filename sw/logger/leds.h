#ifndef LEDS_H
#define LEDS_H

#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"

#define LED1                GPIO_Pin_6 /* blue */
#define LED2                GPIO_Pin_7 /* red */
#define LED3                GPIO_Pin_8 /* orange */
#define LED4                GPIO_Pin_9 /* green */

#define LED_SD_ERR          LED2
#define LED_SD_WR           LED3
#define LED_IN_BUF          LED1
#define LED_READY           LED4

extern void LED_LowLevel_Init(void);
extern void LED_On(uint16_t led);
extern void LED_Off(uint16_t led);

#endif
