

#include <BasicInterruptAbstraction.h>
#include "PlatformDetermination.h"
#include "BasicIoAbstraction.h"

#if defined(IOA_USE_ARDUINO) && !(defined(ESP32) && defined(IOA_USE_ESP32_EXTRAS))

void BasicIoAbstraction::pinDirection(pinid_t pin, uint8_t mode) {
    pinMode(pin, mode);
}

void BasicIoAbstraction::writeValue(pinid_t pin, uint8_t value) {
    digitalWrite(pin, value);
}

uint8_t BasicIoAbstraction::readValue(pinid_t pin) {
    return digitalRead(pin);
}

void BasicIoAbstraction::attachInterrupt(pinid_t pin, RawIntHandler interruptHandler, uint8_t mode) {
    internalHandleInterrupt(pin, interruptHandler, mode);
}


void BasicIoAbstraction::writePort(pinid_t port, uint8_t portVal) {
#ifndef IOA_ARDUINO_MBED
	*portOutputRegister(digitalPinToPort(port)) = portVal;
#endif
}

uint8_t BasicIoAbstraction::readPort(pinid_t port) {
#ifndef IOA_ARDUINO_MBED
	return *portInputRegister(digitalPinToPort(port));
#else
    return 0;
#endif
}


IoAbstractionRef arduinoAbstraction = NULL;
IoAbstractionRef ioUsingArduino() {
    if (arduinoAbstraction == NULL) {
        arduinoAbstraction = new BasicIoAbstraction();
    }
    return arduinoAbstraction;
}

#endif
