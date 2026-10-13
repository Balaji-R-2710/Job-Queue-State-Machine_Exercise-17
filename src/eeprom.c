#include "sdk_project_config.h"
#include "eeprom.h"
#include <stddef.h>

#define EEPROM_BASE_ADDR  0x50U
#define EEPROM_PAGE_SIZE  16U
#define EEPROM_TIMEOUT    100U

static uint8_t writeWaitRemaining = 0U;
static uint8_t writeInProgress = 0U;

static uint8_t EEPROM_GetDeviceAddress(uint16_t address)
{
    return (uint8_t)(EEPROM_BASE_ADDR | ((address >> 8U) & 0x01U));
}

EEPROM_Status EEPROM_StartWrite(uint16_t address, const uint8_t *data, uint8_t length)
{
    uint8_t buffer[BLOCK_SIZE + 1U];
    uint8_t i;
    uint8_t deviceAddress;
    status_t status;

    if ((writeInProgress != 0U) || (data == NULL) || (length == 0U) || (length > BLOCK_SIZE) || ((uint32_t)address + length > EEPROM_SIZE))
    {
        return EEPROM_ERROR;
    }

    /* Do not allow a write to cross a 16-byte page boundary */
    if (((address % EEPROM_PAGE_SIZE) + length) > EEPROM_PAGE_SIZE)
    {
        return EEPROM_ERROR;
    }
    deviceAddress = EEPROM_GetDeviceAddress(address);

    /* First byte is the memory address within the selected half */
    buffer[0] = (uint8_t)(address & 0xFFU);

    for (i = 0U; i < length; i++)
    {
        buffer[i + 1U] = data[i];
    }

    LPI2C_DRV_MasterSetSlaveAddr(0U, deviceAddress, false);

    status = LPI2C_DRV_MasterSendDataBlocking( 0U, buffer, (uint32_t)length + 1U, true, EEPROM_TIMEOUT);

    if (status != STATUS_SUCCESS)
    {
        return EEPROM_ERROR;
    }

    /* Wait for the EEPROM's internal write cycle */
    writeWaitRemaining = EEPROM_WRITE_MS;
    writeInProgress = 1U;

    return EEPROM_PENDING;
}

EEPROM_Status EEPROM_PollWrite(void)
{
    if (writeInProgress == 0U)
    {
        return EEPROM_ERROR;
    }

    if (writeWaitRemaining > 0U)
    {
        OSIF_TimeDelay(1U);
        writeWaitRemaining--;
    }

    if (writeWaitRemaining > 0U)
    {
        return EEPROM_PENDING;
    }

    writeInProgress = 0U;

    return EEPROM_OK;
}

uint8_t EEPROM_Read(uint16_t address, uint8_t *data, uint8_t length)
{
    uint8_t deviceAddress;
    uint8_t memoryAddress;
    status_t status;

    if ((writeInProgress != 0U) || (data == NULL) || (length == 0U) || ((uint32_t)address + length > EEPROM_SIZE))
    {
        return 0U;
    }

    deviceAddress = EEPROM_GetDeviceAddress(address);
    memoryAddress = (uint8_t)(address & 0xFFU);

    LPI2C_DRV_MasterSetSlaveAddr(0U, deviceAddress, false);

    /* Send the memory address without a STOP */
    status = LPI2C_DRV_MasterSendDataBlocking( 0U, &memoryAddress, 1U, false, EEPROM_TIMEOUT);
    if (status != STATUS_SUCCESS)
    {
        return 0U;
    }

    /* Receive the requested bytes */
    status = LPI2C_DRV_MasterReceiveDataBlocking( 0U, data, length, true, EEPROM_TIMEOUT);
    if (status != STATUS_SUCCESS)
    {
        return 0U;
    }

    return 1U;
}

uint16_t GetEepromAddress(uint16_t blockId)
{
    if ((blockId == 0U) || (blockId > MAX_BLOCK_ID))
    {
        return EEPROM_SIZE;
    }

    return (uint16_t)((blockId - 1U) * BLOCK_SIZE);
}
