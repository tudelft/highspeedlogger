#include "logsystem.h"
#include "main.h"
#include "buffer_helpers.h"
#include "panic.h"
#include "endUserConfig.h"

uint32_t sdReadAddr;
uint8_t  sdReadBuff[BLOCK_SIZE];
uint8_t  sdWriteBuff[BLOCK_SIZE];

uint8_t sdDataChunkNumBlocks = 0;

uint32_t logStartBlock = 0;

uint16_t logNumber;
/*
LogsystemStatus logsystem_Init(void) {
  //SD_ReadMultiBlocks(sdReadBuff, blockToWrite, BLOCK_SIZE, RX_BUFFER_NUM_BLOCKS);

  sdReadAddr = 0;

  if (SD_ReadBlock(sdReadBuff, sdReadAddr, BLOCK_SIZE) != SD_OK)
    PANIC("LS can not read from SD Card", 3000, LED_SD_ERR);
  SdStatus = SD_WaitReadOperation();
  while(SD_GetStatus() != SD_TRANSFER_OK);

  //if (((sdReadBuff & SYNCBYTES_MASK) >> (BLOCK_SIZE-SYNCBYTES_NUM)) != SYNCBYTES_CARDINFO)
  uint32_t syncBytesToCheck = SYNCBYTES_CARDINFO;
  SWAP(syncBytesToCheck);

  if (Buffercmp(sdReadBuff, (uint8_t*)&syncBytesToCheck, SYNCBYTES_NUM) != PASSED) {
    if (BS_GET(BS_AUTOFORMAT_SD)) {
      Debug(WRN,"Formatting SD card using PPRZLS...\n");
      logsystem_formatCard();
      Debug(INF,"Formatting finished\n");
    }
    else {
      PANIC("SD card not formatted using PPRZLS. Formatter la carte du Securite Digitalle s.v.p.", 3000, LED_SD_ERR);
    }
  }
  else {
      Debug(INF,"SD card is already formatted using PPRZLS.\n");
      if (sdReadBuff[SYNCBYTES_NUM + IDX_DATA_CHUNK_NUM_BLOCKS] != RX_BUFFER_NUM_BLOCKS) {
          PANIC("Number of blocks per data chunk incorrect. La carte du Securite Digitalle has to be reformatted.", 6000, LED_SD_ERR);
      }
  }
}

 void logsystem_formatCard(void) {
  Debug(INF,"X\n");
  uint16_t n;
  for (n=0; n<BLOCK_SIZE; n++) {
      sdWriteBuff[n] = 0;
  }
  uint32_t syncBytes = SYNCBYTES_CARDINFO;
  SWAP(syncBytes);

  memcpy(sdWriteBuff, &syncBytes, SYNCBYTES_NUM);
  sdWriteBuff[SYNCBYTES_NUM + IDX_DATA_CHUNK_NUM_BLOCKS] = (uint8_t)RX_BUFFER_NUM_BLOCKS;


  SdStatus = SD_WriteBlock(sdWriteBuff, 0, BLOCK_SIZE);
  SdStatus = SD_WaitWriteOperation();

  while(SD_GetStatus() != SD_TRANSFER_OK);
}*/

LogsystemStatus logsystem_FindNextEmptyLogPos(void) {
  logNumber = 0;
  //sdReadAddr = BLOCK_SIZE;
  sdReadAddr = 0;
  uint8_t foundBlock = 0;

  uint32_t syncBytesToCheck = SYNCBYTES_LOG_OR_DATA;
  SWAP(syncBytesToCheck);


  while (sdReadAddr < (1<<31)) {
    if (SD_ReadBlock(sdReadBuff, sdReadAddr, BLOCK_SIZE) != SD_OK)
        PANIC("LS can not read from SD Card", 3000, LED_SD_ERR);

    SdStatus = SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);

    if (Buffercmp(sdReadBuff, (uint8_t*)&syncBytesToCheck, 3) != PASSED) {
        Debug(INF,"Found free position for next log.\n");
        foundBlock = 1;
        logStartBlock = sdReadAddr/512;
        break;
    }
    else {
        sdReadAddr += BLOCK_SIZE*RX_BUFFER_NUM_BLOCKS;
        if (sdReadBuff[3] == (SYNCBYTES_LOGSTART & 0xFF))
          logNumber++;
    }
  }
  if (!foundBlock) {
      PANIC("SD card full.", 1000, LED_IN_BUF);
  }

  blockToWrite = sdReadAddr;
}


