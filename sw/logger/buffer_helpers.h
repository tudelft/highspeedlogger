#ifndef BUFFER_HELPERS_H_
#define BUFFER_HELPERS_H_

#include <stdlib.h>
#include <stdint.h>

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

extern TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);
extern void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);
extern TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength);
extern inline void SwapBytes(void *pv, size_t n);


#define SWAP(x) SwapBytes(&x, sizeof(x));

#endif /* BUFFER_HELPERS_H_ */
