/**
 * This example shows how to use task manager specifically with mbed. The API is exactly the same
 * between Arduino and mbed, so you can also look at the Arduino examples for inspiration too.
 *
 * This example shows advanced usage of taskManager covering scheduled execution, events, putting
 * events on the queue from anote
 */

#include <mbed.h>
#include <TaskManagerIO.h>
#include <TmLongSchedule.h>

#define LOG_TASK_MANGER_DEBUG 0

// Here we create a serial object to write log statements to.
BufferedSerial console(USBTX, USBRX, 115200);

// and we also store the taskId of the one second task, to remove it later.
taskid_t oneSecondTask;

Thread threadEventMgr;
Thread anotherThread;

void log(const char* toLog, int i = 0) {
    char sz[20];
    itoa(millis(), sz, 10);
    console.write(sz, strlen(sz));
    console.write(" ", 1);
    console.write(toLog, strlen(toLog));

    if(i!=0) {
        itoa(i, sz, 10);
        console.write(sz, strlen(sz));
    }

    console.write("\n", 1);
}

// An executable task is a task that where the exec method  is called when the task is due for execution.
// We don't want to log on every run of the microsecond task, it would overwhelm serial so just count instead.
// this class holds a number of ticks and bumps that count on every execute.
class MicrosecondTask : public Executable {
private:
    int ticks;
public:
    MicrosecondTask(int startingTicks = 0) {
        ticks = startingTicks;
    }

    /**
     * This is called by task manager when the task is ready to run.
     */
    void exec() override {
        ticks++;
    }

    /**
     * @return the number of ticks currently counted
     */
    int getCurrentTicks() const {
        return ticks;
    }
};

// here we store a reference to the microsecond task.
MicrosecondTask* microsTask;

// An event that extends BaseEvent allows for event driven programming, either notified by polling, interrupt or
// another thread. There are two important methods that you need to implement, timeOfNextCheck that allows for polling
// events, where you do the check in that method and trigger the event calling setTriggered(). Alternatively, like
// this event, another thread or interrupt can trigger the event, in which case you call markTriggeredAndNotify() which
// wakes up task manager. When the event is triggered, is exec() method will be called.
class DiceEvent : public BaseEvent {
private:
    volatile int diceValue;
    const int desiredValue;
    static const int NEXT_CHECK_INTERVAL = 60 * 1000000; // 60 seconds away, maximum is about 1 hour.
public:
    DiceEvent(int desired) : desiredValue(desired) {
        diceValue = 0;
    }

    /**
     * Here we tell task manager when we wish to be checked upon again, to see if we should execute. In polling events
     * we'd do our check here, and setTriggered(true) if our condition was met, here instead we just tell task manager
     * not to call us for 60 seconds at a go
     * @return the time to the next check
     */
    uint32_t timeOfNextCheck() override {
        return NEXT_CHECK_INTERVAL;
    }

    /**
     * This is called when the event is triggered. We just log something here
     */
    void exec() override {
        log("Dice face matched with ", diceValue);
    }

    /**
     * Called by the other thread when the dice is updated, it checks if the event is now triggered as a result
     * and then triggers the event. When triggering from another thread or interrupt, use markTriggeredAndNotify()
     * @param faceValue the new dice value
     */
    void diceUpdated(int faceValue) {
        diceValue = faceValue;
        if(faceValue == desiredValue) {
            markTriggeredAndNotify();
        }
    }

    /**
     * We should always provide a destructor.
     */
    ~DiceEvent() override = default;
} diceEvent(3);

// A job submitted to taskManager can either be a function that returns void and takes no parameters, or a class
// that extends Executable. In this case the job creates a repeating task and logs to the console.
void tenSecondJob() {
    log("30 seconds up, restart a new repeating job");
    taskManager.scheduleFixedRate(500, [] {
        log("Half second job, micros = ", microsTask->getCurrentTicks());
    });
}

// Again another task manager function, we pass this as the timerFn argument later on
void twentySecondJob() {
    log("20 seconds up, delete 1 second job, schedule 10 second job");
    taskManager.scheduleOnce(10, tenSecondJob, TIME_SECONDS);
    taskManager.cancelTask(oneSecondTask);
}

//
// Set up all the initial tasks and events
//
void setupTasks() {
    // Here we create a new task using milliseconds; which is the default time unit for scheduling. We use a lambda
    // function as the means of scheduling.
    oneSecondTask = taskManager.scheduleFixedRate(1000, [] {
        log("One second job, micro count = ", microsTask->getCurrentTicks());
    });

    // Here we create a new task based on the twentySecondJob function, that will be called at the appropriate time
    // We schedule this with a unit of seconds.
    taskManager.scheduleOnce(20, twentySecondJob, TIME_SECONDS);

    // here we create a new task based on Executable and pass it to taskManager for scheduling. We provide the
    // time unit as microseconds, and with the last parameter we tell task manager to delete it when the task
    // is done, IE for one shot tasks that is as soon as it's done, for repeating tasks when it's cancelled.
    microsTask = new MicrosecondTask(0);
    taskManager.scheduleFixedRate(100, microsTask, TIME_MICROS, true);

    // here we create an event that will be triggered on another thread and then notify task manager when it is
    // triggered. We will allocate using new and let task manager delete it when done.
    taskManager.registerEvent(&diceEvent);

    int capturedValue = 42;
    taskManager.scheduleFixedRate(2, [capturedValue]() {
        log("Execution with captured value = ", capturedValue);
    }, TIME_SECONDS);

    // this shows how to create a long schedule event using the new operator, make sure the second parameter is true
    // as this will delete the event when it completes.
    taskManager.registerEvent(new TmLongSchedule(makeHourSchedule(0, 15), [] {
        log("Fifteen minutes passed");
    }), true);
}

bool exitThreads = false;

//
// Here we create another thread that will constantly put events onto the queue for immediate execution, in addition
// it also updates our dice event frequently too.
//
void taskPump() {
    while(!exitThreads) {
        ThisThread::sleep_for(500);
        taskManager.execute([]() {
            log("execute immediately from thread proc");
        });

        diceEvent.diceUpdated(rand() % 7);
    }
}

//
// Yet another thread that constantly raises events to test thread safety of task manager.
//
void anotherProc() {
    while(!exitThreads) {
        ThisThread::sleep_for(600);
        taskManager.execute([]() {
            char slotData[64];
            log(taskManager.checkAvailableSlots(slotData, sizeof(slotData)));
        });
    }
}

int main() {
    log("starting up taskmanager example");

#if LOG_TASK_MANGER_DEBUG != 0
    // this is how we get diagnostic information from task manager
    // it will notify of significant events to the loggingDelegate.
    tm_internal::setLoggingDelegate([](tm_internal::TmErrorCode code, int task) {
        log("Taskmgr notification code: ", code);
        log("   -> Task num: ", task);
    });
#endif //LOG_TASK_MANGER_DEBUG

    setupTasks();

    threadEventMgr.start(taskPump);
    anotherThread.start(anotherProc);

    while(1) {
        taskManager.runLoop();
    }
}
