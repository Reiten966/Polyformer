/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include <AnalogDeviceAbstraction.h>

#if defined(IOA_USE_MBED)

MBedAnalogDevice* MBedAnalogDevice::theInstance = nullptr;
AnalogDevice* internalAnalogIo() {
    if(MBedAnalogDevice::theInstance == nullptr) MBedAnalogDevice::theInstance = new MBedAnalogDevice();
    return MBedAnalogDevice::theInstance;
}

void MBedAnalogDevice::initPin(pinid_t pin, AnalogDirection direction) {
    if(devices.getByKey(pin) == nullptr) {
        devices.add(AnalogPinReference(pin, direction));
    }
}

unsigned int MBedAnalogDevice::getCurrentValue(pinid_t pin) {
    auto dev = devices.getByKey(pin);
    if(dev == nullptr || dev->getDirection() != DIR_IN) return 0;
    return dev->getReferences().input->read_u16();
}

float MBedAnalogDevice::getCurrentFloat(pinid_t pin) {
    auto dev = devices.getByKey(pin);
    if(dev == nullptr || dev->getDirection() != DIR_IN) return 0;
    return dev->getReferences().input->read();
}

void MBedAnalogDevice::setCurrentValue(pinid_t pin, unsigned int newValue) {
    auto dev = devices.getByKey(pin);
    if(dev == nullptr || dev->getDirection() == DIR_IN) return;
#ifdef DEVICE_ANALOGOUT
    if(dev->getDirection() == DIR_OUT) {
        return dev->getReferences().out->write_u16(newValue);
    }
#endif
    return dev->getReferences().pwm->write(float(newValue) / 65535.0F);
}

void MBedAnalogDevice::setCurrentFloat(pinid_t pin, float newValue) {
    auto dev = devices.getByKey(pin);
    if(dev == nullptr || dev->getDirection() == DIR_IN) return;
#ifdef DEVICE_ANALOGOUT
    if(dev->getDirection() == DIR_OUT) {
        return dev->getReferences().out->write(newValue);
    }
#endif
    return dev->getReferences().pwm->write(newValue);
}

#endif