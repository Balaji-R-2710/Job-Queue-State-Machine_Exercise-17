#include "nvm.h"
#include "eeprom.h"
#include <stddef.h>

static NvM_State nvmState = IDLE;
static WriteRequest activeJob;
static EEPROM_Status jobResult = EEPROM_PENDING;
static uint8_t lastJobSucceeded = 0U;

uint8_t NvM_WriteBlock(uint16_t blockId, const uint8_t *data, uint8_t length)
{
    WriteRequest request;
    uint8_t i;

    if ((blockId == 0U) || (blockId > MAX_BLOCK_ID) || (data == NULL) || (length != BLOCK_SIZE))
    {
        return 0U;
    }

    request.blockId = blockId;
    request.length = length;

    for (i = 0U; i < length; i++)
    {
        request.data[i] = data[i];
    }

    return NvM_Enqueue(&request);
}

void NvM_MainFunction(void)
{
    switch (nvmState)
    {
        case IDLE:
            if (NvM_Dequeue(&activeJob) != 0U)
            {
                jobResult = EEPROM_PENDING;
                lastJobSucceeded = 0U;
                nvmState = WRITE_PENDING;
            }
            break;

        case WRITE_PENDING:
            jobResult = EEPROM_StartWrite( GetEepromAddress(activeJob.blockId), activeJob.data, activeJob.length );

            if (jobResult == EEPROM_PENDING)
            {
                nvmState = WRITE_BUSY;
            }
            else
            {
                lastJobSucceeded = 0U;
                nvmState = DONE;
            }
            break;

        case WRITE_BUSY:
            jobResult = EEPROM_PollWrite();

            if (jobResult == EEPROM_OK)
            {
                lastJobSucceeded = 1U;
                nvmState = DONE;
            }
            else if (jobResult == EEPROM_ERROR)
            {
                lastJobSucceeded = 0U;
                nvmState = DONE;
            }
            else
            {
                /* EEPROM is still busy */
            }
            break;

        case DONE:
            nvmState = IDLE;
            break;

        default:
            nvmState = IDLE;
            break;
    }
}

uint8_t NvM_IsIdle(void)
{
    if ((nvmState == IDLE) && (NvM_QueueIsEmpty() != 0U))
    {
        return 1U;
    }

    return 0U;
}

NvM_State NvM_GetState(void)
{
    return nvmState;
}

uint16_t NvM_GetActiveBlockId(void)
{
    return activeJob.blockId;
}

uint8_t NvM_LastJobSucceeded(void)
{
    return lastJobSucceeded;
}
