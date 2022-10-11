/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <TaskManagerIO.h>
#include "PlatformDeterminationWire.h"

#ifdef IOA_USE_MBED_WIRE

// on mbed this must be set by the user.
WireType defaultWireTypePtr;

void ioaWireBegin(I2C* pI2cToUse) {
    defaultWireTypePtr = pI2cToUse;
}

void ioaWireSetSpeed(WireType i2c, long frequency) {
    i2c->frequency(frequency);
}

bool ioaWireRead(WireType pI2c, int address, uint8_t* buffer, size_t len) {
    return pI2c->read(address, (char*)buffer, len, false) == 0;
}

bool ioaWireWriteWithRetry(WireType pI2c, int address, const uint8_t* buffer, size_t len, int retriesAllowed, bool sendStop) {
    int tries = 0;
    while(pI2c->write(address, (const char*)buffer, len, !sendStop) !=0) {
        if(tries > retriesAllowed) return false;
        taskManager.yieldForMicros(50);
        tries++;
    }
    return true;
}

#endif
