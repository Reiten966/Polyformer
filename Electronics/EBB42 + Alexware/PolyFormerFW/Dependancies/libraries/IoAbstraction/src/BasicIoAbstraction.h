/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _IO_ABSTRACTION_CORE_TYPES
#define _IO_ABSTRACTION_CORE_TYPES

/**
 * @file BasicIoAbstraction.h
 *
 * Provides the core IoAbstraction interface and Arduino implementation of that interface.
 */
#include "PlatformDetermination.h"
#include <TaskManagerIO.h>

#if defined(IOA_USE_MBED)
# include "mbed/MbedDigitalIO.h"
#elif defined(ESP32) && defined(IOA_USE_ESP32_EXTRAS)
# include "esp32/ESP32DigitalIO.h"
#else
# include <Arduino.h>
#endif //IOA_USE_MBED

/**
 * This class provides the interface by which all `IoAbstractionRef` types work. It makes it possible to
 * treat many types of IO in the same way, by providing a standard way of dealing with Arduino pins, 
 * shift registers and IO expanders.
 * 
 * As well as providing the standard interface, it also provides the Arduino pin implementation.
 * 
 * Normally, to use an IoAbstraction one would use the helper functions available, for this IoAbstraction
 * the helper function is `ioUsingArduino`
 */
class BasicIoAbstraction : public InterruptAbstraction {
private:
#ifdef IOA_USE_MBED
    BtreeList<uint32_t, GpioWrapper> pinCache;

    GpioWrapper *allocatePinIfNeedBe(uint8_t pinToAlloc);
#endif //IOA_USE_MBED
public:
	virtual ~BasicIoAbstraction() = default;

	/**
	 * sets the pin direction for a pin controlled by this abstraction - as per `pinMode`
	 * @param pin the pin to be changed
	 * @param mode the new mode, as per pinMode (or on Mbed you can use PinMode enum values)
	 */
	virtual void pinDirection(pinid_t pin, uint8_t mode);

	/**
	 * Writes a value to a pin on this abstraction, as per `digitalWrite`. For serial devices, may need a sync first.
	 * @param pin the pin to be written to 
	 * @param value the new value such as HIGH / LOW
	 */
	virtual void writeValue(pinid_t pin, uint8_t value);

	/**
	 * Reads a value from a pin for this abstraction as per `digitalRead`. For serial devices may need a sync first.
	 * @param pin the pin to be read 
	 */
	virtual uint8_t readValue(pinid_t pin);
	
	/**
	 * Attach an interrupt to this abstraction, regardless of the device location this will perform the required tasks to register
	 * the interrupt in the requested mode. Note that not all devices can support all modes, for arduino pins all modes are supported. 
	 * @param pin the pin on this device to be used
	 * @param intHandler a void function with no parameters, used to handle interrupts. THIS IS A RAW INTERRUPT AND NOT MARSHALLED
	 * @param mode standard Arduino interrupt modes: CHANGE, RISING, FALLING
	 */
	virtual void attachInterrupt(pinid_t pin, RawIntHandler interruptHandler, uint8_t mode);

	/**
	 * This method is not needed on Arduino pins, but for most serial implementations it causes the device and abstraction to be synced.
	 * Returns true if the write call worked, normally true, false indicates error
	 */
	virtual bool runLoop() { return true; }

	/**
	 * Writes out a whole port at once, on Arduino pins this is achieved by providing any pin within that port.
	 * On Arduino pins you should take care not to use ports that are providing core functions.
	 * @param pin the pin determines the hardware port to use.
	 * @param portVal the 8 bit value to write to the port. Use with care. 
	 */
	virtual void writePort(pinid_t pin, uint8_t portVal);

	/**
	 * Reads a whole port at once, on Arduino pins this is achieved by providing any pin within that port.
	 * On Arduino pins you should take care not to use ports that are providing core functions.
	 * @param pin the pin determines the hardware port to use.
	 * @return the 8 bit value read from the port.
	 */
	virtual uint8_t readPort(pinid_t pin);
};

/** 
 * A reference to any type of IoAbstraction, that can be passed to one of the ioDevice.. functions. Normally, a reference
 * is obtained using a helper function, such as `ioUsingArduino`.
 */
typedef BasicIoAbstraction* IoAbstractionRef;

/**
 * Gives a reference to the Arduino pin implementation of IoAbstraction.
 */
#ifdef IOA_USE_MBED
#define pgm_read_byte_near(x) (*(x))
#elif defined(IOA_USE_ARDUINO)
#define ioUsingArduino internalDigitalIo
#endif

/**
 * @return the device abstraction for digital pins, on ESP32 this abstraction does not need Arduino present.
 */
IoAbstractionRef internalDigitalIo();

/**
 * Works in the same way as regular `pinMode` but this works for any IoAbstractionRef that you wish to set the pin mode on. 
 * @param ioDev the previously created IoAbstraction
 * @param the pin on the device to change mode 
 * @param the mode such as INPUT, OUTPUT, INPUT_PULLUP
 */
inline void ioDevicePinMode(IoAbstractionRef ioDev, pinid_t pin, uint8_t dir) { ioDev->pinDirection(pin, dir); }

/**
 * Works in the same way as `digitalRead`, but this works for any `IoAbstractionRef`, on the serial versions, the port is
 * cached, so before calling this method you need to have called `ioDeviceSync` first.
 * @see ioDeviceDigitalReadS for a shortcut version that syncs first, but is less efficient if there are many reads
 * @param ioDev the previously created IoAbstraction
 * @param the pin on the device to read 
 */
