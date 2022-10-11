/*
TaskManagement Example for BasicIoAbstraction library

TaskManager is designed to replace the standard delay() then do something pattern in Arduino code.
When using this library, we express waits differently, more in line with in 10 seconds turn
on the led. To do this we use a scheduler that controls when tasks are done. For each task
that we schedule, we provide a function that will be called upon that schedule. This example
shows all the uses of the scheduler, for a simpler example, see the timedBlink example.

To test the interrupt support, wire a switch to pin 2 with pull up/down. Each change of state
will cause an interrupt.

Written by Dave Cherry of TheCodersCorner.com in 2017
*/

#include <Arduino.h>
#include <TaskManagerIO.h>
#include <BasicInterruptAbstraction.h>

// here we define an interrupt capable pin that will be varied in value during execution causing the
// task manager interrupt handler to be executed. Task manager will marshal the interrupt back into 
// a task, so it's safe to call anything you wish during it's execution.
const int interruptPin = 2;

// When we are not using IoAbstraction with task manager, then if we want to use interrupts, this class
// provides the absolute bare minimum interrupt abstraction. Normally, we'd use IoAbstraction's device
// abstraction capabilities instead.
BasicArduinoInterruptAbstraction interruptAbstractionOnly;

/**
 * We'll use this function to print out the milliseconds from start and also the
 * line to write to Serial
 */
void log(const char* logLine) {
	Serial.print(millis());
	Serial.print(": ");
	Serial.println(logLine);
}

/**
 * This is called by taskManager when the interrupt is raised. TaskManager marshals the
 * interrupt into a task, so it is safe to call Serial etc here. Be aware that interrupts
 * handled by taskManager are not completely real time, so only use when some slight delay
 * can be accepted. 
 * 
 * - Safe usage: change in rotary encoder, button pressed.
 * - Unsafe usage: over temperature shutdown, safety circuit.
 */
void onInterrupt(pintype_t pin) {
	log("Interrupt triggered");
	Serial.print("  Interrupt was ");
	Serial.println(pin);
}

// count up the number of times the micros job has been called.
int microCount = 0;

/**
 * Called by taskManager - this is the microsecond job registered in setup.
 * We just count up the number of calls and log it every 100 calls.
 */
void onMicrosJob() {
	microCount++;
	if (abs(microCount % 10000) == 1) {
		log("Micros job increased by 10000");
	}
}

/**
 * Called by taskManager - this is the one second job registered in setup
 */
void oneSecondPulse() {
	log("One second pulse");
}

/**
 * This is the job that was started in the tenSecondsUp above.
 */
void twentySecondsUp() {
	log("Twenty seconds up");
}

/**
 * Called by taskManager when ten seconds is up, the job is registered in setup.
 * It starts another job in 10 seconds time, the 20 second job.
 */
void tenSecondsUp() {
    log("Ten seconds up");
    if(taskManager.scheduleOnce(10000, twentySecondsUp) == TASKMGR_INVALIDID) {
        log("Failed to register twenty second task");
    }
}

void setup() {
    // start up serial, the first line is for 32 bit boards and may require commenting out on some devices.
    Serial.begin(115200);

    Serial.print("Task manager example is starting. Block size = ");
    Serial.print(DEFAULT_TASK_SIZE);
    Serial.print(", blocks = ");
    Serial.println(DEFAULT_TASK_BLOCKS);

    // if you want to receive notifications from task manager, provide a loggingDelegate as below.
    tm_internal::setLoggingDelegate([] (tm_internal::TmErrorCode code, int id) {
        Serial.print("TM Notification code=");
        Serial.print(code);
        Serial.print(", id=");
        Serial.println(id);
    });

    // connect a switch to interruptPin, so you can raise interrupts.
    pinMode(interruptPin, INPUT);

    //
    // Now we register some tasks, note that on AVR by default there are 6 slots, all others have 10 slots.
    // this can be changed in TaskManager.h to your preferred setting.
    //

    // We schedule the function tenSecondsUp() to be called in 10,000 milliseconds.
    taskManager.scheduleOnce(10000, tenSecondsUp);

    // Now we schedule oneSecondPulse() to be called every second.
    // keep hold of the ID as we will later cancel it from running.
    taskid_t taskId = taskManager.scheduleFixedRate(1, oneSecondPulse, TIME_SECONDS);

    //
    // now we do a yield operation, which is similar to delayMicroseconds but allows other
    // tasks to be run during that time.
    //
    log("Waiting 32 milli second with yield in setup");
    taskManager.yieldForMicros(32000);
    log("Waited 32 milli second with yield in setup");

#ifdef TM_ALLOW_CAPTURED_LAMBDA
    // now schedule a task to run once in 30 seconds, we capture the taskId using a locally captured value. Notice that
    // this only works on 32 bit boards such as ESP*, ARM, mbed etc.
    taskManager.scheduleOnce(30000, [taskId]() {
        log("30 seconds up, stopping 1 second job");

        // now cancel the one second job we scheduled earlier
        taskManager.cancelTask(taskId);
    });
#endif

    // and another to run repeatedly at 5 second intervals, shows the task slot status
    taskManager.scheduleFixedRate(5, [] {
        char slotString[32];
        log(taskManager.checkAvailableSlots(slotString, sizeof(slotString)));
    }, TIME_SECONDS);

    // and now schedule onMicrosJob() to be called every 100 micros
    taskManager.scheduleFixedRate(100, onMicrosJob, TIME_MICROS);

    // register a port 2 interrupt.
    taskManager.setInterruptCallback(onInterrupt);
    taskManager.addInterrupt(&interruptAbstractionOnly, interruptPin, CHANGE);
}

/**
 * The loop method must contain the taskManager.runLoop() call, it
 * checks all the schedules and interrupt states and runs any tasks
 * that are due.
 */
void loop() {
	taskManager.runLoop();
}
