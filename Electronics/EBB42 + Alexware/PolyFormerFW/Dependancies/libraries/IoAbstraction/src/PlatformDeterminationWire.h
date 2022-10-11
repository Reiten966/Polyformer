/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef IOA_PLATFORMDETERMINATIONWIRE_H
#define IOA_PLATFORMDETERMINATIONWIRE_H

#include "PlatformDetermination.h"
#include <SimpleSpinLock.h>

//
// Here we work out what wire looks like on this board! Becoming non trivial these days!
//
#ifdef IOA_USE_MBED
#include <i2c_api.h>
typedef I2C* WireType;
void ioaWireBegin(I2C* pI2cToUse);
# define IOA_USE_MBED_WIRE
#elif defined(IOA_USE_AVR_TWI_DIRECT) && defined(__AVR__) && defined(IOA_DEVELOPMENT_EXPERIMENTAL)
class AvrTwiManager;
typedef AvrTwiManager* WireType;
extern WireType AvrTwi;
void ioaWireBegin();
#else
# define IOA_USE_ARDUINO_WIRE
#include <Wire.h>
typedef TwoWire* WireType;
void ioaWireBegin();
#endif // IOA_USE_MBED

extern WireType defaultWireTypePtr;

/**
 * Reads size bytes from the buffer and returns the number of bytes read if successful. If it was not possible to
 * read IOA_SIZE_ERR will be returned to indicate the failure.
 * @param buffer the buffer to read into
 * @param len amount to read
 * @return true if successful, otherwise false.
 */
bool ioaWireRead(WireType wire, int address, uint8_t* buffer, size_t len);

/**
 * Writes the number of bytes specified from the buffer to the I2C bus. This number must not exceed the maximum buffer
 * size on Arduino boards. If the device is busy this implementation can be configured to wait up to retries allowed
 * loops with a short yield. You can optionally send a stop event.
 * @param pI2c the wire implementation
 * @param address the address to write to
 * @param buffer the buffer of data to be written
 * @param len the length of the buffer
 * @param retriesAllowed the number of retries before failing
 * @param sendStop if the stop event should be sent during transaction end.
 * @return true if successful, otherwise false.
 */
bool ioaWireWriteWithRetry(WireType pI2c, int address, const uint8_t* buffer, size_t len, int retriesAllowed = 0, bool sendStop = true);

/**
 * Sets the frequency of the selected I2C bus.
 * @param pI2c the I2C that the frequency is to be adjusted
 * @param frequency the new frequency
 */
void ioaWireSetSpeed(WireType pI2c, long frequency);

/**
 * Check if there is a device on the bus at the given address, and that it is ready for use.
 * @param address the address to check
 * @return true if raising a start on that device address returned successfully. It will be followed by a stop.
 */
bool ioaWireReady(WireType wire, int address);

/**
 * The lock that must be used before accessing wire. All I2C calls use this lock, but you can wrap it around multiple
 * calls if they should happen together.
 */
extern SimpleSpinLock i2cLock;

#endif //IOA_PLATFORMDETERMINATIONWIRE_H
