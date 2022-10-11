/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * Support for STM32F4xx battery backed up RAM as an IoAbstraction EepromAbstraction. This implementation always caches
 * the EEPROM memory into RAM to avoid having to enable and disable the clock on each read. However, you could easily
 * make a local version that instead just read on demand. By default it caches the first 512 bytes which is usually
 * more than enough for most menu cases. If not define EEPROM_WORD_SIZE as the number of 32-bit words you need.
 *
 * Part of IoAbstraction extras for mbed on STM32, requires definition of IOA_ENABLE_STM32_HAL_EXTRAS to be included
 *
 * @file HalStm32EepromAbstraction.h
 */
#if !defined(IOA_HALSTM32EEPROMABSTRACTION_H) && defined(IOA_ENABLE_STM32_HAL_EXTRAS)
#define IOA_HALSTM32EEPROMABSTRACTION_H

#include "EepromAbstraction.h"

//
// By default, we cache the first 512 bytes of ROM into memory, we cache it to avoid having to leave the clock on
// for longer periods. It is possible to remove this cache. You can increase or reduce its size below.
#ifndef EEPROM_SIZE
#define EEPROM_SIZE (EEPROM_WORD_SIZE * 4)
#define EEPROM_WORD_SIZE 128
#endif

/**
 * An implementation of the EepromAbstraction that works with the STM32 HAL functions to read and write values to
 * the internal battery backed memory. This has been tested with STM32F4 boards and is known to work with mbed 6.
 *
 * This implementation caches the data from the battery backed memory into regular RAM, to avoid having to manage
 * the RAMs clock frequently. After making adjustments on the object you call commit to push it into ROM.
 *
 * Regular usage is to globally create an instance of the class and then call `initialise(offset)` where offset is
 * the zero based offset from the start of memory at which to start writing.
 */
class HalStm32EepromAbstraction : public EepromAbstraction {
private:
    char eepromBuffer[EEPROM_SIZE];
    uint16_t romBase;
    bool errorOccurred;
public:
    ~HalStm32EepromAbstraction() override = default;

    /**
     * Initialise the EEPROM, caching the current values from backup RAM starting at baseOffs.
     * @param baseOffs the offset from the start of ROM
     */
    void initialise(int baseOffs);

    /**
     * Refresh the cache should you have written behind it
     */
    void refresh() { halReadFromCache(); }

    /**
     * Commit the values now in cache to backup
     */
    void commit() { halWriteToCache(); }

    /**
     * Reads an 8-bit value from the cache
     * @param position the position of the variable
     * @return the value associated with the position
     */
    uint8_t read8(EepromPosition position) override;

    /**
     * Writes an 8-bit value to the cache, use commit() to push to backup ram.
     * @param position the position of the variable
     * @param val the value to associate with the position
     */
    void write8(EepromPosition position, uint8_t val) override;

    /**
     * Reads a 16-bit value from the cache
     * @param position the position of the variable
     * @return the value associated with the position
     */
    uint16_t read16(EepromPosition position) override;

    /**
     * Writes a 16-bit value to the cache, use commit() to push to backup ram.
     * @param position the position of the variable
     * @param val the value to associate with the position
     */
    void write16(EepromPosition position, uint16_t val) override;

    /**
     * Reads a 32-bit value from the cache
     * @param position the position of the variable
     * @return the value associated with the position
     */
    uint32_t read32(EepromPosition position) override;

    /**
     * Writes a 32-bit value to the cache, use commit() to push to backup ram.
     * @param position the position of the variable
     * @param val the value to associate with the position
     */
    void write32(EepromPosition position, uint32_t val) override;

    /**
     * Reads from the cache into memory at the requested destination location
     * @param memDest the destination memory
     * @param romSrc the source offset in the ROM cache
     * @param len the length to read
     */
    void readIntoMemArray(uint8_t *memDest, EepromPosition romSrc, uint8_t len) override;

    /**
     * Writes data into the ROM cache that must be committed using commit() later.
     * @param romDest the position in ROM cache
     * @param memSrc the memory location to read from
     * @param len the length to be written
     */
    void writeArrayToRom(EepromPosition romDest, const uint8_t *memSrc, uint8_t len) override;

    /**
     * Indicates if an error has occurred. IE if the regulator failed to enable or a write was outside of bounds
     * @return true if there has been an error, otherwise false.
     */
    bool hasErrorOccurred() override;

private:
    void halReadFromCache();
    void halWriteToCache();
    void enableBackupRam();
};

#endif //IOA_HALSTM32EEPROMABSTRACTION_H or STM32 check
