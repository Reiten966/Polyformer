## TaskManagerIO scheduling and event based library for Arudino and mbed

Dave Cherry / TheCodersCorner.com make this library available for you to use. It takes me significant effort to keep all my libraries current and working on a wide range of boards. Please consider making at least a one off donation via the sponsor button if you find it useful. In forks, please keep text to here intact.

TaskManagerIO is an evolution of the task management class that was originally situated in IoAbstraction. It is backed by a simple queue that supports, immediate queuing, scheduled tasks, and events. It is safe to add tasks from another thread, and safe to trigger events from interrupts. However, your tasks are shielded from threads and interrupts making your code simpler.

We are in a new era of embedded development, where RTOS, multiple threads (and even cores) have become a relatity. Any viable task manager needs to be capable in these environments, while still protecting tasks from multithreaded concerns. We are pleased to say, this version meets both goals. Importantly, any sketch that worked on IoAbstraction task manager will work with this library unaffected. 

Below, we list the main features of TaskManagerIO:

* Simple coroutines style task management, execute now, at a point in time, or on a schedule.
* Your tasks do not need to be thread or interrupt safe, they will only be called from task manager.
* Ability to add events that can be triggered from different threads or interrupts, for either delayed or ASAP execution. Again, always called on the task manager thread.
* Polled event based programming where you set a schedule to be asked if your event is ready to fire.
* Marshalled interrupt support, where task manager handles the raw interrupt ISR, and then calls your interrupt task.

## Getting started with taskManager

Here we just demonstrate the most basic usage. Take a look at the examples for more complex cases, or the reference documentation that's built from the source.

Include the header file:

```
#include <TaskManagerIO.h>
```

In the setup method, add an function callback that gets fired once in the future:

```
	// Create a task scheduled once every 100 miliis
	taskid_t taskId = taskManager.scheduleOnce(100, [] {
		// some work to be done.
	});
	
	// Create a task that's scheduled every second
	taskid_t taskId = taskManager.scheduleFixedRate(1, [] {
		// work to be done.
	}, TIME_SECONDS);
```

From 1.2 onwards: On ESP8266, ESP32, all mbed boards, and most 32 bit Arduino boards you can also *enable* argument capture in lambda expressions. By default, the feature is off because it is quite a heavy feature that many may never use.

To enable add the following flag to your compile options, and it will be enabled if the board supports it: `-DTM_ENABLE_CAPTURED_LAMBDAS`

An example of this usage follows:

```
    int capturedValue = 42;
    taskManager.scheduleFixedRate(2, [capturedValue]() {
        log("Execution with captured value = ", capturedValue);
    }, TIME_SECONDS);

```

You can also create a class that extends from `Executable` and schedule that instead. For example:

```
    // Create the class
    class MyClassToSchedule : public Executable {
        //... your other stuff

        void exec() override {
            // your code to be executed upon schedule.
        }
    };
    
    // Create an instance
    MyClassToSchedule myClass;
    
    // Register with taskManager for once a second execution
    taskManager.scheduleFixedRate(1, &myClass, TIME_SECONDS);
```

Then in the loop method you need to call: 

```
  void loop() {
  	 taskManager.runLoop();
  }
```

To schedule tasks that have an interval of longer than an hour, use the long schedule support as follows (more details in the longSchedule example):

First create a long schedule either globally or using the new operator:

    TmLongSchedule hourAndHalfSchedule(makeHourSchedule(1, 30), &myTaskExec);

Then add it to task manager during setup or as needed:

    taskManager.registerEvent(&hourAndHalfSchedule);
    
After this the callback (or event object) registered in the TmLongSchedule will be called whenever scheduled. 

To enable or disable a task

	taskManager.setTaskEnabled(taskId, enabled);

If you have a shared resource that you need to lock around, you can do this in tasks. See the reentrantLocking example for more details.

Arduino Only - If you want to use the legacy interrupt marshalling support instead of building an event you must additionally include the following:

	#include <BasicInterruptAbstraction.h>


## Further documentation and getting help

* [TaskManagerIO documentation pages](https://www.thecoderscorner.com/products/arduino-libraries/taskmanager-io/)
* [TaskManagerIO reference documentation](https://www.thecoderscorner.com/ref-docs/taskmanagerio/html)

There is a forum where questions can be asked, but the rules of engagement are: **this is my hobby, I make it available because it helps others**. Don't expect immediate answers, make sure you've recreated the problem in a simple sketch that you can send to me. Please consider making at least a one time donation using the sponsor link above before using the forum.

* [TCC Libraries community discussion forum](https://www.thecoderscorner.com/jforum/)
* I also monitor the Arduino forum [https://forum.arduino.cc/], Arduino related questions can be asked there too.

### Known working and supported boards:

| CPU / OS  | Boards using CPU  | Status    | Threading  |
| --------- | ----------------- | --------- | ---------- |
| ARM mbed  | STM32, nRF.       | Supported | CAS locking|
| ESP8266   | Node MCU, Huzzah  | Supported | Interrupt  |
| ESP32     | Wifi32, Huzzah32  | Supported | CAS locking|
| SAMD ARM  | MKR, IoT, Zero.   | Supported | Interrupt  |
| AVR       | Uno, Mega Mighty  | Supported | Interrupt  |
| nRF52840  | Nano BLE          | Supported | CAS locking|
| Particle  | Photon            | Supported | Interrupt  |

Note: if you are using a bare-metal mbed build (non-RTOS) on platformIO, for the moment please add an extra build flag: `PIO_NEEDS_RTOS_WORKAROUND` as a short term fix while a long term solution is determined. See issue [#17](https://github.com/davetcc/TaskManagerIO/issues/17).

Many thanks to contributors for helping us to confirm that this software runs on a wide range of hardware.

### Threading key:

* CAS locking: Protected against access even by multiple cores by using CAS task locking
* Interrupt: Single core device that is protected by an atomic noInterrupt block

## What is TaskManagerIO?

TaskManagerIO library is not a full RTOS, rather it can be used on top of FreeRTOS via ESP32 or mbed RTOS. It is a complimentary technology that can assist with certain types of work-load. It has a major advantage, that the same code runs on many platforms as listed above. It is a core building block of [IoAbstraction](https://github.com/davetcc/IoAbstraction) and [tcMenu framework](https://github.com/davetcc/IoAbstraction)

## Important notes around scheduling tasks and events

TaskManagerIO is a cooperative scheduler, and cooperative schedulers by their very nature have unfair semantics. In practice this means that you should not create repeating events or fixed rate schedules that have a 0 delay, if you do, no other tasks will run because the one with 0 delay will always win.  

## Multi-tasking - advanced usage

TaskManager uses a lock free design, based on "compare and exchange" to acheive thread safety on larger boards, atomic operations on AVR, and interrupt locking back-up on other boards. Below, we discuss the multi-threaded features in more detail.

* On any board, it is safe to add tasks and raise events from any thread. We use whatever atomic operations are available for that board to ensure safety.
* On any board, you can start another thread and run a task manager on it. Only ever call task-manager's runLoop() from the same thread.
* On ESP32 FreeRTOS and mbed RTOS6 it is safe to add tasks to a taskManager from another core, on these platforms task manager uses the processors compare and exchange functionality to ensure thread safety as much as possible.

## Helping out

We are always glad to accept bug fixes and features. However, please always raise an issue first, and for significant work, it's worth waiting for us to reply first. Please see the contributing guide.
