#ifndef __MAIN_H
#define __MAIN_H

//#include "system_stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include <stdio.h>
#include "sdio_sd.h"

/********************  SETTINGS  ********************/
// SDIO Settings
#define BLOCK_SIZE              512   /* SD card block size in Bytes (512 for a normal SD card) */

// Logger settings
//#define RX_BUFFER_NUM_BLOCKS    40 /* 40 blocks * 512 = 20.48 KB RAM required per buffer*/
#define RX_BUFFER_NUM_BLOCKS    20 /* 20 blocks * 512 = 10 KB RAM required per buffer*/

// Debug settings
#define DEBUG_LEVEL             INF
/**************   END OF SETTINGS   *****************/


// Buffer sizes
#define RX_BUFFER_SIZE                  RX_BUFFER_NUM_BLOCKS * BLOCK_SIZE
#define TX_BUFFER_SIZE                  RX_BUFFER_SIZE

// Size of packet counter in bytes
#define COUNTER_SIZE                    sizeof(packetCounter)

// Debug levels
#define FTL                             0
#define ERR                             1
#define WRN                             2
#define INF                             3
#define DBG                             4

extern void Debug(uint8_t level, char* message);


extern void DMA_UART2RX_IRQHandler(void);
extern void WriteStopHook(void);
extern void WriteStartHook(void);

extern volatile uint32_t packetCounter;
//extern volatile uint8_t  RxBuffer[NUM_BUFFERS] [RX_BUFFER_SIZE];
extern uint8_t  SDIO_Rx_Buffer[RX_BUFFER_SIZE];
extern SD_Error SdStatus;
extern uint32_t blockToWrite;

#endif /* __MAIN_H */

