/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "TaskPlatformDeps.h"
#include "TaskManagerIO.h"
#include "ExecWithParameter.h"

TaskManager taskManager;

namespace tm_internal {
    LoggingDelegate loggingDelegate = nullptr;

    void setLoggingDelegate(LoggingDelegate delegate) {
        loggingDelegate = delegate;
    }
}

class TmSpinLock {
private:
    tm_internal::TmAtomicBool* lockObject;
public:
    explicit TmSpinLock(tm_internal::TmAtomicBool* toLockOn) : lockObject(toLockOn) {
        bool locked;
        int count = 0;
        do {
            locked = tm_internal::atomicSwapBool(lockObject, false, true);
            if(!locked) {
                yield(); // something else has the lock, let it get control to finish up!
                if(++count == 1000) tm_internal::tmNotification(tm_internal::TM_WARN_HIGH_SPINCOUNT, TASKMGR_INVALIDID);
            }
        } while(!locked);
    }

    ~TmSpinLock() {
        if(!tm_internal::atomicSwapBool(lockObject, true, false)) {
            tm_internal::tmNotification(tm_internal::TM_ERROR_LOCK_FAILURE, TASKMGR_INVALIDID);
        }
    }
};


ISR_ATTR void TaskManager::markInterrupted(pintype_t interruptNo) {
	taskManager.lastInterruptTrigger = interruptNo;
	taskManager.interrupted = true;
}

TaskManager::TaskManager() : taskBlocks {} {
	interrupted = false;
	tm_internal::atomicWritePtr(&first, nullptr);
	interruptCallback = nullptr;
	lastInterruptTrigger = 0;
	taskBlocks[0] = new TaskBlock(0);
	numberOfBlocks = 1;
	runningTask = nullptr;
	tm_internal::atomicWriteBool(&memLockerFlag, false);
}

TaskManager::~TaskManager() {
    for(taskid_t i=0; i<numberOfBlocks; i++) {
        delete taskBlocks[i];
    }
}

taskid_t TaskManager::findFreeTask() {
    int retries = 0;
    while(retries < 100) {
        for (taskid_t i=0; i<numberOfBlocks;i++) {
            auto taskId = taskBlocks[i]->allocateTask();
            if(taskId != TASKMGR_INVALIDID) {
                tm_internal::tmNotification(tm_internal::TM_INFO_TASK_ALLOC, taskId);
                return taskId;
            }
        }

        // already full, cannot allocate further.
        if(numberOfBlocks == DEFAULT_TASK_BLOCKS) {
            tm_internal::tmNotification(tm_internal::TM_ERROR_FULL, TASKMGR_INVALIDID);
            return TASKMGR_INVALIDID;
        }

        // now we need to take an atomic lock memLockerFlag before proceeding to ensure nobody else is allocating tasks
        // if two threads come here at once, only one will be able to enter the allocate block, the other will go into
        // the else block and let other tasks run for a while and then loop again
        {
            TmSpinLock spinLock(&memLockerFlag);
            auto nextIdSpace = taskBlocks[numberOfBlocks - 1]->lastSlot() + 1;
            taskBlocks[numberOfBlocks] = new TaskBlock(nextIdSpace);
            if(taskBlocks[numberOfBlocks] != nullptr) {
                tm_internal::tmNotification(tm_internal::TM_INFO_REALLOC, numberOfBlocks);
                numberOfBlocks++;
            }
            else {
                tm_internal::tmNotification(tm_internal::TM_ERROR_FULL, numberOfBlocks);
                break;  // no point to continue here, new has failed.
            }
        }

        // count up the tries so far to allocate / wait for allocation.
        retries++;
    }

	return TASKMGR_INVALIDID;
}

taskid_t TaskManager::scheduleOnce(uint32_t when, TimerFn timerFunction, TimerUnit timeUnit) {
	auto taskId = findFreeTask();
	if (taskId != TASKMGR_INVALIDID) {
        auto task = getTask(taskId);
        task->initialise(when, timeUnit, timerFunction, false);
		putItemIntoQueue(task);
	}
	return taskId;
}

taskid_t TaskManager::scheduleFixedRate(uint32_t when, TimerFn timerFunction, TimerUnit timeUnit) {
	auto taskId = findFreeTask();
	if (taskId != TASKMGR_INVALIDID) {
        auto task = getTask(taskId);
        task->initialise(when, timeUnit, timerFunction, true);
		putItemIntoQueue(task);
	}
	return taskId;
}

