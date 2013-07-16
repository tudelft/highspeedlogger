#include "main.h"
#include <stdio.h>
#include <stdlib.h>

#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"

#include "buffer_helpers.h"

#include "leds.h"

#include "sdio_sd.h"

#include "logsystem.h"

#include "panic.h"

#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"

uint32_t blockToWrite = 0;

SD_Error SdStatus = SD_OK;

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;
char debugLevelDescrs[5][3] = { "FTL", "ERR", "WRN", "INF", "DBG" } ;

int main(void) {
  SystemInit();

  // Configure time for 1ms tick
  //SysTick_Config(168000000 / 1000);

  LED_LowLevel_Init();

  USBD_Init(&USB_OTG_dev,
            USB_OTG_FS_CORE_ID,
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);

  Debug(INF,"Powered on.\n");

  //LED_On(LED_READY);

  // Enable SDIO interrupts
  NVIC_Configuration_SDIO();
  Debug(INF,"SDIO interrupts enabled.\n");

  volatile uint8_t nRetries = 10;
  while(nRetries--) {
    if((SdStatus = SD_Init()) != SD_OK) {
      Debug(WRN,"SDIO interface init failed. Retrying...\n");
      if (nRetries == 1)
        PANIC("SDIO initialization failed, even after 10 attempts\n",0xfffff,LED_SD_ERR);
    }
    else {
      LED_On(LED_READY);
      Debug(INF,"SDIO interface init succeeded.\n");
      nRetries = 0;
    }
    LED_On(LED_SD_ERR);
    Delay(0x100000);
    LED_Off(LED_SD_ERR);
    Delay(0xAAAAAA);
  }


  Delay(0xffff);

  dataInput_Init();

  while(SdStatus != SD_OK) {}

  //logsystem_Init();

  logsystem_FindNextEmptyLogPos();
  // todo set blockToWrite

  while(SdStatus == SD_OK) {
     LED_On(LED_IN_BUF);
     dataInput_WhileSdOk();
     LED_Off(LED_IN_BUF);
  }
}

void Debug(uint8_t level, char* message) {
 if (level <= DEBUG_LEVEL){
  char newMessage[sizeof(message)+8+1];
  newMessage[0] = 'L';
  newMessage[1] = 'G';
  newMessage[2] = 'R';
  newMessage[3] = ':';

  newMessage[7] = ':';

  memcpy((char*)newMessage+4, (char*)debugLevelDescrs[level], 3);
  memcpy((char*)newMessage+8, (char*)message, sizeof(message));
  char newline = '\n';
  memcpy((char*)newMessage+8+sizeof(message), (char*)&newline, 1);
  VCP_send_str(message);
 }
}

/*void SetBufferCounterBytes(uint8_t *bufferPointer) {
  bufferPointer[0] = (uint8_t)((packetCounter & 0xff000000) >>  (3*8));
  bufferPointer[1] = (uint8_t)((packetCounter & 0x00ff0000) >>  (2*8));
  bufferPointer[2] = (uint8_t)((packetCounter & 0x0000ff00) >>  (1*8));
  bufferPointer[3] = (uint8_t)((packetCounter & 0x000000ff));
}*/

void WriteStartHook(void) {
  LED_On(LED_SD_WR);
  Debug(INF,"Writing....\n");
}
void WriteStopHook(void) {
  LED_Off(LED_SD_WR);
  Debug(INF,"Done.\n");
}
