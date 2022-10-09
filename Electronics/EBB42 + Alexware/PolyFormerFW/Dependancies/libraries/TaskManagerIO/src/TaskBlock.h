/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TASKMANAGERIO_TASKBLOCK_H_
#define _TASKMANAGERIO_TASKBLOCK_H_

#include <TaskPlatformDeps.h>
#include <TaskTypes.h>

/**
 * @file TaskBlock.h
 *
 * An internal class definition that is the representation of a task
 */

/**
 * This is an internal class, and users of the library generally don't see it.
 *
 * Task manager can never deallocate memory that it has already allocated for tasks, this is in order to make thread
 * safety much easier. Given this we allocate tasks in blocks of DEFAULT_TASK_SIZE and each tranche contains it's start
 * and end point in the "array". DEFAULT task size is set to 20 on 32 bit hardware where the size is negligible, 10
 * on MEGA2560 and all other AVR boards default to 6. We allow up to 8 tranches on AVR and up to 16 on 32 bit boards.
 * This should provide more than enough tasks for most boards.
 */
class TaskBlock {
private:
    TimerTask tasks[DEFAULT_TASK_SIZE];
    const taskid_t first;
    const taskid_t tasksSize;
public:
    explicit TaskBlock(taskid_t first_) : first(first_), tasksSize(DEFAULT_TASK_SIZE) {}

    /**
     * Checks if taskId is contained within this block
     * @param task the task ID to check
     * @return true if contained, otherwise false
     */
    bool isTaskContained(taskid_t task) const {
        return task >= first && task < (first + tasksSize);
    };

    TimerTask* getContainedTask(taskid_t task) {
        return isTaskContained(task) ? &tasks[task - first] : nullptr;
    }

    void clearAll() {
        for(taskid_t i=0; i<tasksSize;i++) {
            tasks[i].clear();
        }
    }

    taskid_t allocateTask() {
        for(taskid_t i=0; i<tasksSize; i++) {
            if(tasks[i].allocateIfPossible()) {
                return i + first;
            }
        }
        return TASKMGR_INVALIDID;
    }

    taskid_t lastSlot() const {
        return first + tasksSize - 1;
    }

    taskid_t firstSlot() const {
        return first;
    }
};


#endif //_TASKMANAGERIO_TASKBLOCK_H_
