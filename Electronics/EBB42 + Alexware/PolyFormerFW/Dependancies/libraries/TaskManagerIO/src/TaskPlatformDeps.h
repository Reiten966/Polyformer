/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */


#ifndef TASKMANAGERIO_PLATFORMDETERMINATION_H
#define TASKMANAGERIO_PLATFORMDETERMINATION_H

class TimerTask;

#if defined(__MBED__) || defined(ARDUINO_ARDUINO_NANO33BLE)

#if defined(TM_ENABLE_CAPTURED_LAMBDAS)
#define TM_ALLOW_CAPTURED_LAMBDA
#endif

// check if this is Arduino mbed or regular mbed.
// list of devices is pulled from https://github.com/arduino/ArduinoCore-mbed/blob/master/full.variables
// set TMIOA_FORCE_ARDUINO_MBED to force IoAbstraction to use Arduino-mbed mode.
#if defined(ARDUINO_NANO_RP2040_CONNECT) || \
    defined(ARDUINO_ARDUINO_NANO33BLE) || \
    defined(ARDUINO_RASPBERRY_PI_PICO) || \
    defined(ARDUINO_PORTENTA_H7_M7) || \
    defined(ARDUINO_PORTENTA_H7_M4) || \
    defined(ARDUINO_EDGE_CONTROL) || \
    defined(ARDUINO_NICLA) || \
    defined(ARDUINO_NICLA_VISION) || \
    defined(TMIOA_FORCE_ARDUINO_MBED)
# define IOA_USE_ARDUINO
# define ARDUINO_MBED_MODE
# include "Arduino.h"
# define IOA_MULTITHREADED
#include "rtos/rtos.h"
inline void* getCurrentThreadId() { return rtos::ThisThread::get_id(); }
#else
# define IOA_USE_MBED
# include "mbed.h"
# define IOA_MULTITHREADED
inline void* getCurrentThreadId() { return ThisThread::get_id(); }

# if !defined(PIO_NEEDS_RTOS_WORKAROUND)
#  include "rtos.h"
# endif // PIO_NEED_RTOS_WORKAROUND
#endif // mbed and arduino-mbed checks

#include <mbed_atomic.h>
typedef uint32_t pintype_t;

namespace tm_internal {
    typedef TimerTask* volatile TimerTaskAtomicPtr;
    typedef volatile bool TmAtomicBool;

    /**
     * Sets the boolean to the new value ONLY when the existing value matches expected.
     * @param ptr the bool memory location to compare / swap
     * @param expected the expected value
     * @param newValue the replacement, replaced on if expected matches
     * @return true if the replacement was done, otherwise false
     */
    inline bool atomicSwapBool(volatile bool *ptr, bool expected, bool newValue) {
        return core_util_atomic_cas_bool(ptr, &expected, newValue);
    }

    /**
     * Reads the value in an atomic boolean object
     */
    inline bool atomicReadBool(TmAtomicBool *pPtr) {
        return *pPtr;
    }

    inline void atomicWriteBool(TmAtomicBool *pPtr, bool newVal) {
        *pPtr = newVal;
    }

    /**
     * Dereferences and returns the value of the pointer at ptr type. On mbed boards this is already an atomic operation and
     * therefore volatile is enough.
     * @tparam PTR_TYPE class type of the pointer
     * @param pPtr reference to memory of the pointer
     * @return the pointer.
     */
    inline TimerTask *atomicReadPtr(TimerTaskAtomicPtr *pPtr) {
        return *pPtr;
    }

    /**
     * Dereferences and then sets the memory of the pointer type. On mbed boards this is already an atomic operation
     * @tparam PTR_TYPE
     * @param pPtr
     * @param newValue
     */
    inline void atomicWritePtr(TimerTaskAtomicPtr *pPtr, TimerTask * volatile newValue) {
        //core_util_atomic_store_ptr((void* volatile*)pPtr,  newValue);
        *pPtr = newValue;
    }
}
#elif defined(ESP8266) || defined(ESP32)
#include "Arduino.h"
typedef uint8_t pintype_t;
# define IOA_USE_ARDUINO
#if defined(TM_ENABLE_CAPTURED_LAMBDAS)
# define TM_ALLOW_CAPTURED_LAMBDA
#endif

#if defined(ESP8266)
#include <atomic>
namespace tm_internal {

    typedef std::atomic<TimerTask *> TimerTaskAtomicPtr;

    typedef std::atomic<uint32_t> TmAtomicBool;