taskid_t TaskManager::scheduleOnce(uint32_t when, Executable* execRef, TimerUnit timeUnit, bool deleteWhenDone) {
	auto taskId = findFreeTask();
	if (taskId != TASKMGR_INVALIDID) {
	    auto task = getTask(taskId);
		task->initialise(when, timeUnit, execRef, deleteWhenDone, false);
		putItemIntoQueue(task);
	}
	return taskId;
}

taskid_t TaskManager::scheduleFixedRate(uint32_t when, Executable* execRef, TimerUnit timeUnit, bool deleteWhenDone) {
	auto taskId = findFreeTask();
	if (taskId != TASKMGR_INVALIDID) {
        auto task = getTask(taskId);
        task->initialise(when, timeUnit, execRef, deleteWhenDone, true);
		putItemIntoQueue(task);
	}
	return taskId;
}

taskid_t TaskManager::registerEvent(BaseEvent *eventToAdd, bool deleteWhenDone) {
    auto taskId = findFreeTask();
    if(taskId != TASKMGR_INVALIDID) {
        auto task = getTask(taskId);
        task->initialiseEvent(eventToAdd, deleteWhenDone);
        putItemIntoQueue(task);
    }
    return taskId;
}

void TaskManager::setTaskEnabled(taskid_t taskId, bool ena) {
    auto task = getTask(taskId);
    if(task == nullptr || !task->isInUse()) return;

    task->setEnabled(ena);

    if(ena) putItemIntoQueue(task);
}

void TaskManager::cancelTask(taskid_t taskId) {
    auto task = getTask(taskId);
    // always create a new task to ensure the task is never, ever cancelled on anything other than the task thread.
	if (task) {
	    taskManager.execute(new ExecWith2Parameters<TimerTask*, TaskManager*>([](TimerTask* task, TaskManager* tm) {
            tm->removeFromQueue(task);
            tm_internal::tmNotification(tm_internal::TM_INFO_TASK_FREE, TASKMGR_INVALIDID);
            task->clear();
	    }, task, this), true);
	}
}

void TaskManager::yieldForMicros(uint32_t microsToWait) {
	yield();

	auto* prevTask = getRunningTask();
	unsigned long microsStart = micros();
	do {
        runLoop();
	} while((micros() - microsStart) < microsToWait);
	tm_internal::atomicWritePtr(&runningTask, prevTask);
}

class TaskExecutionRecorder {
private:
    TimerTask* prevTask;
    TaskManager* taskMgr;
public:
    TaskExecutionRecorder(TaskManager* tm, TimerTask* task) : taskMgr(tm) {
        prevTask = tm->getRunningTask();
        tm_internal::atomicWritePtr(&tm->runningTask, task);
    }

    ~TaskExecutionRecorder() {
        tm_internal::atomicWritePtr(&taskMgr->runningTask, prevTask);
    }
};

void TaskManager::dealWithInterrupt() {
    interrupted = false;
    if(interruptCallback != nullptr) interruptCallback(lastInterruptTrigger);

    auto lastSlot = taskBlocks[numberOfBlocks - 1]->lastSlot() + 1;
    for(taskid_t i=0; i<lastSlot; i++) {
        auto* task = getTask(i);
        if(task->isInUse() && task->isEvent()) {
            if (!task->isRunning()) {
                TaskExecutionRecorder taskExecutionRecorder(this, task);
                task->processEvent();
                removeFromQueue(task);
                if (task->isRepeating()) {
                    putItemIntoQueue(task);
                }
                else {
                    task->clear();
                    tm_internal::tmNotification(tm_internal::TM_INFO_TASK_FREE, TASKMGR_INVALIDID);
                }
            }
            else {
                interrupted = true; // we have to assume we still need to process this event next time around.
            }
        }
    }
}

