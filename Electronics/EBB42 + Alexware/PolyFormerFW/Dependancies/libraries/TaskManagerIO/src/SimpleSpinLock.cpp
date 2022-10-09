/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "SimpleSpinLock.h"

bool SimpleSpinLock::tryLock() {
#if defined(IOA_MULTITHREADED)
    if(locked && getCurrentThreadId() != currentThread) return false;
    return (locked && taskManager.getRunningTask() == initiatingTask);
#else
    // We are not on RTOS, if our task already owns the lock then we are good.
    return (locked && taskManager.getRunningTask() == initiatingTask);
#endif
}

bool SimpleSpinLock::spinLock(unsigned long iterations) {
    if(tryLock()) {
        ++count;
        return true;
    }

    // otherwise we contend to get the lock in a spin wait until we exhaust the micros provided.
    while(iterations) {
        if (tm_internal::atomicSwapBool(&locked, false, true)) {
            tm_internal::atomicWritePtr(&initiatingTask, taskManager.getRunningTask());
#if defined(IOA_MULTITHREADED)
            currentThread = (void*)getCurrentThreadId();
#endif
            ++count;
            return true;
        }
        else {
            delayMicroseconds(50);
        }
        --iterations;
    }

    return false;
}

void SimpleSpinLock::unlock() {
    if(count == 0) {
        return;
    }

    --count;

    if(count == 0) {
        tm_internal::atomicWritePtr(&initiatingTask, nullptr);
        tm_internal::atomicWriteBool(&locked, false);

#if defined(IOA_MULTITHREADED)
        currentThread = nullptr;
#endif

    }
}