    /**
     * Sets the boolean to the new value ONLY when the existing value matches expected.
     * @param ptr the bool memory location to compare / swap
     * @param expected the expected value
     * @param newValue the replacement, replaced on if expected matches
     * @return true if the replacement was done, otherwise false
     */
    inline bool atomicSwapBool(TmAtomicBool *ptr, bool expected, bool newValue) {
        // compare and swap is not implemented on ESP8266
        auto ret = false;
        noInterrupts();
        if(ptr->load() == expected) {
            ptr->store(newValue);
            ret = true;
        }
        interrupts();
        return ret;
    }

    /**
     * Reads an atomic boolean value
     * @param pPtr the pointer to an atomic boolean value
     * @return the boolean value.
     */
    inline bool atomicReadBool(TmAtomicBool *pPtr) {
        return pPtr->load();
    }

    /**
     * Writes a boolean value atomically
     * @param pPtr the atomic ref
     * @param newVal the new value
     */
    inline void atomicWriteBool(TmAtomicBool *pPtr, bool newVal) {
        pPtr->store(newVal);
    }

    /**
    * Dereferences and returns the value of the pointer at ptr type. On mbed boards this is already an atomic operation and
    * therefore volatile is enough.
    * @tparam PTR_TYPE class type of the pointer
    * @param pPtr reference to memory of the pointer
    * @return the pointer.
    */
    inline TimerTask *atomicReadPtr(TimerTaskAtomicPtr *pPtr) {
        return pPtr->load();
    }

    /**
     * Dereferences and then sets the memory of the pointer type. On mbed boards this is already an atomic operation
     * @tparam PTR_TYPE
     * @param pPtr
     * @param newValue
     */
    inline void atomicWritePtr(TimerTaskAtomicPtr *pPtr, TimerTask *newValue) {
        pPtr->store(newValue);
    }
}
#else
# define IOA_MULTITHREADED
inline void* getCurrentThreadId() { return xTaskGetCurrentTaskHandle() ; }

namespace tm_internal {

    typedef TimerTask* volatile TimerTaskAtomicPtr;
    typedef volatile uint32_t TmAtomicBool; // to use CAS, the bool must be 32 bits wide
    inline bool atomicSwapBool(TmAtomicBool *ptr, bool expected, bool newValue) {
        uint32_t exp32 = expected;
        uint32_t new32 = newValue;
        uxPortCompareSet(ptr, exp32, &new32);
        return new32 == expected;
    }

    /**
     * Reads an atomic boolean value
     * @param pPtr the pointer to an atomic boolean value
     * @return the boolean value.
     */
    inline bool atomicReadBool(TmAtomicBool *pPtr) {
        return *pPtr != 0;
    }

    /**
     * Writes a boolean value atomically
     * @param pPtr the atomic ref
     * @param newVal the new value
     */
    inline void atomicWriteBool(TmAtomicBool *pPtr, bool newVal) {
        *pPtr = newVal;
    }

    /**
     * Dereferences and returns the value of the pointer at ptr type. On mbed boards this is already an atomic operation and
     * therefore volatile is enough.
     * @tparam PTR_TYPE class type of the pointer
     * @param pPtr reference to memory of the pointer
     * @return the pointer.
     */
    inline TimerTask *atomicReadPtr(TimerTaskAtomicPtr *pPtr) {
        return *pPtr;
    }

    /**
     * Dereferences and then sets the memory of the pointer type. On mbed boards this is already an atomic operation
     * @tparam PTR_TYPE
     * @param pPtr
     * @param newValue
     */
    inline void atomicWritePtr(TimerTaskAtomicPtr *pPtr, TimerTask *newValue) {
        *pPtr = newValue;
    }
}
#endif

#else
// fall back to using Arduino regular logic, works for all single core boards. If we end up here for a multicore
// board then there may be problems. Here we are in full arduino mode (AVR, MKR etc).
# define IOA_USE_ARDUINO
#include <Arduino.h>
typedef uint8_t pintype_t;

namespace tm_internal {
typedef TimerTask *volatile TimerTaskAtomicPtr;
typedef volatile bool TmAtomicBool;

    static bool atomicSwapBool(volatile bool* ptr, bool expected, bool newValue) {
        bool ret = false;
        noInterrupts();
        if(*ptr == expected) {
            *ptr = newValue;
            ret = true;
        }
        interrupts();
        return ret;
    }

    inline bool atomicReadBool(volatile bool *ptr) {
        return *ptr;
    }

    inline void atomicWriteBool(volatile bool *ptr, bool newVal) {
        *ptr = newVal;
    }

#if defined(__AVR__)
    inline void atomicWritePtr(TimerTaskAtomicPtr* pPtr, TimerTask* newValue) {
        noInterrupts();
        *pPtr = newValue;
        interrupts();
    }

