
#ifndef TASKMANGERIO_TEST_UTILS_H
#define TASKMANGERIO_TEST_UTILS_H

#include <AUnit.h>
#include <IoLogging.h>

void dumpTasks();

#define MILLIS_ALLOWANCE 2000
#define MICROS_ALLOWANCE 200

extern bool scheduled;
extern bool scheduled2ndJob;
extern unsigned long microsStarted;
extern unsigned long microsExecuted;
extern unsigned long microsExecuted2ndJob;
extern int count;
extern int count2;
extern uint8_t pinNo;

using namespace aunit;

class EnsureExecutionWithin {
private:
    unsigned long startMillis;
    unsigned long maximumTime;
public:
    explicit EnsureExecutionWithin(unsigned long maxWaiting) {
        startMillis = millis();
        maximumTime = maxWaiting;
    }
    bool ensureTimely() {
        if(millis() > (startMillis + maximumTime)) {
            serdebugF2("TEST EXECUTION OUT OF RANGE ", int(millis() - startMillis));
            return false;
        }

        serdebugF2("Test execution within range: ", int(millis() - startMillis));
        return true;
    }
};

class TimingHelpFixture : public TestOnce {
protected:
    void setup() override {
        taskManager.reset();
        scheduled = scheduled2ndJob = false;
        microsExecuted = microsExecuted2ndJob = 0;
        microsStarted = micros();
        count = count2 = 0;
    }

    void assertThatTaskRunsOnTime(uint32_t minExpected, uint32_t allowanceOver) {
        unsigned long startTime = millis();

        // wait until the task is marked as scheduled.
        while(!scheduled && (millis() - startTime) < 5000) {
            taskManager.yieldForMicros(10000);
        }

        // we must have called the job.
        assertTrue(scheduled);

        // print information to help with diagnostics.
        Serial.print("Scheduled: "); Serial.print(microsStarted);
        Serial.print(" ExecAt: "); Serial.print(microsExecuted);
        unsigned long howLong = microsExecuted - microsStarted;
        Serial.print(" difference "); Serial.print(howLong);
        Serial.print(" - expected "); Serial.println(minExpected);

        // check that it has been executed within the allowable window.
        unsigned long minRange = (minExpected < allowanceOver) ? 0 : (minExpected - allowanceOver);
        unsigned long maxRange = minExpected + allowanceOver;
        assertMoreOrEqual((microsExecuted - microsStarted), minRange);
        assertLess((microsExecuted - microsStarted), maxRange);
    }

    void assertThatSecondJobRan(unsigned long minExpected, unsigned long allowanceOver) {
        // it is assumed that the first job has been waited for and that this should
        // have been run first (ie the shorter task)

        // print information to help with diagnostics.
        Serial.print("Scheduled: "); Serial.print(microsStarted);
        Serial.print(" ExecAt: "); Serial.print(microsExecuted2ndJob);
        long howLong = microsExecuted2ndJob - microsStarted;
        Serial.print(" difference "); Serial.print(howLong);
        Serial.print(" - expected "); Serial.println(minExpected);

        assertTrue(scheduled2ndJob);
        unsigned long minRange = (minExpected < allowanceOver) ? 0 : (minExpected - allowanceOver);
        unsigned long maxRange = minExpected + allowanceOver;
        assertMoreOrEqual((microsExecuted2ndJob - microsStarted), minRange);
        assertLess((microsExecuted2ndJob - microsStarted), maxRange);
    }

    void assertTasksSpacesTaken(int taken) {
        char sz[32];
        int count = 0;
        char* taskData = taskManager.checkAvailableSlots(sz, sizeof sz);
        while(*taskData) {
            if(*taskData != 'F') count++;
            ++taskData;
        }
        Serial.print("Tasks free "); Serial.println(count);
        assertEqual(taken, count);
    }
};


#endif //TASKMANGERIO_TEST_UTILS_H
