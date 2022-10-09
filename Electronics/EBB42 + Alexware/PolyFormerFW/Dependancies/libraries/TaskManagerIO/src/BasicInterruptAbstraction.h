/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANAGERIO_BASICINTERRUPTABSTRACTION_H
#define TASKMANAGERIO_BASICINTERRUPTABSTRACTION_H

#include <TaskPlatformDeps.h>
#include <TaskManagerIO.h>

#ifdef IOA_USE_ARDUINO

inline void internalHandleInterrupt(pintype_t pin, RawIntHandler fn, uint8_t mode) {
#if defined(ARDUINO_MBED_MODE) || (ARDUINO_API_VERSION >= 10200)
    ::attachInterrupt(pin, fn, (PinStatus)mode);
#elif defined(PARTICLE)
    ::attachInterrupt(pin, fn, (InterruptMode)mode);
#else
    ::attachInterrupt(digitalPinToInterrupt(pin), fn, mode);
#endif // Interrupt mode conditionals
}

/**
 * For Arduino devices when NOT using IoAbstraction, this is the minimum possible implementation that can call
 * through to the Arduino platform ::attachInterrupt call. You can pass a pointer to one of these to the task manager
 * interrupt functions. If you are using IoAbstraction, all IoAbstractionRef's implement InterruptAbstraction.
 */
class BasicArduinoInterruptAbstraction : public InterruptAbstraction {
    void attachInterrupt(pintype_t pin, RawIntHandler fn, uint8_t mode) override {
        internalHandleInterrupt(pin, fn, mode);
    }
};
#endif // IOA_USE_ARDUINO - Arduino Only

#endif //TASKMANAGERIO_BASICINTERRUPTABSTRACTION_H
