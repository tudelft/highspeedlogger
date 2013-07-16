#ifndef DATAINPUT_H_
#define DATAINPUT_H_

#include "main.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx.h"

extern void dataInput_Init(void);
extern void dataInput_WhileSdOk(void);
extern void dataInput_ClockAndGPIO_Init(void);
extern void dataInput_Peripherals_Init(void);
extern void dataInput_DMA_Init(uint8_t PeriphType, void* Periph, DMA_Stream_TypeDef* DmaStream,uint32_t DmaChannel);
extern void dataInput_NVIC_Configuration_DMA(uint32_t DmaIrqChannel);
extern void dataInput_DMA_Rx_IRQHandler(DMA_Stream_TypeDef* DmaStream, uint32_t ItFlags);

extern volatile uint8_t  dataAvailable;


#define PERIPH_SPI      0
#define PERIPH_USART    1

#endif /* DATAINPUT_H_ */
