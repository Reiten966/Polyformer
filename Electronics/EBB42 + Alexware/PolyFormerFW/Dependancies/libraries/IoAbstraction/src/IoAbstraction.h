
/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _BASIC_IO_ABSTRACTION_H_
#define _BASIC_IO_ABSTRACTION_H_

/**
 * @file IoAbstraction.h
 * 
 * Using basic IoFacilities allows one to abstract away the use of IoExpanders, such 
 * that the switching from BasicIoFacilities to IoExpanderFacilities allows the same
 * code to use an IoExpander instead of direct pins
 */
#include "PlatformDetermination.h"
#include "BasicIoAbstraction.h"

#define SHIFT_REGISTER_OUTPUT_CUTOVER 32

#ifndef IOA_USE_MBED
#include <Arduino.h>

/**
 * Notice that the output range has been moved from 24 to 32 onwards , this is to allow support for
 * up to 4 devices chained together, this is a breaking change from the 1.0.x versions.
 * 
 * An implementation of BasicIoFacilities that supports the ubiquitous shift
 * register, using 74HC165 for input (pins 0 to 23) and a 74HC595 for output (32 onwards).
 * It supports up to three registers in each direction at the moment.
 */
class ShiftRegisterIoAbstraction : public BasicIoAbstraction {
private:
	uint32_t toWrite;
	uint32_t lastRead;
	bool needsWrite;

	uint8_t numOfDevicesRead;
	pinid_t readDataPin;
	pinid_t readLatchPin;
	pinid_t readClockPin;

	uint8_t numOfDevicesWrite;
	pinid_t writeDataPin;
	pinid_t writeLatchPin;
	pinid_t writeClockPin;
public:
	/** 
	 * Normally use the shift register helper functions to create an instance.
	 * @see inputOutputFromShiftRegister
	 * @see inputOnlyFromShiftRegister
	 * @see outputOnlyFromShiftRegister
	 */
	ShiftRegisterIoAbstraction(pinid_t readClockPin, pinid_t readDataPin, pinid_t readLatchPin,
	                           pinid_t writeClockPin, pinid_t writeDataPin, pinid_t writeLatchPin, uint8_t numRead, uint8_t numWrite);
	~ShiftRegisterIoAbstraction() override { }
	void pinDirection(pinid_t pin, uint8_t mode) override;
	void writeValue(pinid_t pin, uint8_t value) override;
	uint8_t readValue(pinid_t pin) override;
	/**
	 * Interrupts are not supported on shift registers
	 */
	void attachInterrupt(pinid_t, RawIntHandler, uint8_t) override {;}
	bool runLoop() override;
	
	/**
	 * writes to the output shift register - currently always port 0
	 */
	void writePort(pinid_t port, uint8_t portVal) override;

	/**
	 * reads from the input shift register - currently always port 3
	 */
	uint8_t readPort(pinid_t port) override;
};

class ShiftRegisterIoAbstraction165In : public BasicIoAbstraction {
private:
    uint32_t lastRead;
    uint8_t numOfDevicesRead;
    pinid_t readDataPin;
    pinid_t readLatchPin;
    pinid_t readClockPin;

public:
    /**
     * Normally use the shift register helper functions to create an instance.
     * @see inputOutputFromShiftRegister
     * @see inputOnlyFromShiftRegister
     * @see outputOnlyFromShiftRegister
     */
    ShiftRegisterIoAbstraction165In(pinid_t readClockPin, pinid_t readDataPin, pinid_t readLatchPin, pinid_t numRead);
    ~ShiftRegisterIoAbstraction165In() override = default;

    /** Input only abstraction, does nothing because only input is supported */
    void pinDirection(pinid_t pin, uint8_t mode) override { }

    uint8_t readValue(pinid_t pin) override;
    bool runLoop() override;
    uint8_t readPort(pinid_t port) override;

    //
    // Features not implemented on this abstaction
    //
    void writePort(pinid_t port, uint8_t portVal) override { }
    void writeValue(pinid_t pin, uint8_t value) override { }
    void attachInterrupt(pinid_t, RawIntHandler, uint8_t) override { }

    uint8_t shiftInFor165() const;
};