inline uint8_t ioDeviceDigitalRead(IoAbstractionRef ioDev, pinid_t pin) { return ioDev->readValue(pin); }

/**
 * Works in the same way as digitalWrite, but this works for any `IoAbstractionRef`, on the serial versions, the port is
 * cached, so before calling this method, you need to have called `IoDeviceSync` first.
 * @see ioDeviceDigitalWriteS for a shortcut version that syncs after writing, but is less efficient with many writes.
 * @param ioDev the previously created IoAbstraction
 * @param pin the pin to be updated
 * @param val the new value for the pin, HIGH/LOW
 */
inline void ioDeviceDigitalWrite(IoAbstractionRef ioDev, pinid_t pin, uint8_t val) { ioDev->writeValue(pin, (val)); }

/**
 * On serial versions of this abstraction, this causes synchronization to both the read and write values, so the chip and this IoAbstraction
 * are in sync.
 * @param ioDev the IoAbstraction to be synchronised.
 */ 
inline bool ioDeviceSync(IoAbstractionRef ioDev) { return ioDev->runLoop(); }

/**
 * Attach an interrupt to any IoAbstraction, regardless of the device location this will perform the required tasks to register
 * the interrupt in the requested mode. Note that not all devices can support all modes, check the device datasheet and the
 * particular abstraction you are using carefully, to ensure you get the expected result. 
 * @param ioDev the IoAbstraction to use
 * @param pin the pin on this device to be used
 * @param intHandler a void function with no parameters, used to handle interrupts. THIS IS A RAW INTERRUPT AND NOT MARSHALLED
 * @param mode standard Arduino interrupt modes: CHANGE, RISING, FALLING
 */
inline void ioDeviceAttachInterrupt(IoAbstractionRef ioDev, pinid_t pin, RawIntHandler intHandler, uint8_t mode) {ioDev->attachInterrupt(pin, intHandler, mode) ;}

/**
 * Works in the same way as `digitalRead`, but this works for any `IoAbstractionRef`, unlike the non 'S' version this automatically
 * calls ioDeviceSync first.
 * @see ioDeviceDigitalRead where you have more than one read / write to do at once, performs better in those cases.
 * @param ioDev the previously created IoAbstraction
 * @param the pin on the device to read 
 */
inline uint8_t ioDeviceDigitalReadS(IoAbstractionRef ioDev, pinid_t pin) { ioDev->runLoop(); return ioDev->readValue(pin); }

/**
 * Works in the same way as digitalWrite, but this works for any `IoAbstractionRef`, unlike the non 'S' version this automatically
 * calls ioDeviceSync first.
 * @see ioDeviceDigitalWrite where you have more than one read / write to do at once, performs better in those cases.
 * @param ioDev the previously created IoAbstraction
 * @param pin the pin to be updated
 * @param val the new value for the pin, HIGH/LOW
 */
inline bool ioDeviceDigitalWriteS(IoAbstractionRef ioDev, pinid_t pin, uint8_t val) { ioDev->writeValue(pin, (val)); return ioDev->runLoop(); }

/**
 * Write a whole 8 bit byte onto the port that the pin belongs to. For example if pin 42 where on PORTH then this would write to PORTH.
 * For i2c and shift registers, it works the same, but the results are more predictable.
 * 
 * This version calls ioDeviceSync automatically after doing the write
 * 
 * @param ioDev the previously created IoAbstraction
 * @param pinOnPort any pin belonging to the port
 * @param val the new value for the port
 */
inline bool ioDeviceDigitalWritePortS(IoAbstractionRef ioDev, pinid_t pinOnPort, uint8_t portVal) { ioDev->writePort(pinOnPort, portVal); return ioDev->runLoop(); }

/**
 * Reads a whole 8 bit value back from the port with automatic sync before the operation. Specify the pin on the port that you wish to read.
 * For example if pin 42 where on PORTH then this would write to PORTH. For i2c and shift registers, it works the same, but the results are 
 * more predictable.
 * 
 * This version calls ioDeviceSync automatically before doing the read.
 * 
 * @param ioDev the previously created IoAbstraction
 * @param pin any pin belonging to the port to be read
 * @return the value of the port.
 */
inline uint8_t ioDeviceDigitalReadPortS(IoAbstractionRef ioDev, pinid_t pinOnPort) { ioDev->runLoop(); return ioDev->readPort(pinOnPort);  }

/**
 * Write a whole 8 bit byte onto the port that the pin belongs to. For example if pin 42 where on PORTH then this would write to PORTH.
 * For i2c and shift registers, it works the same, but the results are more predictable.
 * 
 * This version does not automatically sync, call the 'S' variant for that.
 * 
 * @param ioDev the previously created IoAbstraction
 * @param pinOnPort any pin belonging to the port
 * @param val the new value for the port
 */
inline void ioDeviceDigitalWritePort(IoAbstractionRef ioDev, pinid_t pinOnPort, uint8_t portVal) { ioDev->writePort(pinOnPort, portVal); }

/**
 * Reads a whole 8 bit value back from the port with automatic sync before the operation. Specify the pin on the port that you wish to read.
 * For example if pin 42 where on PORTH then this would write to PORTH. For i2c and shift registers, it works the same, but the results are 
 * more predictable.
 * 
 * This version does not automatically sync, call the 'S' variant for that.
 * 
 * @param ioDev the previously created IoAbstraction
 * @param pin any pin belonging to the port to be read
 * @return the value of the port.
 */
inline uint8_t ioDeviceDigitalReadPort(IoAbstractionRef ioDev, pinid_t pinOnPort) { return ioDev->readPort(pinOnPort);  }

#endif // _IO_ABSTRACTION_CORE_TYPES
