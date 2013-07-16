/*
 * endUserConfig.h
 *
 *  Created on: Feb 6, 2013
 *      Author: dirk
 */

#ifndef ENDUSERCONFIG_H_
#define ENDUSERCONFIG_H_
#include "stm32f4xx.h"
#include "stm32f4xx_dma.h"
#include "dataInput.h"

#define PORT_SPI1       0
#define PORT_SPI2       1
#define PORT_UART2      2
#define PORT_UART3      3

extern void* periphAddr[4];
extern DMA_Stream_TypeDef* dmaStreamAddr[4];
extern uint32_t dmaChannAddr[4];
extern uint8_t dmaIrqChannAddr[4];



#define PORT_TYPE(_p) (_p == PORT_SPI1 || _p == PORT_SPI2) ? PERIPH_SPI : PERIPH_USART

#define SPI_DATASIZE_8  8
#define SPI_DATASIZE_16 16

#define SPI_CPOL_LOW    0
#define SPI_CPOL_HIGH   1

#define SPI_CPHA_1EDGE  1
#define SPI_CPHA_2EDGE  2

#define SPI_FIRSTB_MSB  0
#define SPI_FIRSTB_LSB  1

#define SPI_DATASIZE(_c) ((_c.datasize == SPI_DATASIZE_8) ? SPI_DataSize_8b : SPI_DataSize_16b)
#define SPI_CPOL(_c)     ((_c.cpol == SPI_CPOL_LOW)       ? SPI_CPOL_Low : SPI_CPOL_High)
#define SPI_CPHA(_c)     ((_c.cpha == SPI_CPHA_1EDGE)     ? SPI_CPHA_1Edge : SPI_CPHA_2Edge)
#define SPI_FIRSTBIT(_c) ((_c.firstbit == SPI_FIRSTB_MSB) ? SPI_FirstBit_MSB : SPI_FirstBit_LSB)

#define BS_GET(_b)   (boolSettings & (1<<_b))
#define BS_SET(_b)   (boolSettings = boolSettings | (1<<_b))
#define BS_RESET(_b) (boolSettings = boolSettings & (~(1<<_b)))

#define BS_AUTOFORMAT_SD 0

/// END-USER CONFIG ////////////////
extern uint8_t inputPort;
extern uint8_t rewriteAttempts;
extern uint32_t boolSettings;


struct spiPeriphConfig {
  uint8_t datasize;
  uint8_t cpol;
  uint8_t cpha;
  uint8_t firstbit;
};

struct usartPeriphConfig {
  uint32_t baudrate;
};

extern struct spiPeriphConfig   configSpi1;
extern struct spiPeriphConfig   configSpi2;
extern struct usartPeriphConfig configUsart2;
extern struct usartPeriphConfig configUsart3;

#endif /* ENDUSERCONFIG_H_ */