/**
 * performs both input and output functions using two or more shift registers, for both reading and writing.  As shift registers have a fixed direction
 * input and output are handled by different devices, and therefore fixed at the time of building the circuit. This function supports chaining of
 * up to 4 devices for both directions.
 *
 * This abstraction works as follows:
 *
 * * Input pins of the input shift register start at 0
 * * Output pins of the output shift register start at 32.
 *
 * @param readClockPin the clock pin on the INPUT shift register
 * @param readDataPin the data pin on the INPUT shift register
 * @param readLatchPin the latch pin on the INPUT shift register
 * @param numOfDevicesRead the number of shift registers that have been chained for reading
 * @param writeClockPin the clock pin on the OUTPUT shift register
 * @param writeDataPin the data pin on the OUTPUT shift register
 * @param writeLatchPin the latch pin on the OUTPUT shift register
 * @param numOfDevicesWrite the number of shift registers that have been chained for writing
 */
IoAbstractionRef inputOutputFromShiftRegister(uint8_t readClockPin, uint8_t readDataPin, uint8_t readLatchPin, uint8_t numOfReadDevices,
									          uint8_t writeClockPin, uint8_t writeDataPin, uint8_t writeLatchPin, uint8_t numOfWriteDevices);

/**
 * performs both input and output functions using two shift registers, one for reading and one for writing.  As shift registers have a fixed direction
 * input and output are handled by different devices, and therefore fixed at the time of building the circuit. This function is for when you have a
 * single input and single output device. See the other version of the function if you need to chain devices.
 *
 * This abstraction works as follows:
 *
 * * Input pins of the input shift register start at 0
 * * Output pins of the output shift register start at 32.
 *
 * @param readClockPin the clock pin on the INPUT shift register
 * @param readDataPin the data pin on the INPUT shift register
 * @param readLatchPin the latch pin on the INPUT shift register
 * @param writeClockPin the clock pin on the OUTPUT shift register
 * @param writeDataPin the data pin on the OUTPUT shift register
 * @param writeLatchPin the latch pin on the OUTPUT shift register
 */
IoAbstractionRef inputOutputFromShiftRegister(uint8_t readClockPin, uint8_t readDataPin, uint8_t readLatchPin,
									          uint8_t writeClockPin, uint8_t writeDataPin, uint8_t writeLatchPin);

/**
 * performs input only functions using a shift register, the input pins of the shift register start at pin 0.
 * @param readClockPin the clock pin on the INPUT shift register
 * @param dataPin the data pin on the INPUT shift register
 * @param latchPin the latch pin on the INPUT shift register
 */
IoAbstractionRef inputOnlyFromShiftRegister(uint8_t readClockPin, uint8_t dataPin, uint8_t latchPin, uint8_t numOfDevicesRead = 1);

/**
 * performs output only functions using a shift register, the output pins of the shift register start at 32.
 * @param writeClockPin the clock pin on the OUTPUT shift register
 * @param writeDataPin the data pin on the OUTPUT shift register
 * @param writeLatchPin the latch pin on the OUTPUT shift register
 */
IoAbstractionRef outputOnlyFromShiftRegister(uint8_t writeClockPin, uint8_t writeDataPin, uint8_t writeLatchPin, uint8_t numOfDevicesWrite = 1);

/**
 * Performs input only functions using a 74x165 plugin, the input pins start at 0 up to a maximum of 31, each device
 * adds another 8 pins.
 * @param readClkPin the clock pin of the shift register, used as OUTPUT
 * @param dataPin the data pin of the shift register, used as INPUT
 * @param latchPin the latch pin of the shift register, used as OUTPUT
 * @param numOfDevices the number of devices that are chained together in the usual fashion, range is 1..4
 * @return a shift register abstraction as an IoAbstraction ref.
 */
IoAbstractionRef inputFrom74HC165ShiftRegister(pinid_t readClkPin, pinid_t dataPin, pinid_t latchPin, pinid_t numOfDevices = 1);

#else
#include <mbed.h>

#endif // not IOA_USE_MBED

// this defines the number of IOExpanders can be put into a multi IO expander.
#ifndef MAX_ALLOWABLE_DELEGATES
#define MAX_ALLOWABLE_DELEGATES 8
#endif // defined MAX_ALLOWABLE_DELEGATES

