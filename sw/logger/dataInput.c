#include "main.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx.h"
#include "endUserConfig.h"
#include "dataInput.h"
#include "buffer_helpers.h"
#include "logsystem.h"

#include "usbd_cdc_vcp.h"

volatile uint8_t  dataAvailable = 0;
volatile uint8_t*  bufferToWriteToSd;
uint8_t  firstDataBlockInLog = 1;

uint8_t dataInputBuffer[2][RX_BUFFER_SIZE];
uint8_t dataValidationBuffer[RX_BUFFER_SIZE];



void dataInput_Init(void) {
  Debug(INF,"Data input low-level initialization\n");
  dataInput_ClockAndGPIO_Init();
  dataInput_Peripherals_Init();

  Debug(INF,"Data input DMA initialization\n");
  dataInput_DMA_Init(PORT_TYPE(inputPort),
                     periphAddr[inputPort],
                     dmaStreamAddr[inputPort],
                     dmaChannAddr[inputPort]);

  Debug(INF,"Data input NVIC initialization\n");
  dataInput_NVIC_Configuration_DMA(dmaIrqChannAddr[inputPort]);

  Debug(INF,"Data input initialization finished.\n");
}

void dataInput_WhileSdOk(void) {
  if (dataAvailable) {
      dataAvailable = 0;
      Debug(INF,"Data available in buffer.\n");


      WriteStartHook();

      //uint8_t *bufferToWriteAddr = dataInputBuffer[!DMA_GetCurrentMemoryTarget(dmaStreamAddr[inputPort])];

      //VCP_DataTx(bufferToWriteAddr, RX_BUFFER_SIZE);

      uint32_t syncBytes = (firstDataBlockInLog) ? SYNCBYTES_LOGSTART : SYNCBYTES_LOGCONTINUE;
      firstDataBlockInLog = 0;

      SWAP(syncBytes);

      uint8_t n;
      for (n=0; n<SYNCBYTES_NUM; n++) {
          bufferToWriteToSd[n] = 0xAA;
      }

      uint16_t logNrLE = logNumber;
      SWAP(logNrLE);

      memcpy((uint8_t*)bufferToWriteToSd,   &syncBytes, 4);
      memcpy((uint8_t*)bufferToWriteToSd+4, &logNrLE, 2);
      memcpy((uint8_t*)bufferToWriteToSd+6, &inputPort, 1);

      //dataInputBuffer[!DMA_GetCurrentMemoryTarget(dmaStreamAddr[inputPort])][1] = 0xA2;

      SdStatus = SD_WriteMultiBlocks((uint8_t*)bufferToWriteToSd,
                                     blockToWrite,
                                     BLOCK_SIZE,
                                     RX_BUFFER_NUM_BLOCKS);

      //Wait until the write operation is finished
      SdStatus = SD_WaitWriteOperation();
      while(SD_GetStatus() != SD_TRANSFER_OK);

      WriteStopHook();

      if (rewriteAttempts > 0) {
        if (SdStatus == SD_OK) {
          SdStatus = SD_ReadMultiBlocks(dataValidationBuffer, blockToWrite, BLOCK_SIZE, RX_BUFFER_NUM_BLOCKS);
          // Check if the Transfer is finished
          SdStatus = SD_WaitReadOperation();
          while(SD_GetStatus() != SD_TRANSFER_OK);
        }

        Debug(INF,"Validating write\n");
        TestStatus TransferStatus1 = Buffercmp(dataValidationBuffer,
                                               dataInputBuffer[DMA_GetCurrentMemoryTarget(dmaStreamAddr[inputPort])],
                                               RX_BUFFER_SIZE);
        if (TransferStatus1 == PASSED) {
          Debug(INF,"Data valid!\n");
        }
        else {
          Debug(ERR,"Data invalid! Not trying to rewrite due to unfinished code.\n");
        }
      }

      blockToWrite += RX_BUFFER_NUM_BLOCKS*512;
  }
}

