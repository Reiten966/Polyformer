
#include <AUnit.h>
#include <ExecWithParameter.h>
#include "TaskManagerIO.h"
#include "test_utils.h"

using namespace aunit;

void dumpTasks() {
    Serial.println("Dumping the task queue contents");
    TimerTask* task = taskManager.getFirstTask();
    while(task) {
        Serial.print(" - Task schedule "); Serial.print(task->microsFromNow());
        Serial.print(task->isRepeating() ? " Repeating ":" Once ");
        Serial.print(task->isMicrosSchedule() ? " Micros " : " Millis ");
        Serial.println(task->isInUse() ? " InUse":" Free");
        if(task->getNext() == task) {
            Serial.println("!!!Infinite loop found!!!");
        }
        task = task->getNext();
    }
}

// these variables are set during test runs to time and verify tasks are run.
bool scheduled = false;
bool scheduled2ndJob = false;
unsigned long microsStarted = 0, microsExecuted = 0, microsExecuted2ndJob = 0;
int count = 0, count2 = 0;
uint8_t pinNo = 0;

void recordingJob() {
    microsExecuted = micros();
    count++;
    scheduled = true;
}

void recordingJob2() {
    microsExecuted2ndJob = micros();
    count2++;
    scheduled2ndJob = true;
}


class TestingExec : public Executable {
public:
    int noOfTimesRun;

    TestingExec() {
        noOfTimesRun = 0;
    }

    void exec() override {
        noOfTimesRun++;
    }
};

TestingExec exec;

testF(TimingHelpFixture, testRunningUsingExecutorClass) {
    taskManager.scheduleFixedRate(10, &exec);
    taskManager.scheduleOnce(250, recordingJob);
    assertThatTaskRunsOnTime(250000L, MILLIS_ALLOWANCE);
    assertMore(exec.noOfTimesRun, 10);
}

testF(TimingHelpFixture, schedulingTaskOnceInMicroseconds) {
    taskManager.scheduleOnce(800, recordingJob, TIME_MICROS);
    assertThatTaskRunsOnTime(800, MICROS_ALLOWANCE);
    assertTasksSpacesTaken(0);
}

testF(TimingHelpFixture, schedulingTaskOnceInMilliseconds) {
    taskManager.scheduleOnce(20, recordingJob, TIME_MILLIS);
    assertThatTaskRunsOnTime(19500, MILLIS_ALLOWANCE);
    assertTasksSpacesTaken(0);
}

testF(TimingHelpFixture, schedulingTaskOnceInSeconds) {
    taskManager.scheduleOnce(2, recordingJob, TIME_SECONDS);
    // second scheduling is not as granular, we need to allow +- 100mS.
    assertThatTaskRunsOnTime(2000000L, MILLIS_ALLOWANCE);
    assertTasksSpacesTaken(0);
}

testF(TimingHelpFixture, scheduleManyJobsAtOnce) {
    taskManager.scheduleOnce(1, [] {}, TIME_SECONDS);
    taskManager.scheduleOnce(200, recordingJob, TIME_MILLIS);
    taskManager.scheduleOnce(250, recordingJob2, TIME_MICROS);

    assertThatTaskRunsOnTime(199500, MILLIS_ALLOWANCE);
    assertThatSecondJobRan(250, MICROS_ALLOWANCE);
    assertTasksSpacesTaken(1);
}

testF(TimingHelpFixture, enableAndDisableSupport) {
    static int myTaskCounter = 0;
    auto myTaskId = taskManager.scheduleFixedRate(1, [] { myTaskCounter++; }, TIME_MILLIS);
    taskManager.yieldForMicros(20000);
    assertNotEqual(0, myTaskCounter);

    // "turn off" the task
    taskManager.setTaskEnabled(myTaskId, false);

    // it can take one cycle for the task to switch enablement state.
    taskManager.yieldForMicros(2000);
    auto oldTaskCount = myTaskCounter;

    // now run the task for some time, it should never get scheduled.
    taskManager.yieldForMicros(20000);
    assertEqual(myTaskCounter, oldTaskCount);

    // "turn on" the task and see if it increases again
    taskManager.setTaskEnabled(myTaskId, true);
    taskManager.yieldForMicros(20000);
    assertNotEqual(myTaskCounter, oldTaskCount);

}

testF(TimingHelpFixture, scheduleFixedRateTestCase) {
    assertEqual(taskManager.getFirstTask(), NULL);

    auto taskId1 = taskManager.scheduleFixedRate(10, recordingJob, TIME_MILLIS);
    auto taskId2 = taskManager.scheduleFixedRate(100, recordingJob2, TIME_MICROS);

    // now check the task registration in detail.
    assertNotEqual(taskId1, TASKMGR_INVALIDID);
    TimerTask* task = taskManager.getFirstTask();
    assertNotEqual(task, NULL);
    assertFalse(task->isMillisSchedule());
    assertTrue(task->isMicrosSchedule());

    // now check the task registration in detail.
    assertNotEqual(taskId2, TASKMGR_INVALIDID);
    task = task->getNext();
    assertNotEqual(task, NULL);
    assertTrue(task->isMillisSchedule());
    assertFalse(task->isMicrosSchedule());

    dumpTasks();

    uint32_t timeStartYield = millis();
    taskManager.yieldForMicros(secondsToMillis(22));
    uint32_t timeTaken = millis() - timeStartYield;

    dumpTasks();

    // make sure the yield timings were in range.
    assertLess(timeTaken, (uint32_t) 25);
    assertMoreOrEqual(timeTaken, (uint32_t) 20);

    // now make sure that we got in the right ball park of calls.
    assertMore(count, 1);
    assertMore(count2, 150);
}

testF(TimingHelpFixture, cancellingAJobAfterCreation) {
    assertEqual(taskManager.getFirstTask(), NULL);

    auto taskId = taskManager.scheduleFixedRate(10, recordingJob, TIME_MILLIS);

    // now check the task registration in detail.
    assertNotEqual(taskId, TASKMGR_INVALIDID);
    TimerTask* task = taskManager.getFirstTask();
    assertNotEqual(task, NULL);
    assertTrue(task->isMillisSchedule());
    assertFalse(task->isMicrosSchedule());
    assertMore(task->microsFromNow(), 8000UL);

    assertThatTaskRunsOnTime(10000, MILLIS_ALLOWANCE);

    // cancel the task and make sure everything is cleared down
    assertTasksSpacesTaken(1);
    taskManager.cancelTask(taskId);
    taskManager.yieldForMicros(100); // needs to run the cancellation task.
    assertTasksSpacesTaken(0);

    assertEqual(taskManager.getFirstTask(), NULL);
}
