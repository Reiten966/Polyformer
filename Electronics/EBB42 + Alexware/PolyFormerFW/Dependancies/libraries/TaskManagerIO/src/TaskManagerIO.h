/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANAGER_IO_H
#define TASKMANAGER_IO_H

#include "TaskPlatformDeps.h"
#include "TaskTypes.h"
#include "TaskBlock.h"

#ifdef PARTICLE
enum InterruptMode;
#endif // PARTICLE

/**
 * @file TaskManagerIO.h
 *
 * Task manager is a simple co-routine style implementation for Arduino which supports scheduling work to be done
 * at a given time, repeating tasks, interrupt marshalling and events. It is generally thread safe such that code
 * outside of task manager can add, remove and manage tasks even while task manager is running.
 *
 * Note that you should never add tasks from a raw ISR, instead use task manager's marshalling of interrupts or
 * use an event that triggers on the interrupt occurring.
 *
 * The API for this class is compatible across Arduino, ESP and mbed. It is mainly thread safe on tested platforms.
 *
 * Both Commercial and Community support for task manager are available from http://www.thecoderscorner.com
 */

#ifdef IOA_USE_MBED
#include <cstdint>
/** MBED ONLY: This defines the yield function for mbed boards, as per framework on Arduino */
void yield();
/** MBED ONLY: This defines the millis function for mbed boards by using a timer, as per framework on Arduino */
uint32_t millis();
/** MBED ONLY: This defines the micros function to use the standard mbed us timer, as per framework on Arduino */
uint32_t micros();
#define delayMicroseconds(x) wait_us(x)
#endif // IOA_USE_MBED

/**
 * the definition of an interrupt handler function, to be called back when an interrupt occurs.
 */
typedef void (*RawIntHandler)();

/**
 * Definition of a function to be called back when an interrupt is detected, marshalled by task manager into a task.
 * The pin that caused the interrupt is passed in the parameter on a best efforts basis.
 * @param pin the pin on which the interrupt occurred (best efforts)
 */
typedef void (*InterruptFn)(pintype_t pin);

/**
 * Abstracts the method by which interrupts are registered on the platform. Generally speaking this is implemented by
 * all IoAbstractionRef implementations, so having any abstraction means you already have one of these. You can either
 * implement your own variant of this, or use IoAbstraction to do it for you.
 *
 * For example if using IoAbstraction then the IO object returned by internalDigitalIo() implements this.
 */
class InterruptAbstraction {
public:
    /**
     * Register an interrupt handler to a specific pin on the device, such that the interrupt handler function will
     * be triggered when mode is true. Mode can be one of CHANGE, RISING, FALLING.
     * @param pin the pin on the device to attach an interrupt to
     * @param fn the raw interrupt ISR function
     * @param mode either RISING, FALLING or CHANGE
     */
    virtual void attachInterrupt(pintype_t pin, RawIntHandler fn, uint8_t mode) = 0;
};

class TaskExecutionRecorder;

/**
 * TaskManager is a lightweight cooperative co-routine implementation for Arduino, it works by scheduling tasks to be
 * done either immediately, or at a future point in time. It is quite efficient at scheduling tasks as internally tasks
 * are arranged in time order in a linked list. Tasks can be provided as either a function to be called, or a class
 * that implements the Executable interface. Tasks can be scheduled to run either ASAP, once, or repeated.
 *
 * Events can be added based on extensions of the BaseEvent class, these can be triggered outside of task manager and
 * they can then "wake up" task manager, events can also be polled. In addition interrupts can be marshalled through
 * task manager, any interrupt managed by task manager will be marshalled into a task. IE outside of an ISR.
 *
 * There is a globally defined variable called `taskManager` and you should attach this to your main loop. You can
 * create other task managers on different threads if required. For most use cases, the class is thread safe.
 */
class TaskManager {
protected:
    // the memory that holds all the tasks is an array of task blocks, allocated on demand
    TaskBlock* volatile taskBlocks[DEFAULT_TASK_BLOCKS];  // task blocks never ever move in memory, they are not volatile but the pointer is
    volatile taskid_t numberOfBlocks; // this holds the current number of blocks available.

    // here we have a linked list of tasks, this linked list is in time order, nearest task first.
    tm_internal::TimerTaskAtomicPtr first;

    // interrupt handling variables, store the interrupt state and probable pin cause if applicable
    volatile pintype_t lastInterruptTrigger;
    volatile bool interrupted;
    volatile InterruptFn interruptCallback;

    tm_internal::TmAtomicBool memLockerFlag;      // memory and list operations are locked by this flag using the TmSpinLocker
    tm_internal::TimerTaskAtomicPtr runningTask;
public:
    /**
     * On all platforms there is a default instance of TaskManager called taskManager. You can create other instances
     * that can run on other threads, for example to process long running tasks that should be processed separately.
     */
    TaskManager();
    ~TaskManager();