void TaskManager::runLoop() {
	// when there's an interrupt, we marshall it into a timer interrupt.
	if (interrupted) dealWithInterrupt();

	// go through the timer (scheduled) tasks in priority order. they are stored
	// in a linked list ordered by first to be scheduled. So we only go through
	// these until the first one that isn't ready.
	TimerTask* tm = tm_internal::atomicReadPtr(&first);

    while (tm && tm->microsFromNow() == 0) {
        if(!tm->isRunning()) {
            // by here we know that the task is in use. If it's in use nothing will touch it until it's marked as
            // available. We can do this part without a lock, knowing that we are the only thing that will touch
            // the task. We further know that all non-immutable fields on TimerTask are volatile.
            TaskExecutionRecorder executionRecorder(this, tm);
            tm->execute();
            removeFromQueue(tm);
            if (tm->isRepeating()) {
                putItemIntoQueue(tm);
            } else {
                tm->clear();
                tm_internal::tmNotification(tm_internal::TM_INFO_TASK_FREE, TASKMGR_INVALIDID);
            }
        }
        tm = tm->getNext();

#if defined(ESP8266) || defined(ESP32)
        if(tm) {
            // here we are making extra sure we are good citizens on ESP boards
	        yield();
	    }
#endif
    }
}

void TaskManager::putItemIntoQueue(TimerTask* tm) {
    // we must own the lock before adding to the queue, as someone else could be removing.
    TmSpinLock spinLock(&memLockerFlag);

    // we can never schedule a task that is not enabled.
    if(!tm->isEnabled()) return;

    auto theFirst = tm_internal::atomicReadPtr(&first);

	// shortcut, no first yet, so we are at the top!
	if (theFirst == nullptr) {
        tm->setNext(nullptr);
        tm_internal::atomicWritePtr(&first, tm);
		return;
	}

	// if we need to execute now or before the next task, then we are first. For performance, we use unfair semantics
	if (theFirst->microsFromNow() >= tm->microsFromNow()) {
        tm->setNext(theFirst);
		tm_internal::atomicWritePtr(&first, tm);
        return;
	}

	// otherwise we have to find the place in the queue for this item by time
	TimerTask* current = theFirst->getNext();
	TimerTask* previous = theFirst;

	while (current != nullptr) {
		if (current->microsFromNow() > tm->microsFromNow()) {
            tm->setNext(current);
            previous->setNext(tm);
			return;
		}
		previous = current;
		current = current->getNext();
	}

	// we are at the end of the queue
    tm->setNext(nullptr);
	previous->setNext(tm);
}

void TaskManager::removeFromQueue(TimerTask* tm) {
	// Thread Safety notes:
	// This must never be called on anything other than the task manager thread. There's only two places that this
	// is used, firstly after execution of a task, this is already on the task manager thread, secondly when used
	// from cancelTask, this is now marshalled back onto task manager as a task to remove the item.

    // we must own the lock before we can modify the queue, as someone else could otherwise be adding..
    TmSpinLock spinLock(&memLockerFlag);
    auto theFirst = tm_internal::atomicReadPtr(&first);

    // there must be at least one item to proceed.
    if(theFirst == nullptr) return;

    // shortcut, if we are first, just remove us by getting the next and setting first.
	if (theFirst == tm) {
        tm_internal::atomicWritePtr(&first, tm->getNext());
        tm->setNext(nullptr);
        return;
	}

	// otherwise, we have a single linked list, so we need to keep previous and current and
	// then iterate through each item
	TimerTask* current = theFirst->getNext();
	TimerTask* previous = theFirst;

	while (current != nullptr) {

		// we've found the item, unlink it from the queue and nullify its next.
		if (current == tm) {
			previous->setNext(current->getNext());
			current->setNext(nullptr);
			break;
		}

		previous = current;
		current = current->getNext();
	}
}

ISR_ATTR void interruptHandler1() {
	taskManager.markInterrupted(1);
}
ISR_ATTR void interruptHandler2() {
	taskManager.markInterrupted(2);
}
ISR_ATTR void interruptHandler3() {
	taskManager.markInterrupted(3);
}
ISR_ATTR void interruptHandler4() {
	taskManager.markInterrupted(4);
}
ISR_ATTR void interruptHandler5() {
	taskManager.markInterrupted(5);
}
ISR_ATTR void interruptHandler6() {
	taskManager.markInterrupted(6);
}
ISR_ATTR void interruptHandler7() {
	taskManager.markInterrupted(7);
}
ISR_ATTR void interruptHandler8() {
	taskManager.markInterrupted(8);
}
ISR_ATTR void interruptHandler9() {
	taskManager.markInterrupted(9);
}
ISR_ATTR void interruptHandler10() {
	taskManager.markInterrupted(10);
}
ISR_ATTR void interruptHandler11() {
	taskManager.markInterrupted(11);
}
ISR_ATTR void interruptHandler12() {
	taskManager.markInterrupted(12);
}
ISR_ATTR void interruptHandler13() {
	taskManager.markInterrupted(13);
}
ISR_ATTR void interruptHandler14() {
	taskManager.markInterrupted(14);
}
ISR_ATTR void interruptHandler15() {
	taskManager.markInterrupted(15);
}
ISR_ATTR void interruptHandler18() {
	taskManager.markInterrupted(18);
}
ISR_ATTR void interruptHandlerOther() {
	taskManager.markInterrupted(0xff);
}

