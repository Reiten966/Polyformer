/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef IOA_MBED_DIGITAL_IO_H
#define IOA_MBED_DIGITAL_IO_H

#include <mbed.h>
#include <stdint.h>
#include <SimpleCollections.h>

// the following defines allow you to pass regular arduino pin modes in mbed

#define INPUT PullNone
#define INPUT_PULLUP PullUp
#define OUTPUT 0xff
#define RISING 0x01
#define FALLING 0x02
#define CHANGE 0x03
#define PROGMEM
#define HIGH 1
#define LOW 0

#define bitRead(value, bit) (((value) & (1 << (bit))) != 0)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

class GpioWrapper {
private:
    uint32_t pin;
    gpio_t gpio;
    InterruptIn* interruptHandler;
    uint8_t pinMode;
public:
    GpioWrapper() {
        pin = (uint32_t)-1;
        interruptHandler = NULL;
    }
    GpioWrapper(uint32_t pin) {
        this->pin = pin;
        interruptHandler = NULL;
    }
    GpioWrapper(const GpioWrapper& other) {
        this->pin = other.pin;
        this->interruptHandler = other.interruptHandler;
        this->pinMode = other.pinMode;
        memcpy(&this->gpio, &other.gpio, sizeof(gpio_t));
    }
    GpioWrapper& operator=(const GpioWrapper& other) {
        if(this == &other) return *this;
        this->pin = other.pin;
        this->pinMode = other.pinMode;
        this->interruptHandler = other.interruptHandler;
        memcpy(&this->gpio, &other.gpio, sizeof(gpio_t));
        return *this;
    }
    uint32_t getPin() const { return pin; }
    uint32_t getKey() const { return pin; }
    gpio_t* getGpio() { return &gpio; }
    void setPinMode(uint8_t mode) { pinMode = mode; }
    uint8_t getPinMode() { return pinMode; }
    InterruptIn* getInterruptIn() { return interruptHandler; }
    void setInterruptIn(InterruptIn* in) { interruptHandler = in; }
};

#endif