    /**
     * Executes a task manager task as soon as possible. Useful to add work into task manager from another thread of
     * execution. Shorthand for scheduleOnce(2, task);
     *
     * Why not scheduleOnce with 0 you may ask, taskManager is cooperative which also means it is an unfair scheduler.
     * Given this, it would always add at the front of the queue if the time was 0, but by making the time 2, we ensure
     * it is queued behind other things needing immediate execution.
     *
     * @param workToDo the work to be done
     * @return the task ID that can be queried and cancelled.
     */
    inline taskid_t execute(TimerFn workToDo) {
        return scheduleOnce(2, workToDo, TIME_MICROS);
    }

    /**
	 * Executes a task manager task as soon as possible. Useful to add work into task manager from another thread of
	 * execution. Shorthand for scheduleOnce(2, task);
     *
     * Why not scheduleOnce with 0 you may ask, taskManager is cooperative which also means it is an unfair scheduler.
     * Given this, it would always add at the front of the queue if the time was 0, but by making the time 2, we ensure
     * it is queued behind other things needing immediate execution.
     *
     * @param execToDo the work to be done
     * @param deleteWhenDone default to false, task manager will reclaim the memory when done with this executable.
     * @return the task ID that can be queried and cancelled.
     */
    inline taskid_t execute(Executable* execToDo, bool deleteWhenDone = false) {
        return scheduleOnce(2, execToDo, TIME_MICROS, deleteWhenDone);
    }

    /**
     * Schedules a task for one shot execution in the timeframe provided.
     * @param millis the time frame in which to schedule the task
     * @param timerFunction the function to run at that time
     * @param timeUnit defaults to TIME_MILLIS but can be any of the possible values.
     */
    taskid_t scheduleOnce(uint32_t when, TimerFn timerFunction, TimerUnit timeUnit = TIME_MILLIS);

    /**
     * Schedules a task for one shot execution in the timeframe provided calling back the exec
     * function on the provided class extending Executable.
     * @param millis the time frame in which to schedule the task
     * @param execRef a reference to a class extending Executable
     * @param timeUnit defaults to TIME_MILLIS but can be any of the possible values.
     * @param deleteWhenDone default to false, task manager will reclaim the memory when done with this executable.
     */
    taskid_t scheduleOnce(uint32_t when, Executable* execRef, TimerUnit timeUnit = TIME_MILLIS, bool deleteWhenDone = false);

    /**
     * Schedules a task for repeated execution at the frequency provided.
     * @param millis the frequency at which to execute
     * @param timerFunction the function to run at that time
     * @param timeUnit defaults to TIME_MILLIS but can be any of the possible values.
     */
    taskid_t scheduleFixedRate(uint32_t when, TimerFn timerFunction, TimerUnit timeUnit = TIME_MILLIS);

    /**
     * Schedules a task for repeated execution at the frequency provided calling back the exec
     * method on the provided class extending Executable.
     * @param millis the frequency at which to execute
     * @param execRef a reference to a class extending Executable
     * @param timeUnit defaults to TIME_MILLIS but can be any of the possible values.
     * @param deleteWhenDone true if taskManager should call delete on the object when done, otherwise false.
     */
    taskid_t scheduleFixedRate(uint32_t when, Executable* execRef, TimerUnit timeUnit = TIME_MILLIS, bool deleteWhenDone = false);

    /**
     * Adds an event to task manager that can be triggered either once or can be repeated. See the
     * BaseEvent interface for more details of the interaction with taskManager.
     * @param eventToAdd the event that is added to task manager
     * @param deleteWhenDone true if taskManager should call delete on the object when done, otherwise false.
     */
    taskid_t registerEvent(BaseEvent* eventToAdd, bool deleteWhenDone = false);

    /**
     * Generally used by the BaseEvent class in the markTriggeredAndNotify to indicate that at least
     * one event has now triggered and needs to be evaluated.
     */
    ISR_ATTR void triggerEvents() {
        lastInterruptTrigger = 0xff; // 0xff is the shorthand for event trigger basically.
        interrupted = true;
    }

    /**
     * Adds an interrupt that will be handled by task manager, such that it's marshalled into a task.
     * This registers an interrupt with any IoAbstractionRef.
     * @param ref the Interrupt abstraction (or IoAbstractionRef) that we want to register the interrupt for
     * @param pin the pin upon which to register (on the IoDevice above)
     * @param mode the mode in which to register, eg. CHANGE, RISING, FALLING
     */
    void addInterrupt(InterruptAbstraction* interruptAbstraction, pintype_t pin, uint8_t mode);

