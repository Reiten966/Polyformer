/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _ANALOG_DEVICE_ABSTRACTION_H_
#define _ANALOG_DEVICE_ABSTRACTION_H_

#include "PlatformDetermination.h"
#include <BasicIoAbstraction.h>

/**
 * @file AnalogDeviceAbstraction.h
 * Contains a series of helper classes for dealing with analog devices, these are compatible across a wide range of
 * devices.
 */

/** 
 * an enumeration that describes direction, eg input or output for the ADC/POT/DAC. For some
 * devices only one mode will make sense.
 */
enum AnalogDirection { DIR_IN, DIR_OUT, DIR_PWM };

/**
 * Describes an analog device that has commands to both read values from and write values to
 * a device. Not all devices will support both input and output. When such a case occurs the
 * getMaximumRange would return -1 for that direction. This abstraction can support ADC, PWM
 * DAC and Potentiometer devices. On every device we support, calling internalAnalogIO gives
 * the instance for the internal analog pins.
 */
class AnalogDevice {
public:
	/**
	 * @param dir the direction required
	 * @param pin the pin for which the range is desired
	 * @return the maximum range of the analog input or output on this device for the given pin 
	 */
	virtual int getMaximumRange(AnalogDirection direction, pinid_t pin)=0;

    /**
     * @param pin the pin for which the bit depth is required.
     * @param direction the direction in which the depth is queried (DIR_IN, DIR_OUT)
     * @return the number of bits
     */
    virtual int getBitDepth(AnalogDirection direction, pinid_t pin) = 0;

	/**
	 * initialises a pin as either an input or output of analog signals. No validation to check if
	 * that pin can support input or output is performed.
	 *
	 * @param pin the pin to initialise
	 * @param direction the direction required
	 */
	virtual void initPin(pinid_t pin, AnalogDirection direction) = 0;

	/**
	 * Returns the current value on the ADC for the given pin
	 * @param pin the pin to read from
	 * @return the current value on that pin
	 */
	virtual unsigned int getCurrentValue(pinid_t pin)=0;

	/*
	 * Returns the current value on the ADC as a float between
	 * 0 and 1.
	 */
	virtual float getCurrentFloat(pinid_t pin) = 0;

	/**
	 * Sets the current value on an output capable device to a new value
	 * @param pin the pin to read from
	 * @param newValue the value to be set
	 */
	virtual void setCurrentValue(pinid_t pin, unsigned int newValue)=0;

	/**
	 * sets the current value based on a float from 0 to 1, where 0 is
	 * minimum and 1 is maximum.
	 * @param pin the pin for which to set
	 * @param newValue the new value which should be between 0 and 1.0
	 */
    virtual void setCurrentFloat(pinid_t pin, float newValue)=0;

};

#if defined(IOA_USE_MBED)
#include "mbed/MbedAnalogDevice.h"
#elif defined(ESP32)
# include "esp32/ESP32AnalogDevice.h"
#elif defined(IOA_USE_ARDUINO)
#include "arduino/ArduinoAnalogDevice.h"
#endif

/**
 * Create an instance of the analog IO abstraction for the current hardware and cache it
 * so further calls return the same one, use this instead of creating one.
 * @return the analog device as a pointer.
 */
AnalogDevice* internalAnalogIo();


#endif //_ANALOG_DEVICE_ABSTRACTION_H_