void TaskManager::addInterrupt(InterruptAbstraction* ioDevice, pintype_t pin, uint8_t mode) {
	if (interruptCallback == nullptr) return;

	switch (pin) {
	case 1: ioDevice->attachInterrupt(pin, interruptHandler1, mode); break;
	case 2: ioDevice->attachInterrupt(pin, interruptHandler2, mode); break;
	case 3: ioDevice->attachInterrupt(pin, interruptHandler3, mode); break;
	case 4: ioDevice->attachInterrupt(pin, interruptHandler4, mode); break;
	case 5: ioDevice->attachInterrupt(pin, interruptHandler5, mode); break;
	case 6: ioDevice->attachInterrupt(pin, interruptHandler6, mode); break;
	case 7: ioDevice->attachInterrupt(pin, interruptHandler7, mode); break;
	case 8: ioDevice->attachInterrupt(pin, interruptHandler8, mode); break;
	case 9: ioDevice->attachInterrupt(pin, interruptHandler9, mode); break;
	case 10: ioDevice->attachInterrupt(pin, interruptHandler10, mode); break;
	case 11: ioDevice->attachInterrupt(pin, interruptHandler11, mode); break;
	case 12: ioDevice->attachInterrupt(pin, interruptHandler12, mode); break;
	case 13: ioDevice->attachInterrupt(pin, interruptHandler13, mode); break;
	case 14: ioDevice->attachInterrupt(pin, interruptHandler14, mode); break;
	case 15: ioDevice->attachInterrupt(pin, interruptHandler15, mode); break;
	case 18: ioDevice->attachInterrupt(pin, interruptHandler18, mode); break;
	default: ioDevice->attachInterrupt(pin, interruptHandlerOther, mode); break;
	}
}

void TaskManager::setInterruptCallback(InterruptFn handler) {
	interruptCallback = handler;
}

char* TaskManager::checkAvailableSlots(char* data, size_t dataSize) const {
    auto maxLen = min(taskid_t(dataSize - 1), taskBlocks[numberOfBlocks - 1]->lastSlot());
    size_t position = 0;

    for(taskid_t i=0; i<numberOfBlocks;i++) {
	    auto last = taskBlocks[i]->lastSlot() + 1;
	    for(taskid_t j=taskBlocks[i]->firstSlot(); j < last; j++) {
	        auto task = taskBlocks[i]->getContainedTask(j);
            data[position] = task->isRepeating() ? 'R' : (task->isInUse() ? 'U' : 'F');
            if (task->isRunning()) data[position] = tolower(data[position]);
            position++;
	    }
	    if(position >= maxLen) break;
	}
	data[position] = 0;
	return data;
}

TimerTask *TaskManager::getTask(taskid_t taskId) {
    for(taskid_t i=0; i<numberOfBlocks; i++) {
        auto possibleTask = taskBlocks[i]->getContainedTask(taskId);
        if(possibleTask != nullptr) return possibleTask;
    }
    return nullptr;
}

#ifdef IOA_USE_MBED

volatile bool timingStarted = false;
Timer ioaTimer;

void yield() {

# if !defined(PIO_NEEDS_RTOS_WORKAROUND)
    ThisThread::yield();
#else
    wait(0.0000001);
#endif
}

unsigned long millis() {
    if(!timingStarted) {
        timingStarted = true;
        ioaTimer.start();
    }
    return ioaTimer.read_ms();
}

unsigned long micros() {
    if(!timingStarted) {
        timingStarted = true;
        ioaTimer.start();
    }
    return (unsigned long) ioaTimer.read_high_resolution_us();
}

#endif
