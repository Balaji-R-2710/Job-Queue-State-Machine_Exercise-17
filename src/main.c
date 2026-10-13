#include "sdk_project_config.h"
#include <stdio.h>
#include <stdint.h>
#include "nvm.h"
#include "eeprom.h"

#define BLOCK_COUNT 3U

lpi2c_master_state_t lpi2cMasterState;
//lpuart_state_t lpUartState0;

static const uint8_t sampleData[BLOCK_COUNT][BLOCK_SIZE] =
{
    {10U, 20U, 30U, 20U},
    {50U, 60U, 70U, 80U},
    {90U, 100U, 110U, 120U}
};

int main(void)
{
    uint8_t i;
    uint8_t j;
    uint8_t readData[BLOCK_SIZE];
    uint8_t readSuccess;
    uint8_t match;
    uint8_t queued;
    NvM_State previousState;
    NvM_State currentState;
    char txBuff[128];
    int len;

    CLOCK_DRV_Init(&clockMan1_InitConfig0);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0,g_pin_mux_InitConfigArr0);
    LPUART_DRV_Init(INST_LPUART_1, &lpUartState0, &lpuart_0_InitConfig0);
    LPI2C_DRV_MasterInit(0U, &lpi2c0_MasterConfig0, &lpi2cMasterState);

    len = sprintf(txBuff, "\r\n========== NvM JOB QUEUE DEMO ==========\r\n");
    LPUART_DRV_SendDataBlocking(INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U );

    len = sprintf(txBuff,"NvM Initialization Successful\r\n\r\n");
    LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);

    /* Queue all requests before processing any of them */
    for (i = 0U; i < BLOCK_COUNT; i++)
    {
        queued = NvM_WriteBlock( (uint16_t)(i + 1U), sampleData[i], BLOCK_SIZE );

        if (queued != 0U)
        {
            len = sprintf(txBuff, "Block %u: Write request queued\r\n", (unsigned int)(i + 1U));
        }
        else
        {
            len = sprintf(txBuff, "Block %u: Queue request FAILED\r\n", (unsigned int)(i + 1U));
        }

        LPUART_DRV_SendDataBlocking(INST_LPUART_1,(const uint8_t *)txBuff,(uint32_t)len,1000U );

        if (queued == 0U)
        {
        	 len = sprintf(txBuff,"Cannot continue: queue request failed.\r\n");
        	 LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);
            while (1) {}
        }
    }
    len = sprintf(txBuff,"\r\n");
    LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);

    previousState = (NvM_State)255U;

    /* Process the queued jobs */
    while (NvM_IsIdle() == 0U)
    {
        NvM_MainFunction();

        currentState = NvM_GetState();

        if (currentState != previousState)
        {
            switch (currentState)
            {
                case IDLE:
                	 len = sprintf(txBuff,"State: IDLE\r\n\r\n");
                	 LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);
                    break;

                case WRITE_PENDING:
                	 len = sprintf(txBuff,"State: WRITE_PENDING\r\n");
                	 LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);
                    break;

                case WRITE_BUSY:
                    len = sprintf( txBuff, "State: WRITE_BUSY\r\nBlock %u: EEPROM write started\r\n", (unsigned int)NvM_GetActiveBlockId() );
                    LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U );
                    break;

                case DONE:
                    if (NvM_LastJobSucceeded() != 0U)
                    {
                        len = sprintf( txBuff, "Block %u: EEPROM write completed\r\n",(unsigned int)NvM_GetActiveBlockId());
                    }
                    else
                    {
                        len = sprintf( txBuff, "Block %u: EEPROM write FAILED\r\n", (unsigned int)NvM_GetActiveBlockId() );
                    }

                    LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);

                    len = sprintf(txBuff,"State: DONE\r\n");
                    LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);
                    break;

                default:
                    break;
            }
            previousState = currentState;
        }
    }
    len = sprintf(txBuff,"\r\nAll write requests processed.\r\n");
    LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);
    len = sprintf(txBuff,"Queue is empty.\r\n");
    LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);
    len = sprintf(txBuff,"\r\n========== EEPROM READ-BACK TEST ==========\r\n");
    LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);

    for (i = 0U; i < BLOCK_COUNT; i++)
    {
        readSuccess = EEPROM_Read(GetEepromAddress((uint16_t)(i + 1U)), readData, BLOCK_SIZE );

        if (readSuccess == 0U)
        {
            len = sprintf(txBuff, "Block %u: READ ERROR\r\n", (unsigned int)(i + 1U));

            LPUART_DRV_SendDataBlocking( INST_LPUART_1,(const uint8_t *)txBuff,(uint32_t)len,1000U );
            continue;
        }

        match = 1U;

        len = sprintf(txBuff, "Block %u: ", (unsigned int)(i + 1U));

        for (j = 0U; j < BLOCK_SIZE; j++)
        {
            len += sprintf( &txBuff[len], "%u ", (unsigned int)readData[j] );

            if (readData[j] != sampleData[i][j])
            {
                match = 0U;
            }
        }

        len += sprintf( &txBuff[len], "%s\r\n", (match != 0U) ? "| PASS" : "| FAIL" );

        LPUART_DRV_SendDataBlocking( INST_LPUART_1, (const uint8_t *)txBuff, (uint32_t)len, 1000U);
    }

    while (1){}
}
