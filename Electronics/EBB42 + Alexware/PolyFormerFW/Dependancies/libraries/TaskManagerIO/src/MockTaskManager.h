/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef MOCKED_TASK_MANAGER_H
#define MOCKED_TASK_MANAGER_H

/**
 * @file MockTaskManager.h
 * 
 * This file contains a version of task manager that is useful for dev & testing.
 * None of the implementations in this file are designed for production use.
 */

#include "TaskManager.h"

/** 
 * This adds a few extra capabilities to task manager for testing. Never call the runLoop
 * method, as it will try and schedule directly. Instead manually use the helper methods
 * to run the scheduled tasks manually during test running.
 */
class SimulatedTaskManager : public TaskManager {
private:
    uint32_t yieldTimes[10]{};
    uint8_t numOfYields{};
public:
    SimulatedTaskManager() {
        reset();
    }

    void reset() {
        numOfYields = 0;
        TaskManager::reset();
    }

    void yieldForMicros(uint32_t micros) override {
        if(numOfYields >= 10) return;
        yieldTimes[numOfYields++] = micros;
    }

    int getNumberOfYieldCalls() {return numOfYields;}
    uint32_t getYieldTime(int i) {return yieldTimes[i];}
    int getMaxTaskNo() {return taskBlocks[numberOfBlocks - 1]->lastSlot();}
    InterruptFn getInterruptFunction() {return interruptCallback;}
};

#endif // MOCKED_TASK_MANAGER_H
