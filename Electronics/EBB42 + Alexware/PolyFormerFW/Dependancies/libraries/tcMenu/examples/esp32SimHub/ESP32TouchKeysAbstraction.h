/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef IOABSTRACTION_ESP32TOUCHKEYSABSTRACTION_H
#define IOABSTRACTION_ESP32TOUCHKEYSABSTRACTION_H

#include <IoAbstraction.h>

/**
 * @file ESP32TouchKeysAbstraction.h
 *
 * Contains an ESP32 specific touch key implementation
 */

#ifndef DEFAULT_TOUCHKEY_FILTER_FREQ
#define DEFAULT_TOUCHKEY_FILTER_FREQ 10
#endif

/**
 * An IoAbstraction tht can work with the touch key sensors on ESP32 boards using the underlying functions.
 */
class ESP32TouchKeysAbstraction : public BasicIoAbstraction {
private:
    uint16_t pinThreshold;
    uint16_t enabledPinMask;
    bool allOk;
    bool interruptCodeNeeded;
    bool startedUp;
public:
    ESP32TouchKeysAbstraction(int defThreshold, touch_high_volt_t highVoltage = TOUCH_HVOLT_2V7, touch_low_volt_t lowVoltage = TOUCH_LVOLT_0V5,
                              touch_volt_atten_t attenuation = TOUCH_HVOLT_ATTEN_1V);
    ~ESP32TouchKeysAbstraction() override = default;

    void ensureInterruptRegistered();

    /**
     * You can call this method to check if any errors occurred during initialisation
     * @return true if correctly created, otherwise false.
     */
    bool isCorrectlyCreated() const { return allOk; }

    /**
     * Sets the direction, but direction is always input for this abstraction. Must still be called to initialise the
     * touch interface for that pin.
     * @param pin the touch pin number 0..9
     * @param mode always input, ignored
     */
    void pinDirection(pinid_t pin, uint8_t mode) override;

    uint8_t readValue(pinid_t pin) override;
    void attachInterrupt(pinid_t pin, RawIntHandler interruptHandler, uint8_t mode) override;
    bool runLoop() override;

    //
    // Unimplemented functions
    //
    void writePort(pinid_t pin, uint8_t portVal) override { }
    void writeValue(pinid_t pin, uint8_t value) override { }
    uint8_t readPort(pinid_t pin) override { return 0; }
};

#endif //IOABSTRACTION_ESP32TOUCHKEYSABSTRACTION_H