typedef uint8_t (*ExpanderOpFn)(IoAbstractionRef ref, uint8_t pin, uint8_t val);

/** 
 * An implementation of the BasicIoAbstraction that provides support for more than one IOExpander
 * in a single abstraction, along with a single set of Arduino pins.
 * Arduino pins will be from 0..arduinoPinsNeeded in the constructor (default 100)
 * expanders will directly follow this, expanders are added using addIoExpander.
 * 
 * Take note that the usual way to use the MultiIoAbstraction is to create a global variable
 * and append the additional IO devices during setup. In order to pass such a varable to
 * the ioDevice functions, such as ioDeviceDigitalRead you must put an ampersand in front
 * of the variable to make it into a pointer.
 */
class MultiIoAbstraction : public BasicIoAbstraction {
private:
	IoAbstractionRef delegates[MAX_ALLOWABLE_DELEGATES];
	pinid_t limits[MAX_ALLOWABLE_DELEGATES];
	uint8_t numDelegates;
public:
	explicit MultiIoAbstraction(pinid_t arduinoPinsNeeded = 100);
	~MultiIoAbstraction() override;
	void addIoExpander(IoAbstractionRef expander, pinid_t numOfPinsNeeded);

	/** 
	 * delegates the pin direction call to whichever abstraction owns the pin, and that
	 * abstraction will then set the pin mode
	 * @param pin the pin to set the mode on
	 * @param mode as per pinMode modes
	 */
	void pinDirection(pinid_t pin, uint8_t mode) override;

	/**
	 * delegates writing the value to whichever abstraction owns the pin, this abstraction
	 * will then write out the value.
	 * @param pin the pin to write to
	 * @param value the value to write to the pin
	 */
	void writeValue(pinid_t pin, uint8_t value) override;

	/**
	 * delegates reading the value from a pin to whichever abstraction owns the pin.
	 * @param pin the pin to read from 
	 */
	uint8_t readValue(pinid_t pin) override;

	/**
	 * delegates writing the port value to the abstraction that owns that pin, the abstraction
	 * will then determine the port that the pin belongs to
	 * @param port any pin within the port
	 * @param portVal an 8 bit value to write direct to the port. 
	 */
	void writePort(pinid_t port, uint8_t portVal) override;

	/**
	 * delegates reading a port to the abstraction that owns the pin, the abstraction will then
	 * determine which port owns the pin and return the port value.
	 * @param port any pin within that port.
	 */
	uint8_t readPort(pinid_t port) override;

	/**
	 * delegates attaching an interrupt to the abstraction that owns the pin, see each abstraction
	 * for more information about how interrupts work with the given device.
	 * @param pin the pin on the device
	 * @param intHandler the interrupt intHandler
	 * @param mode as per arduino interrupt modes.
	 */
	void attachInterrupt(pinid_t pin, RawIntHandler intHandler, uint8_t mode) override;

	/**
	 * will run through all delegate abstractions and sync them
	 */
	bool runLoop() override;
private:
	uint8_t doExpanderOp(pinid_t pin, uint8_t aVal, ExpanderOpFn fn);
};


/**
 * A reference specifically to a MultiIoAbstraction that can be passed to any of the ioDevice calls, but can also have more
 * IO expanders added to it.
 */
typedef MultiIoAbstraction* MultiIoAbstractionRef;

/**
 * Create a multiIoExpander by adding together more than one IoAbstraction, for example Arduino pins plus a few 8574 devices.
 * @param arduinoPinRange the number of pins to assign to Arduino.
 */ 
inline MultiIoAbstractionRef multiIoExpander(pinid_t arduinoPinRange) { return new MultiIoAbstraction(arduinoPinRange); }

/**
 * Add an additional expander to an existing multiIoExpander.
 * @param expander the expander to be added
 * @param pinRange the number of pins needed by the expander.
 */
inline void multiIoAddExpander(MultiIoAbstractionRef multiIo, IoAbstractionRef expander, pinid_t pinRange) { multiIo->addIoExpander(expander, pinRange); }

#include "TaskManagerIO.h"
#include "SwitchInput.h"
#include "IoLogging.h"

#endif
