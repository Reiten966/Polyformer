/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANAGER_IO_TASKTYPES_H
#define TASKMANAGER_IO_TASKTYPES_H

/**
 * @file TaskTypes.h
 *
 * This class represents the core tasks that are added to task manager, and the TimerTask object itself.
 */

#include "TaskPlatformDeps.h"

#define TASKMGR_INVALIDID 0xffffU

/**
 * Represents the identifier of a task, it can be used to query, alter and cancel tasks. You should not rely on
 * any characteristics of this type, it could change later, it is essentially no more than a handle to a task.
 */
typedef unsigned int taskid_t;

/**
 * Any class extending from executable can be passed by reference to task manager and the exec() method will be called
 * when the scheduled time is reached.
 */
class Executable {
public:
    /**
     * Called when the schedule is reached
     */
    virtual void exec() = 0;

    /**
     * A virtual destructor must always be provided for interfaces that can be allocated with new.
     */
    virtual ~Executable() = default;
};

// forward references to TaskManager to avoid circular include.
class TaskManager;
/** the global task manager instance that would normally be associated with the main loop */
extern TaskManager taskManager;

/**
 * BaseEvent objects represent events that can be managed by task manager. We can create a base event as either
 * a global variable, or as a new object that lasts at least as long as it's registered with task manager.
 *
 * Events work differently to other tasks, there are two methods to implement, the first method timeOfNextCheck is
 * for events that are polled, and you control how often it is polled by the value you return. To trigger an event
 * you can call setTriggered(bool), and this will set a flag that task manager will look at during the next poll. If
 * you are in the timeOfNextCheck and you set the event as triggered, it will execute immediately. If you are in an
 * interrupt triggered by task manager, it will act on the trigger immediately. However, in other cases you can call
 * markTriggeredAndNotify which additionally notifies task manager, making the effect immediate.
 */
class BaseEvent : public Executable {
private:
    TaskManager *taskMgrAssociation;
    volatile bool triggered;
    volatile bool finished;
public:
    explicit BaseEvent(TaskManager *taskMgrToUse = &taskManager) :
            taskMgrAssociation(taskMgrToUse), triggered(false), finished(false) {}

    /**
     * This method must be implemented by all event handlers. It will be called when the event is first registered with
     * task manager, then it will be called in whatever number of micros you return from the call. For polling events
     * you'd set this to whatever frequency you needed to poll on. For interrupt / threaded situations, it may be set
     * to a very large value, as you would not need to poll.
     * @return the number of microseconds before calling this method again.
     */
    virtual uint32_t timeOfNextCheck() = 0;

    /**
     * Set the event as triggered, which means it will be executed next time around. Usually prefer to use
     * markTriggeredAndNotify as this also notifies task manager.
     */
    void setTriggered(bool t) {
        triggered = t;
    }

    /**
     * @return true if the event has triggered and requires execution now
     */
    bool isTriggered() const {
        return triggered;
    }

    /**
     * @return true if the task has completed
     */
    bool isComplete() const {
        return finished;
    }

    /**
     * Sets the task as completed or not completed. Can be called from interrupt safely. Warning once this is called
     * assume that task manager will remove this from the queue immediately. Further, if deleteWhenDone was true,
     * the object would be deleted almost immediately.
     */
    void setCompleted(bool complete) {
        finished = complete;
    }

    /**
     * Set the event as triggered and then tell task manager that the event needs to be
     * re-evaluated. It will not run immediately, rather it will do the equivalent of
     * triggering an interrupt through taskManager. If this event already receives an
     * interrupt through task manager, then there is no need to call notify.
     */
    void markTriggeredAndNotify();
};

/**
 * Definition of a function to be called back when a scheduled event is required. Takes no parameters, returns nothing.
 */
#ifdef TM_ALLOW_CAPTURED_LAMBDA
#include <functional>
typedef std::function<void()> TimerFn;
#else
typedef void (*TimerFn)();
#endif

/**
 * The time units that can be used with the schedule calls.
 */
enum TimerUnit : uint8_t {
    /** Schedule the task in microseconds */
    TIME_MICROS = 0,
    /** Schedule the task in seconds */
    TIME_SECONDS = 1,
    /** Schedule the task in milliseconds - the default */
    TIME_MILLIS = 2,

    TM_TIME_REPEATING = 0x10,
    TM_TIME_RUNNING = 0x20,
};

/**
 * Internal class.
 * The execution types stored internally in a task, records what kind of task is in use, and if it needs deleting
 * when clearing the timer task.
 */
enum ExecutionType : uint8_t {
    EXECTYPE_FUNCTION = 0,
    EXECTYPE_EXECUTABLE = 1,
    EXECTYPE_EVENT = 2,

    EXECTYPE_MASK = 0x03,
    EXECTYPE_DELETE_ON_DONE = 0x08,

    EXECTYPE_DEL_EXECUTABLE = EXECTYPE_EXECUTABLE | EXECTYPE_DELETE_ON_DONE,
    EXECTYPE_DEL_EVENT = EXECTYPE_EVENT | EXECTYPE_DELETE_ON_DONE
};

/**
 * Internal class that represents a single task slot. You should never have to deal with this class in user code.
 *
 * Represents a single task or event that will be processed at some point in time. It stores the last evaluation time
 * and also the execution parameters. EG execute every 100 millis.
 */
class TimerTask {
private:
#ifdef TM_ALLOW_CAPTURED_LAMBDA
    /** the thing that needs to be executed when the time is reached or event is triggered */
    volatile union {
        Executable *taskRef;
        BaseEvent *eventRef;
    };
    TimerFn callback;
#else
    /** the thing that needs to be executed when the time is reached or event is triggered */
    volatile union {
        TimerFn callback;
        Executable *taskRef;
        BaseEvent *eventRef;
    };
#endif

