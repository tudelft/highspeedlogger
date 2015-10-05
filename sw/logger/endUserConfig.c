/*
 * endUserConfig.h
 *
 *  Created on: Feb 6, 2013
 *      Author: dirk
 */

#include "stm32f4xx.h"
#include "stm32f4xx_dma.h"
#include "dataInput.h"
#include "endUserConfig.h"

void* periphAddr[4] = { (SPI_TypeDef*)  SPI1,
                        (SPI_TypeDef*)  SPI2,
                        (USART_TypeDef*)USART2,
                        (USART_TypeDef*)USART3 };

DMA_Stream_TypeDef* dmaStreamAddr[4] = { DMA2_Stream0,
                                         DMA1_Stream3,
                                         DMA1_Stream5,
                                         DMA1_Stream1 };

uint32_t dmaChannAddr[4] = { DMA_Channel_3,
                             DMA_Channel_0,
                             DMA_Channel_4,
                             DMA_Channel_4};

uint8_t dmaIrqChannAddr[4] = { DMA2_Stream0_IRQn,
                               DMA1_Stream3_IRQn,
                               DMA1_Stream5_IRQn,
                               DMA1_Stream1_IRQn};


/// END-USER CONFIG ////////////////
uint8_t inputPort = PORT_SPI2;
//uint8_t inputPort = PORT_UART2;
uint8_t rewriteAttempts = 1;
uint32_t boolSettings = 0x00000001;


struct spiPeriphConfig   configSpi1 =
{
  SPI_DATASIZE_8,
  SPI_CPOL_HIGH,
  SPI_CPHA_2EDGE,
  SPI_FIRSTB_MSB
};
struct spiPeriphConfig   configSpi2 =
{
  SPI_DATASIZE_8,
  SPI_CPOL_HIGH,
  SPI_CPHA_2EDGE,
  SPI_FIRSTB_MSB
};
struct usartPeriphConfig configUsart2 = {230400};
struct usartPeriphConfig configUsart3 = {230400};

