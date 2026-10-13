#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>
#include "nvm.h"

typedef enum
{
    EEPROM_OK,
    EEPROM_PENDING,
    EEPROM_ERROR
} EEPROM_Status;

EEPROM_Status EEPROM_StartWrite(uint16_t address, const uint8_t *data, uint8_t length);

EEPROM_Status EEPROM_PollWrite(void);

uint8_t EEPROM_Read(uint16_t address, uint8_t *data, uint8_t length);

uint16_t GetEepromAddress(uint16_t blockId);

#endif
