/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef IOABSTRACTION_EEPROMABSTRACTIONWIRE_H_
#define IOABSTRACTION_EEPROMABSTRACTIONWIRE_H_

/**
 * @file EepromAbstractionWire.h
 * 
 * Contains the i2c variants of the EepromAbstraction
 */

#include "PlatformDeterminationWire.h"
#include "EepromAbstraction.h"
#include <TaskManager.h>

/**
 * Defines all the variants of the chip that we can pass to the I2cAt24Eeprom constructor. From this we can determine
 * both the size and page size of a given EEPROM.
 */
enum At24EepromType {
    PAGESIZE_AT24C01,
    PAGESIZE_AT24C02,
    PAGESIZE_AT24C04,
    PAGESIZE_AT24C08,
    PAGESIZE_AT24C16,
    PAGESIZE_AT24C32,
    PAGESIZE_AT24C64,
    PAGESIZE_AT24C128,
    PAGESIZE_AT24C256,
    PAGESIZE_AT24C512
};

/**
 * Given an eeprom type enum value this returns the actual size of the rom in bytes
 * @param size the eeprom type
 * @return the size of the rom
 */
size_t at24ActualSizeFromRomSize(At24EepromType size);

/**
 * Given the eeprom type enum value this returns the page size to use for the device.
 * @param size the eeprom type
 * @return the page size
 */
uint8_t at24PageFromRomSize(At24EepromType size);

/**
 * An implementation of eeprom that works with the very well known At24CXXX chips over i2c. Before
 * using this class you must first initialise the Wire library by calling Wire.begin(); If you
 * do not do this, your code may hang. Further, avoid any call to read or write until at least
 * the setup() function is called. This is a limitation of the way the Wire library gets
 * constructed.
 *
 * It is your responsibility to call Wire.begin because you don't want more than one class
 * reinitialising the Wire library.
 *
 * Thanks to https://github.com/cyberp/AT24Cx for some of the ideas I've used in this library,
 * although this is implemented differently.
 */

#ifdef __AVR__
#define WIRE_BUFFER_SIZE 16
#else
#define WIRE_BUFFER_SIZE 32
#endif


class I2cAt24Eeprom : public EepromAbstraction {
	WireType wireImpl;
	uint8_t  eepromAddr;
	bool     errorOccurred;
	uint8_t  pageSize;
    size_t   eepromSize;
public:
	/**
	 * Create an I2C EEPROM object giving it's address and the page size of the device.
	 * Page sizes are defined in this header file.
	 */
    I2cAt24Eeprom(uint8_t address, At24EepromType ty, WireType wireImpl = defaultWireTypePtr);
	~I2cAt24Eeprom() override = default;

	/** 
	 * This indicates if an I2C error has ocrrued at any point since the last call to error.
	 * Side effect: Every call clears it's state.
	 */
	bool hasErrorOccurred() override;

	uint8_t read8(EepromPosition position) override;
	void write8(EepromPosition position, uint8_t val) override;

	uint16_t read16(EepromPosition position) override;
	void write16(EepromPosition position, uint16_t val) override;

	uint32_t read32(EepromPosition position) override;
	void write32(EepromPosition position, uint32_t val) override;

	void readIntoMemArray(uint8_t* memDest, EepromPosition romSrc, uint8_t len) override;
	void writeArrayToRom(EepromPosition romDest, const uint8_t* memSrc, uint8_t len) override;
private:
	uint8_t findMaximumInPage(uint16_t romDest, uint8_t len) const;
	void writeByte(EepromPosition position, uint8_t val);
	uint8_t readByte(EepromPosition position);
    void writeAddressWire(uint16_t memAddr, const uint8_t* data = nullptr, int len = 0);
};

#endif /* IOABSTRACTION_EEPROMABSTRACTIONWIRE_H_ */
