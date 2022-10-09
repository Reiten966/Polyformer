
#include <AUnit.h>
#include <ExecWithParameter.h>
#include "TaskManagerIO.h"
#include "test_utils.h"

using namespace aunit;

bool taskWithinEvent;

class TestPolledEvent : public BaseEvent {
private:
    int execCalls;
    int scheduleCalls;
    uint32_t interval;
    bool triggerNow;
public:
    TestPolledEvent() {
        execCalls = scheduleCalls = 0;
        interval = 100000; // 100 millis
        triggerNow = false;
        taskWithinEvent = false;
    }

    ~TestPolledEvent() override = default;

    void exec() override {
        execCalls++;
        taskManager.scheduleOnce(100, [] {
            taskWithinEvent = true;
        });
        setCompleted(true);
    }

    uint32_t timeOfNextCheck() override {
        scheduleCalls++;
        setTriggered(triggerNow);
        return interval;
    }

    void startTriggering() {
        triggerNow = true;
        interval = 10000;
    }

    int getScheduleCalls() const { return scheduleCalls; }

    int getExecCalls() const { return execCalls; }
} polledEvent;

typedef bool (*TMPredicate)();

bool runScheduleUntilMatchOrTimeout(TMPredicate predicate) {
    unsigned long startTime = millis();
    // wait until the predicate matches, or it takes too long.
    while (!predicate() && (millis() - startTime) < 1000) {
        taskManager.yieldForMicros(10000);
    }
    return predicate();
}

testF(TimingHelpFixture, testRaiseEventStartTaskCompleted) {
    EnsureExecutionWithin timelyChecker(600);

    // first register the event
    taskManager.registerEvent(&polledEvent);

    // then we
    assertTrue(runScheduleUntilMatchOrTimeout([] { return polledEvent.getScheduleCalls() >= 3; } ));

    // and now we tell the event to trigger itself
    polledEvent.startTriggering();

    // wait until the exec() method is called
    assertTrue(runScheduleUntilMatchOrTimeout([] { return polledEvent.getExecCalls() != 0; }));

    // and then make sure that the task registed inside the event triggers
    assertTrue(runScheduleUntilMatchOrTimeout([] { return taskWithinEvent; }));

    assertTrue(timelyChecker.ensureTimely());
}

class TestExternalEvent : public BaseEvent {
private:
    int execCalls;
    bool nextCheckCalled = false;
public:
    TestExternalEvent() {
        execCalls = 0;
        taskWithinEvent = false;
    }

    ~TestExternalEvent() override = default;

    void exec() override {
        execCalls++;
        markTriggeredAndNotify();
        taskManager.yieldForMicros(500);
        setTriggered(false);
        taskManager.execute([] {
            taskWithinEvent = true;
        });
    }

    uint32_t timeOfNextCheck() override {
        nextCheckCalled = true;
        return 100000000UL;
    }

    void resetStats() {
        nextCheckCalled = false;
        execCalls = 0;
    }
    bool wasNextCheckCalled() const { return nextCheckCalled; }
    int getExecCalls() const { return execCalls; }
} externalEvent;

testF(TimingHelpFixture, testNotifyEventThatStartsAnotherTask) {
    EnsureExecutionWithin timelyChecker(100);
    auto taskId = taskManager.registerEvent(&externalEvent);

    for(int i=0; i<100; i++) {
        taskWithinEvent = false;
        externalEvent.markTriggeredAndNotify();
        taskManager.yieldForMicros(100);
        assertTrue(runScheduleUntilMatchOrTimeout([] { return taskWithinEvent; }));
        assertEqual(i + 1, externalEvent.getExecCalls());
    }

    // now we let the task complete and after one more cycle it should be removed by task manager.
    externalEvent.setCompleted(true);
    externalEvent.markTriggeredAndNotify();
    taskManager.yieldForMicros(100);

    //now it should be completely removed, and whatever we do should not affect task manager
    externalEvent.resetStats();
    externalEvent.markTriggeredAndNotify();
    taskManager.yieldForMicros(200);
    assertFalse(externalEvent.wasNextCheckCalled());
    assertEqual(0, externalEvent.getExecCalls());

    // it should not be in task manager any longer.
    assertFalse(taskManager.getTask(taskId)->isEvent());

    assertTrue(timelyChecker.ensureTimely());
}
