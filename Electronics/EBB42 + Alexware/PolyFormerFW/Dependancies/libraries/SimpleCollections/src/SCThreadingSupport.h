/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef SIMPLECOLLECTIONS_SCTHREADINGSUPPORT_H
#define SIMPLECOLLECTIONS_SCTHREADINGSUPPORT_H

/**
 * @file SCThreadingSupport.h
 * Contains two definitions that are board specific that allow the circular buffer to be thread safe on a wide range of
 * boards. On mbed and Arduino mbed based boards to uses the mbed utility for CAS. On STM32 M4 and above ARM based
 * boards we use LDREX/STREX to create an atomic write situation. On ESP32 we use the FreeRTOS CAS operation, and on all
 * other boards we disable interrupts around the check. This should work for nearly all cases, even with multiple threads
 * and interrupt usage.
 */

#include <inttypes.h>

// START PROCESSOR/BOARD SELECTION BLOCK
#if defined(__MBED__) || defined(TMIOA_FORCE_ARDUINO_MBED)
#include <mbed_atomic.h>
typedef volatile uint32_t* position_ptr_t;
typedef volatile uint32_t position_t;
inline bool casAtomic(position_ptr_t ptr, position_t expected, position_t newVal) {
    uint32_t exp = expected;
    return core_util_atomic_cas_u32(ptr, &exp, newVal);
}
inline position_t readAtomic(position_ptr_t ptr) { return *(ptr); }
#elif (defined(SC_USE_ARM_ASM_CAS) || defined(ARDUINO_ARCH_STM32)) && !defined(SC_NO_ARM_ASM_CAS)
#include <Arduino.h>
#if __CORTEX_M > 0x03U
#define SIMPLE_COLLECTIONS_ARM_SUPPORT
typedef volatile uint32_t* position_ptr_t;
typedef volatile uint32_t position_t;
bool casAtomic(position_ptr_t ptr, position_t expected, position_t newVal);
inline uint32_t readAtomic(position_ptr_t ptr) { return *(ptr); }
#else
typedef volatile uint32_t* position_ptr_t;
typedef volatile uint32_t position_t;
#define NEEDS_CAS_EMULATION
#endif // __CORTEX_M > 3
#elif defined(ESP8266)
#include <Arduino.h>
typedef volatile uint32_t* position_ptr_t;
typedef volatile uint32_t position_t;
#define NEEDS_CAS_EMULATION
#elif defined(ESP32)
#include <Arduino.h>
typedef volatile uint32_t* position_ptr_t;
typedef volatile uint32_t position_t;
inline bool casAtomic(position_ptr_t ptr, position_t expected, position_t newVal) {
    uint32_t exp32 = expected;
    uint32_t new32 = newVal;
    uxPortCompareSet(ptr, exp32, &new32);
    return new32 == expected;
}
inline uint16_t readAtomic(position_ptr_t ptr) { return *(ptr); }
#else
#include <Arduino.h>
typedef volatile uint16_t* position_ptr_t;
typedef volatile uint16_t position_t;
#define NEEDS_CAS_EMULATION
#endif // PROCESSOR/BOARD SELECTION BLOCK

#ifdef NEEDS_CAS_EMULATION
bool casAtomic(position_ptr_t ptr, position_t expected, position_t newVal);
inline uint16_t readAtomic(position_ptr_t ptr) { return *ptr; }
#endif // NEEDS_CAS_EMULATION

#endif //SIMPLECOLLECTIONS_SCTHREADINGSUPPORT_H