void dataInput_ClockAndGPIO_Init(void) {
  GPIO_InitTypeDef  GPIO_InitStructure;

  //// Clocks for GPIO
  // SPI1
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  // SPI2
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  //// Clocks for peripherals
  // SPI1
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  // SPI2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  // UART2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  // UART3
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  //// Alternative function pins
  // SPI1
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource4 ,GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5 ,GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6 ,GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7 ,GPIO_AF_SPI1);

  // SPI2
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource12 ,GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13 ,GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource14 ,GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15 ,GPIO_AF_SPI2);

  // UART2
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2 ,GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3 ,GPIO_AF_USART2);

  // UART3
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10 ,GPIO_AF_USART3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11 ,GPIO_AF_USART3);

  //// GPIO pin general configuration
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

  //// GPIO pin configuration
  // SPI1
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // SPI2
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // UART2
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // UART3
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void dataInput_Peripherals_Init(void) {
  //// SPI
  SPI_InitTypeDef   SPI_InitStructure;

  SPI_InitStructure.SPI_Direction     = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_NSS           = SPI_NSS_Soft;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode          = SPI_Mode_Slave;

  //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;

  SPI_I2S_DeInit(SPI1);
  SPI_I2S_DeInit(SPI2);

  SPI_InitStructure.SPI_DataSize      = SPI_DATASIZE(configSpi1);
  SPI_InitStructure.SPI_CPOL          = SPI_CPOL(configSpi1);
  SPI_InitStructure.SPI_CPHA          = SPI_CPHA(configSpi1);
  SPI_InitStructure.SPI_FirstBit      = SPI_FIRSTBIT(configSpi1);
  SPI_Init(SPI1, &SPI_InitStructure);


  SPI_InitStructure.SPI_DataSize      = SPI_DATASIZE(configSpi2);
  SPI_InitStructure.SPI_CPOL          = SPI_CPOL(configSpi2);
  SPI_InitStructure.SPI_CPHA          = SPI_CPHA(configSpi2);
  SPI_InitStructure.SPI_FirstBit      = SPI_FIRSTBIT(configSpi2);
  SPI_Init(SPI2, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
  SPI_Cmd(SPI2, ENABLE);


  //// USART
  USART_InitTypeDef   USART_InitStructure;

  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_InitStructure.USART_BaudRate = configUsart2.baudrate;
  USART_Init(USART2, &USART_InitStructure);
  USART_Cmd(USART2, ENABLE);

  USART_InitStructure.USART_BaudRate = configUsart3.baudrate;
  USART_Init(USART3, &USART_InitStructure);
  USART_Cmd(USART3, ENABLE);
}

void dataInput_DMA_Init(uint8_t PeriphType, void* Periph, DMA_Stream_TypeDef* DmaStream,uint32_t DmaChannel) {
  DMA_InitTypeDef   dataInput_DMA_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  //// Clear HalfTransfer and TransferComplete flags
  if (PeriphType == PERIPH_SPI && DmaStream == DMA2_Stream0)
    DMA_ClearFlag(DmaStream, DMA_FLAG_HTIF0 | DMA_FLAG_TCIF0); //SPI1_Rx
  else if (PeriphType == PERIPH_SPI && DmaStream == DMA1_Stream3)
    DMA_ClearFlag(DmaStream, DMA_FLAG_HTIF3 | DMA_FLAG_TCIF3); //SPI2_Rx
  else if (PeriphType == PERIPH_USART && DmaStream == DMA1_Stream5)
    DMA_ClearFlag(DmaStream, DMA_FLAG_HTIF5 | DMA_FLAG_TCIF5); //USART2_Rx
  else if (PeriphType == PERIPH_USART && DmaStream == DMA1_Stream1)
    DMA_ClearFlag(DmaStream, DMA_FLAG_HTIF1 | DMA_FLAG_TCIF1); //USART3_Rx

  DMA_Cmd(DmaStream, DISABLE);

  // Disable the data-in DMA requests
  if (PeriphType == PERIPH_SPI) {
    SPI_DMACmd((SPI_TypeDef*)Periph, SPI_DMAReq_Rx, DISABLE);
    dataInput_DMA_InitStructure.DMA_PeripheralBaseAddr      = (uint32_t)(&((SPI_TypeDef*)Periph)->DR);
  }
  else if (PeriphType == PERIPH_USART) {
    USART_DMACmd((USART_TypeDef*)Periph, SPI_DMAReq_Rx, DISABLE);
    dataInput_DMA_InitStructure.DMA_PeripheralBaseAddr      = (uint32_t)(&((USART_TypeDef*)Periph)->DR);
  }

  DMA_DeInit(DmaStream);

  dataInput_DMA_InitStructure.DMA_Channel                 = DmaChannel;

  dataInput_DMA_InitStructure.DMA_DIR                     = DMA_DIR_PeripheralToMemory;
  dataInput_DMA_InitStructure.DMA_Memory0BaseAddr         = (uint32_t)&(dataInputBuffer[0][0]) + SYNCBYTES_NUM;
  //dataInput_DMA_InitStructure.DMA_Memory0BaseAddr         = (uint32_t)dataInputBuffer;
  //dataInput_DMA_InitStructure.DMA_BufferSize              = RX_BUFFER_SIZE;
  dataInput_DMA_InitStructure.DMA_BufferSize              = (RX_BUFFER_SIZE - SYNCBYTES_NUM)/2;
  dataInput_DMA_InitStructure.DMA_PeripheralInc           = DMA_PeripheralInc_Disable;
  dataInput_DMA_InitStructure.DMA_MemoryInc               = DMA_MemoryInc_Enable;
  //dataInput_DMA_InitStructure.DMA_MemoryInc               = DMA_MemoryInc_Disable;
  dataInput_DMA_InitStructure.DMA_PeripheralDataSize      = DMA_PeripheralDataSize_HalfWord;
  dataInput_DMA_InitStructure.DMA_MemoryDataSize          = DMA_MemoryDataSize_HalfWord;
  dataInput_DMA_InitStructure.DMA_Mode                    = DMA_Mode_Circular;
  dataInput_DMA_InitStructure.DMA_Priority                = DMA_Priority_VeryHigh;
  dataInput_DMA_InitStructure.DMA_FIFOMode                = DMA_FIFOMode_Enable;
  //dataInput_DMA_InitStructure.DMA_FIFOMode                = DMA_FIFOMode_Disable;
  dataInput_DMA_InitStructure.DMA_FIFOThreshold           = DMA_FIFOThreshold_HalfFull;
  dataInput_DMA_InitStructure.DMA_MemoryBurst             = DMA_MemoryBurst_Single;
  dataInput_DMA_InitStructure.DMA_PeripheralBurst         = DMA_PeripheralBurst_Single;

  //DMA_DoubleBufferModeConfig(DmaStream,(uint32_t)&dataInputBuffer[0],DMA_Memory_0);
  //DMA_DoubleBufferModeConfig(DmaStream,(uint32_t)&dataInputBuffer[1],DMA_Memory_1);

  //DMA_DoubleBufferModeConfig(DmaStream,(uint32_t)&dataInputBuffer[0] + SYNCBYTES_NUM,DMA_Memory_0);
  DMA_DoubleBufferModeConfig(DmaStream, (uint32_t)&(dataInputBuffer[1][0]) + SYNCBYTES_NUM, DMA_Memory_1);

  DMA_DoubleBufferModeCmd(DmaStream, ENABLE);

  DMA_Init(DmaStream, &dataInput_DMA_InitStructure);

  // Enable the Data in DMA requests
  if (PeriphType == PERIPH_SPI)
    SPI_DMACmd((SPI_TypeDef*)Periph, SPI_DMAReq_Rx, ENABLE);
  else if (PeriphType == PERIPH_USART)
    USART_DMACmd((USART_TypeDef*)Periph, USART_DMAReq_Rx, ENABLE);

  // Enable the DMA stream
  DMA_Cmd(DmaStream, ENABLE);
  DMA_ITConfig(DmaStream, DMA_IT_TC, ENABLE);
}

void dataInput_NVIC_Configuration_DMA(uint32_t DmaIrqChannel) {
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = DmaIrqChannel; //DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_EnableIRQ(DmaIrqChannel);
}

void dataInput_DMA_Rx_IRQHandler(DMA_Stream_TypeDef* DmaStream, uint32_t ItFlags) {
  if(DMA_GetITStatus(DmaStream, ItFlags)) {
    DMA_ClearITPendingBit(DmaStream, ItFlags);
    dataAvailable = 1;
    bufferToWriteToSd = dataInputBuffer[!DMA_GetCurrentMemoryTarget(dmaStreamAddr[inputPort])];
  }
}

