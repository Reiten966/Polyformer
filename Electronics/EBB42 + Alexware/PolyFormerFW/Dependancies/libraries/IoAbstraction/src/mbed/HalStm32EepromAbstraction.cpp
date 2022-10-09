/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifdef IOA_ENABLE_STM32_HAL_EXTRAS

#include <IoLogging.h>
#include "HalStm32EepromAbstraction.h"

void HalStm32EepromAbstraction::enableBackupRam()
{
    HAL_PWR_EnableBkUpAccess();  // access to back-up domain..
    __HAL_RCC_PWR_CLK_ENABLE();  // enable the clock

    errorOccurred = HAL_PWREx_EnableBkUpReg() != HAL_OK;   // enable the backup regulator

    serlogF2(SER_IOA_INFO, "STM32 Backup RAM enabled status: ", errorOccurred);
}

void HalStm32EepromAbstraction::initialise(int baseOffs) {
    serlogF3(SER_IOA_DEBUG, "Initialise STM32 backup RAM based ROM (offs, size): ", baseOffs, EEPROM_SIZE);

    romBase = baseOffs;
    enableBackupRam();
    halReadFromCache();
}

void HalStm32EepromAbstraction::halWriteToCache() {
    __HAL_RCC_BKPSRAM_CLK_ENABLE(); // turn on back up ram clock

    auto* dataCache = reinterpret_cast<uint32_t*>(eepromBuffer);
    for(uint32_t i=0; i<EEPROM_WORD_SIZE; i++) {
        *(uint32_t*)(BKPSRAM_BASE + romBase + (i<<2)) = dataCache[i];
    }

    __HAL_RCC_BKPSRAM_CLK_DISABLE(); // turn off backup ram clock

    serlogF(SER_IOA_DEBUG, "Completed write to cache of present data: ");
}

void HalStm32EepromAbstraction::halReadFromCache() {
    __HAL_RCC_BKPSRAM_CLK_ENABLE(); // enable back up ram clock

    auto* dataCache = reinterpret_cast<uint32_t*>(eepromBuffer);
    for(uint32_t i=0; i<EEPROM_WORD_SIZE; i++) {
        dataCache[i] = *(uint32_t*)(BKPSRAM_BASE + romBase + (i<<2));
    }

    __HAL_RCC_BKPSRAM_CLK_DISABLE();

    serlogF(SER_IOA_DEBUG, "Completed read into cache of backup data: ");
}

uint8_t HalStm32EepromAbstraction::read8(EepromPosition position) {
    if(position >= EEPROM_SIZE) {
        errorOccurred = true;
        return 0;
    }
    return eepromBuffer[position];
}

void HalStm32EepromAbstraction::write8(EepromPosition position, uint8_t val) {
    if(position >= EEPROM_SIZE) {
        errorOccurred = true;
    }
    else {
        eepromBuffer[position] = (char)val;
    }
}

uint16_t HalStm32EepromAbstraction::read16(EepromPosition position) {
    if(position + 2 >= EEPROM_SIZE) {
        errorOccurred = true;
        return 0;
    }
    return eepromBuffer[position] | ((uint16_t)eepromBuffer[position + 1] << 8);
}

void HalStm32EepromAbstraction::write16(EepromPosition position, uint16_t val) {
    if (position + 2 >= EEPROM_SIZE) {
        errorOccurred = true;
    } else {
        eepromBuffer[position] = char(val & 0xffU);
        eepromBuffer[position + 1] = char(val >> 8U);
    }
}

uint32_t HalStm32EepromAbstraction::read32(EepromPosition position) {
    if(position + 4 >= EEPROM_SIZE) {
        errorOccurred = true;
        return 0;
    }
    return eepromBuffer[position] | ((uint32_t)eepromBuffer[position + 1] << 8U) | ((uint32_t)eepromBuffer[position + 2] << 16U) |
                ((uint32_t)eepromBuffer[position + 3] << 24U);
}

void HalStm32EepromAbstraction::write32(EepromPosition position, uint32_t val) {
    if(position + 2 >= EEPROM_SIZE) {
        errorOccurred = true;
    }
    else {
        eepromBuffer[position] = char(val & 0xff);
        eepromBuffer[position + 1] = char((val >> 8U) & 0xffU);
        eepromBuffer[position + 2] = char((val >> 16U) & 0xffU);
        eepromBuffer[position + 3] = char((val >> 24U));
    }
}

void HalStm32EepromAbstraction::readIntoMemArray(uint8_t *memDest, EepromPosition romSrc, uint8_t len) {
    if((romSrc + len) > EEPROM_SIZE) {
        errorOccurred = true;
        memDest[0] = 0;
    }
    else {
        memcpy(memDest, eepromBuffer + romSrc, len);
    }
}

void HalStm32EepromAbstraction::writeArrayToRom(EepromPosition romDest, const uint8_t *memSrc, uint8_t len) {
    if((romDest + len) > EEPROM_SIZE) {
        errorOccurred = true;
    }
    else {
        memcpy(eepromBuffer + romDest, memSrc, len);
    }
}

bool HalStm32EepromAbstraction::hasErrorOccurred() {
    return errorOccurred;
}

#endif
