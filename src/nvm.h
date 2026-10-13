#ifndef NVM_H
#define NVM_H

#include <stdint.h>

#define EEPROM_SIZE     512U
#define BLOCK_SIZE      4U
#define MAX_BLOCK_ID    5U
#define QUEUE_SIZE      8U
#define EEPROM_WRITE_MS 5U

typedef struct
{
    uint16_t blockId;
    uint8_t data[BLOCK_SIZE];
    uint8_t length;
} WriteRequest;

typedef enum
{
    IDLE,
    WRITE_PENDING,
    WRITE_BUSY,
    DONE
} NvM_State;

/* Add a block-write request to the queue */
uint8_t NvM_WriteBlock(uint16_t blockId, const uint8_t *data, uint8_t length);

/* Process one step of the state machine */
void NvM_MainFunction(void);

/* Check whether processing and the queue are idle */
uint8_t NvM_IsIdle(void);

/* Obtain the current state and active block */
NvM_State NvM_GetState(void);
uint16_t NvM_GetActiveBlockId(void);

/* Queue operations */
uint8_t NvM_Enqueue(const WriteRequest *request);
uint8_t NvM_Dequeue(WriteRequest *request);
uint8_t NvM_QueueIsEmpty(void);
uint8_t NvM_LastJobSucceeded(void);
#endif