    /**
     * Sets the interrupt callback to be used when an interrupt is signalled. Note that you will be
     * called back by task manager, and you are safe to use any variables as normal. Task manager
     * marshalls the interrupt for you.
     * @param handler the interrupt handler
     */
    void setInterruptCallback(InterruptFn handler);

    /**
     * Stop a task from executing or cancel it from executing again if it is a repeating task
     * @param task the task ID returned from the schedule call
     */
    void cancelTask(taskid_t task);

    /**
     * Sets a tasks enable status. An enabled task is scheduled whereas a disabled task is not scheduled. Note that
     * after disabling a task it may run one more time before switching state. Upon re-enablement then task manager
     * will put the item back into the run queue if needed.
     * @param task the task to change status
     * @param ena the enablement status
     */
    void setTaskEnabled(taskid_t task, bool ena);

    /**
     * Use instead of delays or wait loops inside code that needs to perform timing functions. It will
     * not call back until at least `micros` time has passed.
     * @param micros the number of microseconds to wait.
     */
    virtual void yieldForMicros(uint32_t micros);

    /**
     * This should be called in the loop() method of your sketch, ensure that your loop method does
     * not do anything that will unduly delay calling this method.
     */
    void runLoop();

    /**
     * Used internally by the interrupt handlers to tell task manager an interrupt is waiting. Not for external use.
     */
    static void markInterrupted(pintype_t interruptNo);

    /**
     * Reset the task manager such that all current tasks are cleared, back to power on state.
     */
    void reset() {
        // all the slots should be cleared
        for(taskid_t i =0; i<numberOfBlocks; i++) {
            taskBlocks[i]->clearAll();
        }
        // the queue must be completely cleared too.
        tm_internal::atomicWritePtr(&first, nullptr);
    }

    /**
     * This method fills slotData with the current running conditions of each available task slot.
     * Useful for debugging purposes, where each task the .
     * Key:
     * * 'R' repeating task  ('r' when currently running)
     * * 'U' one shot task ('u' when currently running)
     * * 'F' free slot (should never be 'f')
     * @param slotData the char array to fill in with task information. Must be as long as number of tasks + 1.
     * @param slotDataSize the size of the array passed in (normally using sizeof).
     */
    char* checkAvailableSlots(char* slotData, size_t slotDataSize) const;

    /**
     * Gets the first task in the run queue. Not often useful outside of testing.
     */
    TimerTask* getFirstTask() {
        return tm_internal::atomicReadPtr(&first);
    }

    /**
     * Gets the underlying TimerTask variable associated with this task ID.
     * @param task the task's ID
     * @return the task or nullptr.
     */
    TimerTask* getTask(taskid_t task);

    /**
     * Gets the number of microseconds as an unsigned long to the next task execution.
     * To convert to milliseconds: divide by 1000, to seconds divide by 1,000,000.
     * @return the microseconds from now to next execution
     */
    uint32_t microsToNextTask() {
        auto maybeTask = tm_internal::atomicReadPtr(&first);
        if(maybeTask == nullptr) return 600 * 1000000U; // wait for 10 minutes if there's nothing to do
        else return maybeTask->microsFromNow();
    }

    /**
     * Gets the currently running task, this is only useful for places where re-entrant checking is needed to ensure
     * the same task is taking the lock again for example. Never change the task state in this call, and also never
     * store this pointer.
     * @return a temporary pointer to the running task that lasts as long as it is running.
     */
    TimerTask* getRunningTask() { return runningTask; }

    friend class TaskExecutionRecorder;
private:
    /**
     * Finds and allocates the next free task, once this returns a task will either have been allocated, making task
     * manager storage bigger if needed, or it will return TASKMGR_INVALIDID otherwise.
     * @return either a taskID or TASKMGR_INVALIDID if a slot could not be allocated
     */
    taskid_t findFreeTask();

    /**
     * Removes an item from the task queue, so it is no longer in the run linked list. Note that there is a certain
     * amount of concurrency and it's possible that this may coincide with the task running.
     *
     * Thread safety: Must only be called on the queue
     * @param task the task to remove
     */
    void removeFromQueue(TimerTask* task);

    /**
     * Puts an item into the queue in time order, so the first to execute is at the top of the list.
     * @param tm the task to be added.
     */
    void putItemIntoQueue(TimerTask* tm);

    /**
     * When an interrupt occurs, this goes through all active tasks
     */
    void dealWithInterrupt();
};

/** the global task manager, this would normally be associated with the main runLoop. */
extern TaskManager taskManager;

/**
 * Converts a duration in milliseconds to microseconds.
 */
#define millisToMicros(x) ((x)*1000UL)

/**
 * Converts a duration in seconds to microseconds.
 */
#define secondsToMicros(x) ((x)*1000000UL)

/**
 * Converts a duration in seconds to milliseconds.
 */
#define secondsToMillis(x) ((x)*1000UL)

#endif //TASKMANAGER_IO_H
