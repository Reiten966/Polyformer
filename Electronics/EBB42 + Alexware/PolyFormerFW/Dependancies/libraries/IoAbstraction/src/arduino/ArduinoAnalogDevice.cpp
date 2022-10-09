/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <IoLogging.h>
#include "../PlatformDetermination.h"

#if !defined(ESP32) && defined(IOA_USE_ARDUINO)

#include "ArduinoAnalogDevice.h"

ArduinoAnalogDevice* ArduinoAnalogDevice::theInstance = nullptr;
AnalogDevice* internalAnalogIo() {
    if(ArduinoAnalogDevice::theInstance == nullptr) ArduinoAnalogDevice::theInstance = new ArduinoAnalogDevice();
    return ArduinoAnalogDevice::theInstance;
}

ArduinoAnalogDevice::ArduinoAnalogDevice(uint8_t readBitResolution, uint8_t writeBitResolution) {
#if IOA_ANALOGIN_RES > 10
    // some boards have the option for greater analog resolution on input. It needs to be configured
    analogReadResolution(readBitResolution);
#endif
#if (IOA_ANALOGOUT_RES > 8) && !defined(ESP8266)
    // some boards have the option for greater analog output resolution, but then it needs configuring.
    // except on esp8266 where 1024 pwm resolution is standard
    analogWriteResolution(writeBitResolution);
#endif
    this->readBitResolution = readBitResolution;
    this->writeBitResolution = writeBitResolution;
    this->readResolution = (1 << readBitResolution) - 1;
    this->writeResolution = (1 << writeBitResolution) - 1;
    theInstance = this;
}

void ArduinoAnalogDevice::setCurrentFloat(pinid_t pin, float value) {
    if(value < 0.0F) value = 0.0F;
    auto maxValue = getMaximumRange(DIR_OUT, pin);
    auto compVal = (int)(value * float(maxValue));
    if(compVal  > maxValue) compVal = maxValue;
    setCurrentValue(pin, compVal);
    serlogF4(SER_IOA_DEBUG, "Flt set ", value, maxValue, compVal);
}

float ArduinoAnalogDevice::getCurrentFloat(pinid_t pin) {
    auto maxValue = (float)getMaximumRange(DIR_IN, pin);
    return float(analogRead(pin)) / maxValue;
}

void ArduinoAnalogDevice::initPin(pinid_t pin, AnalogDirection direction) {
    pinMode(pin, (direction == DIR_IN) ? INPUT : OUTPUT);
}

#endif