    /** TimerTask is essentially stored in a linked list by time in TaskManager, this represents the next item */
    tm_internal::TimerTaskAtomicPtr next;

    /** the time at which the task was last scheduled, used to compare against the current time */
    volatile uint32_t scheduledAt;
    /** The timing information for this task, or it's interval */
    volatile sched_t myTimingSchedule;

    // 8 bit values start here.

    /** The timing information for this task IE, millis, seconds or micros and if it's running or repeating. */
    volatile TimerUnit timingInformation;
    /** An atomic flag used to indicate if the task is in use, it should be set before doing anything to a task */
    tm_internal::TmAtomicBool taskInUse;
    /** the mode in which the task executes, IE call a function, call an event or executable. Also if memory is owned */
    volatile ExecutionType executeMode;
    /** Stores a flag to indicate if the task is enabled */
    tm_internal::TmAtomicBool taskEnabled;
public:
    TimerTask();

    /**
     * @return the number of microseconds before execution is to take place, 0 means it's due or past due.
     */
    unsigned long microsFromNow();

    /**
     * Initialise a task slot with execution information
     * @param executionInfo the time of execution
     * @param unit the unit of time measurement
     * @param execCallback the function to call back
     */
    void initialise(sched_t when, TimerUnit unit, TimerFn execCallback, bool repeating);

    /**
     * Initialise a task slot with execution information
     * @param executionInfo the time of execution
     * @param unit the unit of time measurement
     * @param executable the class instance to call back
     * @param deleteWhenDone indicates taskmanager owns this memory and should delete it when clear is called.
     */
    void initialise(sched_t when, TimerUnit unit, Executable *executable, bool deleteWhenDone, bool repeating);

    /**
     * Initialise an event structure, which will call the event immediately to get the next poll time
     * @param event the event object reference
     * @param deleteWhenDone if task manager owns it, if true, it will be deleted when clear is called.
     */
    void initialiseEvent(BaseEvent *event, bool deleteWhenDone);

    /**
     * Called by all the initialise methods to actually do the initial scheduling.
     * @param when when the task is to take place.
     * @param unit the time unit upon which it will occur.
     */
    void handleScheduling(sched_t when, TimerUnit unit, bool repeating);

    /**
     * Atomically checks if the task is in use at the moment.
     * @return true if in use, otherwise false.
     */
    bool isInUse() { return tm_internal::atomicReadBool(&taskInUse); }

    /**
     * Checks if this task is a repeating task.
     * @return true if repeating, otherwise false.
     */
    bool isRepeating() const;

    /**
     * Take a task out of use and clear down all it's fields. Clears the in use flag last for thread safety
     */
    void clear();

    /**
     * Checks if it is possible to allocaate this task, IE that it is presently not in use.
     * @return true if it can be allocated, otherwise false
     */
    bool allocateIfPossible() {
        return tm_internal::atomicSwapBool(&taskInUse, false, true);
    }

    /**
     * Marks the task as in a running condition, this prevents the task being re-entered if it yields.
     */
    void markRunning() { timingInformation = TimerUnit(timingInformation | TM_TIME_RUNNING); }

    /**
     * Clears the running state of the task, thus allowing it to be scheduled again.
     */
    void clearRunning() { timingInformation = TimerUnit(timingInformation & ~TM_TIME_RUNNING); }

    /**
     * @return true if the task is running at the moment, otherwise false. See above running flag methods.
     */
    bool isRunning() const { return (timingInformation & TM_TIME_RUNNING) != 0; }

    /**
     * @return true if this timer is representing an event class, otherwise false
     */
    bool isEvent() {
        auto execType = ExecutionType(executeMode & EXECTYPE_MASK);
        return (isInUse() && execType == EXECTYPE_EVENT);
    }

    /**
     * Task manager holds a linked list of TimerTask, linked by the next field. It is atomically referenced
     * @return the next task in the linked list
     */
    TimerTask *getNext() { return tm_internal::atomicReadPtr(&next); }

    /**
     * Task manager holds a linked list of TimerTask, linked by the next field. It is atomically referenced
     * @param nextTask the new next pointer
     */
    void setNext(TimerTask *nextTask) { tm_internal::atomicWritePtr(&this->next, nextTask); }

    /**
     * actually does the execution of the task, or in the case of an event, it runs through the processEvent method.
     */
    void execute();

    /**
     * This method processes an event in full.
     *
     * * If it is triggered it executes it
     * * If it is complete, it clears it
     * * Otherwise it calls timeOfNextCheck and reschedules it.
     */
    void processEvent();

    /**
     * @return true if the task is on a microsecond schedule
     */
    bool isMicrosSchedule()  { return (timingInformation & 0x0fU)==TIME_MICROS; }
    /**
     * @return true if the task in on a millisecond schedule
     */
    bool isMillisSchedule()  { return (timingInformation & 0x0fU)==TIME_MILLIS; }

    /**
     * @return if the task is presently enabled - IE it is being scheduled.
     */
    bool isEnabled() { return tm_internal::atomicReadBool(&taskEnabled); }

    /**
     * Set the task aspi either enabled or disabled. When enabled it is scheduled, otherwise it is not scheduled.
     * @param ena the enablement status
     */
    void setEnabled(bool ena) { tm_internal::atomicWriteBool(&taskEnabled, ena); }
};

#endif //TASKMANAGER_IO_TASKTYPES_H
