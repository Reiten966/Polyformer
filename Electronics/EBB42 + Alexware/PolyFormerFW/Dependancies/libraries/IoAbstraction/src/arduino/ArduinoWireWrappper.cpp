/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <TaskManagerIO.h>
#include <IoLogging.h>
#include "PlatformDeterminationWire.h"

SimpleSpinLock i2cLock;

#if defined(IOA_USE_ARDUINO_WIRE)

#include <Wire.h>


WireType defaultWireTypePtr = &Wire;

void ioaWireBegin() {
    defaultWireTypePtr->begin();
}

void ioaWireSetSpeed(WireType wireType, long frequency) {
    wireType->setClock(frequency);
}

bool ioaWireRead(WireType pI2c, int addr, uint8_t* buffer, size_t len) {
    if(pI2c->requestFrom(addr, len)) {
        uint8_t idx = 0;
        while(pI2c->available() && idx < len) {
            buffer[idx] = pI2c->read();
            idx++;
        }
        return idx == len;
    }
    return false;
}

bool ioaWireWriteWithRetry(WireType pI2c, int address, const uint8_t* buffer, size_t len, int retriesAllowed, bool sendStop) {
    bool firstTime = true;
    bool i2cReady = retriesAllowed == 0;
    while(retriesAllowed && !i2cReady) {
        if(!firstTime) {
            taskManager.yieldForMicros(50);
        }
        firstTime = false;
        pI2c->beginTransmission(address);
        i2cReady = pI2c->endTransmission() == 0;
        retriesAllowed--;
    }

    if(!i2cReady) {
        serlogF(SER_ERROR, "I2C was not ready after retries, failing");
        return false;
    }

    pI2c->beginTransmission(address);
    pI2c->write(buffer, len);
    auto writeOk = pI2c->endTransmission(sendStop) == 0;

    return writeOk;
}

#endif
