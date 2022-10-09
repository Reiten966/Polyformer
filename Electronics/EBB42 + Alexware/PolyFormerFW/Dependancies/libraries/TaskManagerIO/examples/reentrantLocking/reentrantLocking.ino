
#include <TaskManagerIO.h>
#include <SimpleSpinLock.h>

// this is the global lock that that we'll use to protect the variable below myVar.
SimpleSpinLock myLock;
volatile int myVar = 0;
int localVarCopy = 0;

//
// A simple log function for writing to serial
//
void log(const char* str, int val) {
    Serial.print(millis());
    Serial.print(' ');
    Serial.print(str);
    Serial.print('=');
    Serial.println(val);
}

//
// here we define a nested function that is called from the task, it locks again. This test that the reentrant
// functionality is working
//
void nestedFunction() {
    // here we take a nested lock, which is allowed by our lock, which is essentially reference counted for up to
    // 255 levels of depth.
    TaskMgrLock locker(myLock);
    log("in nested function, lock count", myLock.getLockCount());
}

void stateMachineLockingAsync();

void setup() {
    Serial.begin(115200);

    // start a task that locks the bus, calls a nested function and yields time back to task manager.
    taskManager.scheduleFixedRate(1000, stateMachineLockingAsync);


    taskManager.scheduleFixedRate(250, [] {
        log("Running inner task, before lock ", myLock.getLockCount());

        // take the lock
        TaskMgrLock locker(myLock);

        log("locked synchronously, count", myLock.getLockCount());

        nestedFunction();

        // do something that's unlikely to be optimised out, to waste some time
        for(int i=0; i < 100; i++) {
            myVar++;
        }

        log("Synchronous task done", myVar);

        // lock released at end of function
    });
}

void loop() {
    taskManager.runLoop();
}

//
// This function shows how to use the lock asynchronously, in code where the lock being taken and unlocked are not
// sequential actions. Great care is needed with this approach to avoid deadlocks. It is far easier to get the regular
// TaskMgrLock based locking right.
//
void stateMachineLockingAsync() {
    log("Before try lock, count", myLock.getLockCount());
    if(myLock.tryLock()) {
        // do a check to ensure nothing else has run.
        if(localVarCopy != myVar) {
            log("ERROR - another task got into the lock", myVar - localVarCopy);
        }

        log("We already had the async lock, now unlock, count", myLock.getLockCount());
        myLock.unlock();
    }
    else {
        // we didn't have the lock, try and acquire with a spin wait, that times out if it takes too long.
        log("We need to do async lock", myLock.getLockCount());

        bool gotLock = myLock.spinLock(millisToMicros(10));
        if(gotLock) {
            localVarCopy = myVar;
            log("  - got the lock", myLock.getLockCount());
        }
        else {
            log("  - spin wait failed", myLock.getLockCount());
        }
    }
}
