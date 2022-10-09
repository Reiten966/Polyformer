#ifdef ESP32

#include <AUnit.h>
#include <TaskManagerIO.h>
#include "test_utils.h"
#include <pthread.h>

using namespace aunit;

int executions = 0;
int failedCreations = 0;

void callbackFunction() {
    executions++;
}

void* creationThread(void* param) {
    Serial.println("Start thread to create tasks");

    for(int i = 0; i < 500; i++) {
        if(TASKMGR_INVALIDID == taskManager.execute(callbackFunction)) failedCreations++;
        delay(5);
    }

    Serial.println("Completed task creation");
}

void* scheduledCreationThread(void* param) {
    Serial.println("Start thread to schedule tasks");

    for(int i = 0; i < 250; i++) {
        if(TASKMGR_INVALIDID == taskManager.scheduleOnce(15, callbackFunction)) failedCreations++;
        delay(10);
    }

    Serial.println("Completed scheduled task creation");
}

/**
 * when I create a lot of tasks over three threads that are either immediate executed or near immediately executed
 * then they should all be scheduled and there should be no failures
 */
test(multiThreadedTestForESP) {
    pthread_t threadId1;
    pthread_t threadId2;
    pthread_t threadId3;
    pthread_create(&threadId1, NULL, creationThread, (void*)1);
    pthread_create(&threadId2, NULL, creationThread, (void*)2);
    pthread_create(&threadId3, NULL, scheduledCreationThread, (void*)3);

    unsigned long startTime = millis();

    // wait until the task is marked as scheduled.
    while(executions < 2000 && (millis() - startTime) < 5000) {
        taskManager.yieldForMicros(10000);
    }

    assertEqual(1250, executions);
    assertEqual(0, failedCreations);
}

#endif
