/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "SCThreadingSupport.h"

#ifdef SIMPLE_COLLECTIONS_ARM_SUPPORT
bool casAtomic(position_ptr_t ptr, position_t expected, position_t newVal) {
    uint32_t res;
    /** here we load the expected(R1), newValue(R2), and existing(R0). Existing is loaded exclusively so that any
     * change to the memory is tracked. We ensure R0 and R1 are the same and if they are we try the exchange.
     * The result of the exchange is turned into a boolean where true means the value was set. It must be marked
     * volatile as it has side effects the compiler may not aware of and should never be reordered or optimized.
     * Reference was https://developer.arm.com/documentation/ddi0301/h/level-two-interface/synchronization-primitives/example-of-ldrex-and-strex-usage
     * Support is not as good as suggested, and only starts I think around cortex M4, see this wiki link it is not in V6 thumb profile.
     * https://en.wikipedia.org/wiki/ARM_architecture#Built_on_ARM_Cortex_Technology_licence
     */
    __asm volatile(
        "        LDREX R0, [%[ptr_in]]\n"
        "        CMP   R0, %[regExp]\n"
        "        BNE   SWP_FL\n"
        "        STREX %[result], %[regNew], [%[ptr_in]]\n"
        "        B     SWP_DNE\n"
        "SWP_FL: MOV   %[result], #1\n"
        "SWP_DNE:"
        : [result] "=r" (res)
        : [ptr_in] "r" (ptr), [regExp] "r" (expected), [regNew] "r" (newVal)
        : "r0", "cc");
    return res == 0;
}
#endif

// In this case we are only partially thread safe, we block against interrupts firing but that is about all we can do
// For AVR and smaller ARM devices that don't support pre-emptive threads this may be enough.
#ifdef NEEDS_CAS_EMULATION
bool casAtomic(position_ptr_t ptr, position_t expected, position_t newVal) {
    auto ret = false;
    noInterrupts();
    if(*ptr == expected) {
        *ptr = newVal;
        ret = true;
    }
    interrupts();
    return ret;
}

#endif
