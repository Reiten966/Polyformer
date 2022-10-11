/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANAGERIO_RENTRANTYIELDINGLOCK_H
#define TASKMANAGERIO_RENTRANTYIELDINGLOCK_H

#include "TaskManagerIO.h"

/**
 * A very simple lock that can be used to provide a very simple mutex like behaviour based on task manager
 * atomic constructs. It has the ability to try and spin lock, and also to fully lock in conjunction with
 * TaskMgrLock class. Use only for activities that do not take very long, it cannot relinquish control to
 * task manager, it freezes the bus when locked. You must never call yieldForMicros while locked.
 */
class SimpleSpinLock {
private:
    tm_internal::TimerTaskAtomicPtr initiatingTask;
#if defined(IOA_MULTITHREADED)
    volatile void* currentThread = nullptr;
#endif
    tm_internal::TmAtomicBool locked;
    volatile uint8_t count;
public:
    /**
     * Construct a lock that represents this object
     */
    SimpleSpinLock() {
        initiatingTask = nullptr;
        locked = false;
        count = 0;
    }

    /**
     * Take the lock waiting the longest possible time for it to become available.
     */
    void lock() {
        spinLock(0xFFFFFFFFUL);
    }

    /**
     * tries the lock and returns immediately if it was already locked by us.
     * @return true if the lock was already owned by us, otherwise false
     */
    bool tryLock();

    /**
     * Attempt to take the lock using a spin wait, it only returns true if the lock was taken.
     * @param number of iterations to wait, each iteration is at least 100 mics
     * @return true if the lock was taken, otherwise false
     */
    bool spinLock(unsigned long iterations);

    /**
     * Release the lock taken by spinlock or lock. DOES NOT check that the callee is correct so use carefully.
     */
    void unlock();

    uint8_t getLockCount() const { return count; }

    bool isLocked() const { return locked; }
};

/**
 * A wrapper around the task manager locking facilities that allow you to lock within a block of code by putting
 * an instance on the stack. Be very careful not to use yieldForMicros along with this method of locking.
 * For example:
 *
 * ```
 * SimpleSpinLock myLock;
 * void myFunctionToLock() {
 *     TaskSafeLock(myLock);
 *     // do some work that needs the lock here. lock will always be released.
 *     // never use yieldForMicros(..) here or your code may deadlock.
 * }
 * ```
 */
class TaskMgrLock {
private:
    SimpleSpinLock& lock;

public:
    TaskMgrLock(SimpleSpinLock& theLock) : lock(theLock) {
        lock.lock();
    }

    ~TaskMgrLock() {
        lock.unlock();
    }
};

#endif //TASKMANAGERIO_RENTRANTYIELDINGLOCK_H
