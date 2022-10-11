/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "../PlatformDetermination.h"
#include "../AnalogDeviceAbstraction.h"

#if !defined(IOA_MBEDANALOGDEVICE_H) && defined(IOA_USE_MBED)
#define IOA_MBEDANALOGDEVICE_H

class AnalogPinReference {
private:
    pinid_t pin;
    AnalogDirection direction;
    union AnalogPinReferences {
        AnalogIn* input;
#ifdef DEVICE_ANALOGOUT
        AnalogOut* out;
#endif
        PwmOut* pwm;
    } analogRef;
public:
    AnalogPinReference() {
        analogRef.input = NULL;
        pin = 0;
        direction = DIR_IN;
    }

    AnalogPinReference(const AnalogPinReference& other) {
        this->pin = other.pin;
        this->direction = other.direction;
        this->analogRef.input = other.analogRef.input;
    }

    AnalogPinReference& operator=(const AnalogPinReference& other) {
        if(this == &other) return *this;
        this->pin = other.pin;
        this->direction = other.direction;
        this->analogRef.input = other.analogRef.input;
        return *this;
    }

    AnalogPinReference(pinid_t pin, AnalogDirection direction) {
        this->pin = pin;
        this->direction = direction;
        switch(direction) {
            case DIR_IN:
                analogRef.input = new AnalogIn((PinName)pin);
                break;
#ifdef DEVICE_ANALOGOUT
            case DIR_OUT:
                analogRef.out = new AnalogOut((PinName)pin);
                break;
#endif
            default:
                analogRef.pwm = new PwmOut((PinName)pin);
                break;
        }
    }

    AnalogPinReferences getReferences() { return analogRef; }
    AnalogDirection getDirection() { return direction; }
    pinid_t getKey() const { return pin; }
};


/**
 * Represents the mbed analog capabilities as an Analog device abstraction, allows
 * for the same code to be used between Mbed and Arduino. Note that presently the
 * value for integers is represented between 0..65535 although this may not be
 * what your hardware supports. It's better to always use the float functions when
 * you can.
 *
 * Get an instance by calling internalAnalogIO() rather than creating one.
 */
class MBedAnalogDevice : public AnalogDevice {
private:
    BtreeList<pinid_t, AnalogPinReference> devices;
public:
    static MBedAnalogDevice* theInstance;

    int getMaximumRange(AnalogDirection direction, pinid_t pin) override { return 0xffff; }

    int getBitDepth(AnalogDirection direction, pinid_t pin) override { return 16; }

    void initPin(pinid_t pin, AnalogDirection direction) override;

    unsigned int getCurrentValue(pinid_t pin) override;

    float getCurrentFloat(pinid_t pin) override;

    void setCurrentValue(pinid_t pin, unsigned int newValue) override;

    void setCurrentFloat(pinid_t pin, float newValue) override;

    AnalogPinReference* getAnalogGPIO(pinid_t pin) { return devices.getByKey(pin); }
};

#endif //IOA_MBEDANALOGDEVICE_H and using mbed
