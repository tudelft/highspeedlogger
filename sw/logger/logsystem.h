/*
 * filesystem.h
 *
 *  Created on: Feb 8, 2013
 *      Author: dirk
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "main.h"

#define SYNCBYTES_LOG_OR_DATA  0x1A1B1C00
#define SYNCBYTES_LOGSTART     (SYNCBYTES_LOG_OR_DATA | 0xAA)
#define SYNCBYTES_LOGCONTINUE  (SYNCBYTES_LOG_OR_DATA | 0xBB)

#define SYNCBYTES_NUM          64
#define SYNCBYTES_MASK         0xffffffff

#define SPAREBYTES_NUM         4

#define IDX_DATA_CHUNK_NUM_BLOCKS       0

typedef enum {UNINITIALIZED     = 0,
              CARD_UNFORMATTED  = 1,
              CARD_OK           = 2,
              READ_ERROR        = 9
              } LogsystemStatus;

extern uint16_t logNumber;
extern void logsystem_formatCard(void);
extern LogsystemStatus logsystem_Init(void);
extern LogsystemStatus logsystem_FindNextEmptyLogPos(void);
#endif /* FILESYSTEM_H_ */
