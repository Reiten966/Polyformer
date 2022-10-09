
#include <AUnit.h>
#include <TaskManagerIO.h>
#include "test_utils.h"

using namespace aunit;

int counts[6];

void testCall1() {
    counts[0]++;
}

void testCall2() {
    counts[1]++;
}

void testCall3() {
    counts[2]++;
}

void testCall4() {
    counts[3]++;
    taskManager.scheduleOnce(1, testCall4, TIME_SECONDS);
}

void testCall6() {
    counts[5]++;
    if(counts[5] < 10) taskManager.scheduleOnce(500, testCall6);
}

void testCall5() {
    counts[4]++;
    taskManager.scheduleOnce(1000, testCall6);
}


//
// This test actually runs the task manager full up with tasks for around 15 seconds, scheduling using
// lots of different intervals from micros through to seconds, using both single shot and repeating
// schedules, this is the most important test to pass in the whole suite.
//

class HighThroughputFixture : public TestOnce {
public:
    /**
     * This method checks that taskmanager tasks are in proper and stable order. And that only running tasks
     * are in the linked list.
     */
    void assertTasksAreInOrder() {
        bool inOrder = true;
        TimerTask* task = taskManager.getFirstTask();
        unsigned long prevTaskMicros = 0;
        while(inOrder && task != nullptr) {
            unsigned long currentTaskMicros = task->microsFromNow();
            // the task must be in use
            inOrder = inOrder && task->isInUse();

            // we then compare it in order, obviously millis tick slower than micros
            if(currentTaskMicros < prevTaskMicros && (prevTaskMicros - currentTaskMicros) > 1000) {
                inOrder = false;
                Serial.print("Failed prev "); Serial.print(prevTaskMicros);
                Serial.print(", current ");Serial.println(currentTaskMicros);
            }

            // get the next item and store this micros for next compare.
            prevTaskMicros = currentTaskMicros;
            task = task->getNext();
        }

        // if somehting goes wrong, dump out the whole lot!
        if(!inOrder) dumpTasks();

        // assert that it's in order.
        assertTrue(inOrder);
    }

    void clearCounts() {
        for(int i=0;i<6;i++) counts[i]=0;
        taskManager.reset();
    }
};

testF(HighThroughputFixture, taskManagerHighThroughputTest) {
    char slotData[32];
    clearCounts();

    Serial.print("Dumping threads"); Serial.println(taskManager.checkAvailableSlots(slotData, sizeof slotData));

    taskManager.scheduleFixedRate(10, testCall1);
    taskManager.scheduleFixedRate(100, testCall2);
    taskManager.scheduleFixedRate(250, testCall3, TIME_MICROS);
    taskManager.scheduleOnce(1, testCall4, TIME_SECONDS);
    taskManager.scheduleOnce(10, testCall5, TIME_SECONDS);

    Serial.print("Dumping threads"); Serial.println(taskManager.checkAvailableSlots(slotData, sizeof slotData));

    unsigned long start = millis();
    while(counts[5] < 10 && (millis() - start) < 25000) {
        taskManager.yieldForMicros(10000);
        assertTasksAreInOrder();
    }

    Serial.print("Dumping threads"); Serial.println(taskManager.checkAvailableSlots(slotData, sizeof slotData));

    assertEqual(counts[5], 10); 	// should be 10 runs as it's manually repeating
    assertMore(counts[1], 140);		// should be at least 140 runs, scheduled every 100 millis,
    assertMore(counts[3], 14);		// should be at least 14 runs, as this test lasts about 20 seconds.
    assertMore(counts[0], 1400);	// should be at least 1400 runs it's scheduled every 10 millis
    assertEqual(counts[4], 1); 		// should have been triggered once
    assertNotEqual(counts[2], 0); 	// meaningless to count micros calls. check it happened
}

//
// This test cleans down task manager and then tries to cancel a job within a running task,
// it then waits for the other jobs scheduled after the cancelled to run. See github #38
// Kindly isolated and reported by @martin-klima
//
uint8_t taskId1;
bool taskCancelled;
int storedCount1;
int storedCount2;
testF(HighThroughputFixture, testCancellingsTasksWithinAnotherTask) {
    char slotData[15];

    // set up for the run by clearing down state.
    taskManager.reset();
    counts[0] = counts[1] = counts[2] = 0;
    storedCount1 = storedCount2 = 0;
    taskCancelled = false;

    // register three tasks, the first is to be cancelled.
    taskId1 = taskManager.scheduleFixedRate(100, testCall1);
    taskManager.scheduleFixedRate(100, testCall2);
    taskManager.scheduleFixedRate(100, testCall3);

    // schedule the job to cancel the first registered task.
    taskManager.scheduleOnce(500, [] {
        taskManager.cancelTask(taskId1);
        taskCancelled = true;
        storedCount1 = counts[1];
        storedCount2 = counts[2];
    });

    assertTasksAreInOrder();

    // now run the task manager until the job gets cancelled (or it times out)
    int count = 1000;
    while ((--count != 0) && !taskCancelled) {
        taskManager.yieldForMicros(1000L);
    }

    // the cancelled job should have run at least once before cancellation
    // and then must have been cancelled. Tasks should be in order
    assertNotEqual(0, counts[0]);
    assertTrue(taskCancelled);

    count = 500;
    while (--count != 0) {
        taskManager.yieldForMicros(1000L);
    }

    // once the task manager has been scheduled again, the call counts should not be the same and the tasks
    // should remain in order
    assertTasksAreInOrder();

    // in this case we dump the queue, something is wrong.
    if (counts[1] == storedCount1) {
        dumpTasks();
        Serial.print("Dumping threads"); Serial.println(taskManager.checkAvailableSlots(slotData, sizeof slotData));
    }

    assertNotEqual(counts[1], storedCount1);
    assertNotEqual(counts[2], storedCount1);
}