    inline TimerTask* atomicReadPtr(TimerTaskAtomicPtr* pPtr) {
        noInterrupts();
        auto ptr = *pPtr;
        interrupts();
        return ptr;
    }
#else
// all other supported Arduino boards are atomic for pointer types
inline void atomicWritePtr(TimerTaskAtomicPtr* pPtr, TimerTask* newValue) {
        *pPtr = newValue;
    }

    inline TimerTask* atomicReadPtr(TimerTaskAtomicPtr* pPtr) {
        return *pPtr;
    }
#endif // AVR check for PTR atomicity
}
#endif // All platform checks

// for all mbed and ESP boards we already enable lambda captures, SAMD is a known extra case that works.
// we can only enable on larger boards with enough memory to take the extra size of the structures.
#if defined(TM_ENABLE_CAPTURED_LAMBDAS) && defined(ARDUINO_ARCH_SAMD)
# define TM_ALLOW_CAPTURED_LAMBDA
#endif

//
// Scheduling size. On all boards by default task manager uses 32 bit schedule data to make it more general purpose.
// Note that even on 8 bit boards, all the math still needs to be 32 bit to deal with times, so there is very little
// to no performance gain by doing this.
//
// If you need the few extra bytes back, and can live with 16 bit schedule values then define TM_FORCE_16BIT_SCHEDULER
//
#if defined(TM_FORCE_16BIT_SCHEDULER)
typedef uint16_t sched_t;
#else
typedef uint32_t sched_t;
#endif // TM_FORCE_16BIT_SCHEDULER

//
// DEFAULT_TASK_SIZE definition:
// TaskManagerIO will re-allocate the task array if needed. So there's little need to adjust this for most use cases.
// For AVR boards: MEGA 2560 has a default size of 10, others have a default size of 6.
// For all other boards the default size is 16. You can change it by defining DEFAULT_TASK_SIZE yourself.
//
#ifndef DEFAULT_TASK_SIZE
#ifdef __AVR_ATmega2560__
# define DEFAULT_TASK_SIZE 12
# define DEFAULT_TASK_BLOCKS 8
#elif defined(__AVR__)
# define DEFAULT_TASK_SIZE 6
# define DEFAULT_TASK_BLOCKS 4
#else
# define DEFAULT_TASK_SIZE 16
# define DEFAULT_TASK_BLOCKS 16
#endif // platform
#else
#ifndef DEFAULT_TASK_BLOCKS
#define DEFAULT_TASK_BLOCKS 8
#endif // DEFAULT_TASK_BLOCKS not defined when task size is
#endif // DEFAULT_TASK_SIZE defined already

namespace tm_internal {
    enum TmErrorCode {
        /** reallocating memory by adding another block, number of blocks in the second parameter */
        TM_INFO_REALLOC = 1,
        /** A task slot has been allocated, taskid in second parameter */
        TM_INFO_TASK_ALLOC = 2,
        /** A task slot has been freed, taskid in second parameter */
        TM_INFO_TASK_FREE = 3,

        // warnings and errors are over 100, info level 0..99

        /** probable bug, the lock status was not as expected, please report along with sketch to reproduce  */
        TM_ERROR_LOCK_FAILURE = 100,
        /** high concurrency is resulting in very high spin counts, performance may be affected */
        TM_WARN_HIGH_SPINCOUNT,
        /** task manager is full, consider settings the default task settings higher. */
        TM_ERROR_FULL
    };
    typedef void (*LoggingDelegate)(TmErrorCode code, int task);
    extern LoggingDelegate loggingDelegate;
    void setLoggingDelegate(LoggingDelegate delegate);

    inline void tmNotification(TmErrorCode code, int task) {
        if(loggingDelegate) loggingDelegate(code, task);
    }
}

//
// Here we define an attribute needed for interrupt support on ESP8266 and ESP32 boards, any interrupt code that is
// going to run on these boards should be marked with this attribute.
//
#undef ISR_ATTR
#if defined(ESP8266) || defined(ESP32)
# define ISR_ATTR IRAM_ATTR
#else
# define ISR_ATTR
#endif

//
// Here we have one last go at determining if we should enable capture lambdas by checking if the functional include
// is available, we only do so if we are on GCC > 5
//
# if !defined(TM_ALLOW_CAPTURED_LAMBDA) && defined(TM_ENABLE_CAPTURED_LAMBDAS) && __GNUC__ >= 5
#if __has_include(<functional>)
# define TM_ALLOW_CAPTURED_LAMBDA
#endif // _has_include
#endif // GCC>=5 and !TM_ALLOW_CAPTURED_LAMBDA

#endif //TASKMANGERIO_PLATFORMDETERMINATION_H
