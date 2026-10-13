#include "nvm.h"
#include <stddef.h>

static WriteRequest jobQueue[QUEUE_SIZE];

static uint8_t queueHead = 0U;
static uint8_t queueTail = 0U;
static uint8_t queueCount = 0U;

uint8_t NvM_Enqueue(const WriteRequest *request)
{
    uint8_t i;
    if ((request == NULL) || (queueCount >= QUEUE_SIZE))
    {
        return 0U;
    }

    jobQueue[queueTail] = *request;
    queueTail++;

    if (queueTail >= QUEUE_SIZE)
    {
        queueTail = 0U;
    }
    queueCount++;

    return 1U;
}

uint8_t NvM_Dequeue(WriteRequest *request)
{
    if ((request == NULL) || (queueCount == 0U))
    {
        return 0U;
    }
    *request = jobQueue[queueHead];

    queueHead++;

    if (queueHead >= QUEUE_SIZE)
    {
        queueHead = 0U;
    }
    queueCount--;
    return 1U;
}

uint8_t NvM_QueueIsEmpty(void)
{
    return (queueCount == 0U) ? 1U : 0U;
}
