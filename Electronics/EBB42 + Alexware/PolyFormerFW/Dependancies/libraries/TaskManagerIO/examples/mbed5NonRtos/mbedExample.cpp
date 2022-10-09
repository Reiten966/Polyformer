//
// This is an example of using mbed 5 without RTOS support
// It demonstrates raising a few tasks for scheduling and an interrupt based event.
// WARNING - I actually have not got a non RTOS board big enough to run this.
//

#include <TaskManagerIO.h>

bool taskRunning = true;
taskid_t millisJob;

Serial serPort(USBTX, USBRX);

void log(const char* toLog) {
    char sz[14];
    itoa(millis(), sz, 10);
    serPort.puts(sz);
    serPort.puts(toLog);
    serPort.putc('\n');
}

void setup() {
    serPort.baud(115200);
    taskManager.scheduleFixedRate(1, [] {
        log("One second job");
    }, TIME_SECONDS);

    millisJob = taskManager.scheduleFixedRate(100, [] {
        log("1/10th sec");
    });

    taskManager.scheduleFixedRate(10, [] {
        log("Ten seconds up, remove millisecond job");
        taskManager.cancelTask(millisJob);
    }, TIME_SECONDS);
}

int main() {
   setup();

   while(taskRunning) {
       taskManager.runLoop();
   }
}
