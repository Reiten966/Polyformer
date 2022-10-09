/**
 * An example that shows how to create long schedules, in hours and days with task manager. Any schedule over an hour
 * in length can be created using a TmLongSchedule instance, these instances are registered as events with task manager
 * and other than this work as usual.
 *
 * You cannot create schedules over 60 minutes using the scheduleFixedRate / scheduleOnce functions, instead you use
 * the procedure below.
 */

#include <TaskManagerIO.h>
#include <TmLongSchedule.h>
#include <Wire.h>

//
// Here we create an OO task (a task that is actually a class instance). In this case the exec() method will be
// called at the specified time.
//
class MyTaskExecutable : public Executable {
private:
    int callCount;
    bool enableTask = false;
    taskid_t taskToSuspend = TASKMGR_INVALIDID;
public:
    void setTaskToSuspend(taskid_t id) { taskToSuspend = id; }

    void exec() override {
        callCount++;
        Serial.print("Called my task executable ");
        Serial.println(callCount);

        taskManager.setTaskEnabled(taskToSuspend, enableTask);
        enableTask = !enableTask;
    }
} myTaskExec;

// Forward references
void dailyScheduleFn();

//
// Here we create two schedules to be registered with task manager later. One will fire every days and one will fire
// every hour and a half. You provide the schedule in milliseconds (generally using the two helper functions shown) and
// also either a timer function or class extending executable.
//
TmLongSchedule hourAndHalfSchedule(makeHourSchedule(1, 30), &myTaskExec);
TmLongSchedule onceADaySchedule(makeDaySchedule(1), dailyScheduleFn);

void setup() {
    Serial.begin(115200);

    Serial.println("Started long schedule example");

    // First two long schedules are global variables.
    // IMPORTANT NOTE: If you use references to a variable like this THEY MUST BE GLOBAL
    taskManager.registerEvent(&hourAndHalfSchedule);
    taskManager.registerEvent(&onceADaySchedule);

    // this shows how to create a long schedule event using the new operator, make sure the second parameter is true
    // as this will delete the event when it completes.
    taskManager.registerEvent(new TmLongSchedule(makeHourSchedule(0, 15), [] {
        Serial.print(millis());
        Serial.println(": Fifteen minutes passed");
    }), true);

    // lastly we show the regular event creation method, this task is enabled and disabled by the OO task.
    auto taskId = taskManager.scheduleFixedRate(120, [] {
        Serial.print(millis());
        Serial.println(": Two minutes");
    }, TIME_SECONDS);

    myTaskExec.setTaskToSuspend(taskId);
}

void loop() {
    taskManager.runLoop();
}

void dailyScheduleFn() {
    Serial.print(millis());
    Serial.println(": Daily schedule");
}