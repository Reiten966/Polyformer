/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _DF_ROBOT_INPUT_ABSTRACTION_H
#define _DF_ROBOT_INPUT_ABSTRACTION_H

#include "BasicIoAbstraction.h"
#include "AnalogDeviceAbstraction.h"

/**
 * @file DfRobotInputAbstraction.h
 * 
 * This file contains an implementation of BasicIoAbstraction that works with the DfRobot shield
 * in order to convert the analog keys into regular digital IO. This allows DfRobot to be used
 * with switches for button management.
 * 
 * See: https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)
 */

#define DF_KEY_RIGHT 0
#define DF_KEY_LEFT 1
#define DF_KEY_UP 2
#define DF_KEY_DOWN 3
#define DF_KEY_SELECT 4

struct DfRobotAnalogRanges {
    float right;
    float up;
    float down;
    float left;
    float select;
};

#define ALLOWABLE_RANGE 0.01F
#ifdef IOA_USE_MBED
#define pgmAsFloat(x) ((float)(*x))
#else
#define pgmAsFloat(x) ((float)pgm_read_float_near(x))
#endif
/**
 * DfRobotInputAbstraction provides the means to use many buttons connected to a single
 * Analog input. It is mainly designed to work with the df robot shield.
 * It simulates a digital port of 8 bits where the following mappings are made
 * 
 * * pin0 = right (DF_KEY_RIGHT)
 * * pin1 = up (DF_KEY_UP)
 * * pin2 = down (DF_KEY_DOWN)
 * * pin3 = left (DF_KEY_LEFT)
 * * pin4 = select (DF_KEY_SELECT)
 */
class DfRobotInputAbstraction : public BasicIoAbstraction {
private:
    pinid_t analogPin;
    uint8_t readCache;
    float lastReading;
    const DfRobotAnalogRanges* analogRanges;
    AnalogDevice* device;

public:
    DfRobotInputAbstraction(const DfRobotAnalogRanges* ranges, pinid_t pin, AnalogDevice* device) {
        analogRanges = ranges;
        analogPin = pin;
        this->device = device;

        device->initPin(analogPin, DIR_IN);
        lastReading = device->getCurrentValue(analogPin);
        readCache = mapAnalogToPin(lastReading);
    }

    uint8_t readValue(pinid_t pin) override {
    	return bitRead(readCache, pin);
    }

    uint8_t readPort(pinid_t port) override {
        return readCache;
    }

	bool runLoop() override { 
        auto newReading = device->getCurrentFloat(analogPin);
        if(abs(newReading - lastReading) > ALLOWABLE_RANGE) {
            readCache = mapAnalogToPin(newReading);
        }
        lastReading = newReading;
        return true;
    }

    uint8_t mapAnalogToPin(float reading) {
        uint8_t ret = 0xff;
        if(reading < pgmAsFloat(&analogRanges->right)) ret =  DF_KEY_RIGHT;
        else if(reading < pgmAsFloat(&analogRanges->up)) ret =  DF_KEY_UP;
        else if(reading < pgmAsFloat(&analogRanges->down)) ret = DF_KEY_DOWN;
        else if(reading < pgmAsFloat(&analogRanges->left)) ret = DF_KEY_LEFT;
        else if(reading < pgmAsFloat(&analogRanges->select)) ret = DF_KEY_SELECT;

        if(ret == 0xff) 
            return 0;
        else
            return 1 << ret;
    }

    // we ignore all non-input methods, as this is input only

    void pinDirection(pinid_t pin, uint8_t mode) override {
        /** ignored as only input is supported */
    }

   	void writeValue(pinid_t pin, uint8_t value) override {
        /** ignored as only input is supported */        
    }

	void writePort(pinid_t pin, uint8_t portVal) override {
        /** ignored as only input is supported */
    }
};

#ifndef IOA_USE_MBED

/**
 * Defines the analog ranges to pass to the DfRobotInputAbstraction - default 
 */
const PROGMEM DfRobotAnalogRanges dfRobotAvrRanges { 0.0488F, 0.2441F, 0.4394F, 0.6347F, 0.8300F};

/**
 * Defines the analog ranges to pass to the DfRobotInputAbstraction - for V1.0 of the board 
 */
const PROGMEM DfRobotAnalogRanges dfRobotV1AvrRanges { 0.0488F, 0.1904F, 0.3710F, 0.5419F, 0.7714F};


inline IoAbstractionRef inputFromDfRobotShield(uint8_t pin = A0, AnalogDevice* device = nullptr) {
    device = internalAnalogIo();
    return new DfRobotInputAbstraction(&dfRobotAvrRanges, pin, device);
}

inline IoAbstractionRef inputFromDfRobotShieldV1(uint8_t pin = A0, AnalogDevice* device = nullptr) {
    device = internalAnalogIo();
    return new DfRobotInputAbstraction(&dfRobotV1AvrRanges, pin, device);
}

#endif

#endif
