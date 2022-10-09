/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file EepromAbstraction.h
 * 
 * Wraps up EEPROM support in a way that is compatible between implementations. For example presently there
 * are AVR, EEPROM, and i2c implementations that work interchangeably. Adding another variant is quite trivial.
 */

#ifndef _IOABSTRACTION_EEPROMABSTRACTION_H_
#define _IOABSTRACTION_EEPROMABSTRACTION_H_

#include "PlatformDetermination.h"

#ifdef IOA_USE_MBED
#include <mbed.h>
#else
#include <Arduino.h>
#endif

/**
 * Defines an address or position within the eeprom storage 
 */
typedef uint16_t EepromPosition;

/**
 * Provides an abstraction on eeprom storage, to allow either on chip or external I2c based eeprom storage, or even
 * No storage whatsoever. This helps no end with 32 bit boards that don't have eeprom!
 * Most EEPROM implementions here only write if there are changes.
 */
class EepromAbstraction {
public:
	virtual ~EepromAbstraction() {}

	/**
	 * Best efforts error flag that will clear once read.
	 */
	virtual bool hasErrorOccurred() { return false;}

	/** 
	 * Read an 8 bit (byte) value at a specified position 
	 * @param position address at which to read
	 */
	virtual uint8_t read8(EepromPosition position) = 0;

	/** 
	 * write an 8 bit (byte) value to the specified position 
	 * @param position the position at which to write
	 * @param val the new value
	 */
	virtual void write8(EepromPosition position, uint8_t val) = 0;

	/** 
	 * read a 16 bit value at position at a specified position 
	 * @param position the position at which to read 
	 */
	virtual uint16_t read16(EepromPosition position) = 0;

	/** 
	 * write a 16 bit value to the specified position 
	 * @param position the position at which to write
	 * @param val the value to read
	 */
	virtual void write16(EepromPosition position, uint16_t val) = 0;

	/** 
	 * read a 32 bit value at a specified position 
	 * @param position the position at which to read
	 */
	virtual uint32_t read32(EepromPosition position) = 0;

	/** 
	 * write a 32 bit value to position 
	 * @param position the position at which to write
	 * @param val the value to write out.
	 */
	virtual void write32(EepromPosition position, uint32_t val) = 0;

	/**
	 * Read an array of bytes from EEPROM into memory
	 * @param memDest the memory where the EEPROM data should be copied to
	 * @param romSrc the source position in EEPROM storage
	 * @param len the length of the array
	 */
	virtual void readIntoMemArray(uint8_t* memDest, EepromPosition romSrc, uint8_t len) = 0;

	/**
	 * Writes an array of bytes from memory to EEPROM storage
	 * @param romDest the start position in eeprom storage that the array should be copied to
	 * @param memSrc the memory where the rom should be copied from
	 * @param len the length of the array
	 */
	virtual void writeArrayToRom(EepromPosition romDest, const uint8_t* memSrc, uint8_t len) = 0;
};

// only include the atmel AVR support if it's available on this platform.
#ifdef __AVR__

/**
 * An implementation of eeprom that uses the standard AVR EEPROM built into most 8 bit chips.
 * This will only write values to eeprom if they have actually changed. Preserving write capacity.
 */
class AvrEeprom : public EepromAbstraction {
public:
	virtual ~AvrEeprom() {}

	virtual uint8_t read8(EepromPosition position);
	virtual void write8(EepromPosition position, uint8_t val);

	virtual uint16_t read16(EepromPosition position);
	virtual void write16(EepromPosition position, uint16_t val);

	virtual uint32_t read32(EepromPosition position);
	virtual void write32(EepromPosition position, uint32_t val);

	virtual void readIntoMemArray(uint8_t* memDest, EepromPosition romSrc, uint8_t len);
	virtual void writeArrayToRom(EepromPosition romDest, const uint8_t* memSrc, uint8_t len);
};

#endif

/**
 * An implementation of eeprom that does nothing, for situations where no such storage is needed
 */
class NoEeprom : public EepromAbstraction {
public:
	virtual ~NoEeprom() {}

	virtual uint8_t read8(__attribute__((unused)) EepromPosition position) {return 0;}
	virtual void write8(__attribute__((unused)) EepromPosition position, __attribute__((unused)) uint8_t val) {}

	virtual uint16_t read16(__attribute__((unused)) EepromPosition position) {return 0;}
	virtual void write16(__attribute__((unused)) EepromPosition position, __attribute__((unused)) uint16_t val) {}

	virtual uint32_t read32(__attribute__((unused)) EepromPosition position) {return 0;}
	virtual void write32(__attribute__((unused)) EepromPosition position, __attribute__((unused)) uint32_t val) {}

	virtual void readIntoMemArray(__attribute__((unused)) uint8_t* memDest, __attribute__((unused)) EepromPosition romSrc, __attribute__((unused)) uint8_t len) {memDest[0]=0;}
	virtual void writeArrayToRom(__attribute__((unused)) EepromPosition romDest, __attribute__((unused)) const uint8_t* memSrc, __attribute__((unused)) uint8_t len) {}
};

#endif /* _IOABSTRACTION_EEPROMABSTRACTION_H_ */
