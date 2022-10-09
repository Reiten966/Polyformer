/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "../PlatformDetermination.h"

#if !defined(ARDUINO_ANALOG_DEVICE_H) && defined(IOA_USE_ARDUINO) && !defined(ESP32)
#define ARDUINO_ANALOG_DEVICE_H

#include "../AnalogDeviceAbstraction.h"

/**
 * Creates an analog device that uses the core Arduino analog capabilities of ADC for reading
 * values and PWM or the inbuilt DAC if available for writing values. Generally speaking one
 * starts by initialising a pin as either input or output, then calling the read and write
 * methods to work with the device.
 *
 * If performance is not critical, use the floating point versions of the read and write methods
 * as these abstract away the absolute ranges, to 0 being GND and 1 being maximum voltage.
 *
 * Get an instance by calling internalAnalogIO() rather than creating one
 */
class ArduinoAnalogDevice : public AnalogDevice {
private:
    uint8_t readBitResolution;
    uint8_t writeBitResolution;
    uint16_t readResolution;
    uint16_t writeResolution;
public:
    static ArduinoAnalogDevice* theInstance;

    /**
	 * Initialise the Arduino analog device with a given read and write bit resolution, on AVR and
	 * ESP8266 input is set to 10 bits (1024) and output to 8 bits (255). On SAMD and some other board
     * types you can configure either 8, 10 or 12 bit. On SAMD by default 12 bit (4096) input 10 bit output.
	 */
    explicit ArduinoAnalogDevice(uint8_t readBitResolution = IOA_ANALOGIN_RES, uint8_t writeBitResolution = IOA_ANALOGOUT_RES);

    int getMaximumRange(AnalogDirection dir, pinid_t /*pin*/) override {
        return (dir == DIR_IN) ?  readResolution : writeResolution;
    }

    int getBitDepth(AnalogDirection direction, pinid_t /*pin*/) override {
        return (direction == DIR_IN) ? readBitResolution : writeBitResolution;
    }

    float getCurrentFloat(pinid_t pin) override;

    void setCurrentFloat(pinid_t pin, float value) override;

    void initPin(pinid_t pin, AnalogDirection direction) override;

    unsigned int getCurrentValue(pinid_t pin) override { return analogRead(pin); }

    void setCurrentValue(pinid_t pin, unsigned int newVal) override { analogWrite(pin, newVal); }
};

#endif
