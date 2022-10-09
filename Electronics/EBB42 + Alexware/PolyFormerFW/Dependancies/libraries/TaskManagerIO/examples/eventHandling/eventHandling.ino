/**
 * An example simple event that simulates a dice and an interrupt on a pin. When the dice simulated by random reads
 * 3 then we raise an event. This is a polled event meaning that we rely on task manager polling to "roll the dice".
 * Further, there is a basic interrupt on a selected pin that is not marshalled by task manager, instead it raises
 * an event.
 */

#include <TaskManagerIO.h>

#define INT_PIN 14

/**
 * An event that extends BaseEvent allows for event driven programming, either notified by polling, interrupt or
 * another thread. There are two important methods that you need to implement, timeOfNextCheck that allows for polling
 * events, where you do the check in that method and trigger the event calling setTriggered(). Alternatively, like
 * this event, another thread or interrupt can trigger the event, in which case you call markTriggeredAndNotify() which
 * wakes up task manager. When the event is triggered, is exec() method will be called.
 */
class DiceEvent : public BaseEvent {
private:
    volatile int diceValue;
    const int desiredValue;
    static const uint32_t NEXT_CHECK_INTERVAL = 60UL * 1000000UL; // 60 seconds away, maximum is about 1 hour.
public:
    DiceEvent(int desired) : desiredValue(desired) {
        diceValue = 0;
    }

    /**
     * Here we tell task manager when we wish to be checked upon again, to see if we should execute. In polling events
     * we'd do our check here, and mark it as triggered if our condition was met, here instead we just tell task manager
     * not to call us for 60 seconds at a go
     * @return the time to the next check
     */
    uint32_t timeOfNextCheck() override {
        // simulate rolling the dice
        diceValue = (rand() % 7);

        if(diceValue == desiredValue) {
            markTriggeredAndNotify();
        }

        return 250UL * 1000UL; // every 100 milliseconds we roll the dice
    }

    /**
     * This is called when the event is triggered. We just log something here
     */
    void exec() override {
        Serial.print("Dice face matched with ");
        Serial.println(diceValue);
    }

    /**
     * We should always provide a destructor.
     */
    ~DiceEvent() override = default;
} diceEvent(3);

void interruptHandler();

/**
 * Here we show how to use a raw interrupt that is not marshalled by task manager to raise an event. The constructor
 * registers the interrupt, which when hit just triggers the event. The event is then executed in task manager with
 * high priority, IE as the next task.
 */
class InterruptEvent : public BaseEvent {
public:
    /**
     * The constructor just registers a raw interrupt handler, that we must not do any significant
     * work on, in this case our ISR just tells the event it's triggered.
     * @param pin the interrupt pin
     */
    InterruptEvent(pintype_t pin) {
        pinMode(pin, INPUT_PULLUP);
        ::attachInterrupt(digitalPinToInterrupt(pin), interruptHandler, RISING);
    }

    ~InterruptEvent() override = default;

    /**
     * We don't need to poll in this case, just set the poll interval very high
     * @return a large poll interval for interrupt only events
     */
    uint32_t timeOfNextCheck() override {
        return 300 * 1000000; // trigger every 5 minutes. we are not polling here, max gap is 1 hour.
    }

    /**
     * This is called back by task manager as soon after the event is triggered as possible
     */
    void exec() override {
        Serial.println("interrupt event was triggered");
    }
};

InterruptEvent interruptEvent(INT_PIN);

/**
 * THis is a raw ISR, do very little here. In this case we just mark the event as triggered
 */
ISR_ATTR void interruptHandler() {
    interruptEvent.markTriggeredAndNotify();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting the event example");

    // now we register both of the events with task manager.
    taskManager.registerEvent(&diceEvent);
    taskManager.registerEvent(&interruptEvent);
}


//
// This is the regular loop method for a taskManager sketch, just repeatedly calls runLoop. If you want to use
// this library in a low power situation, then you can use a low power library for your board, see the comments
// within loop. If you use a low power delay function, it must be able to come out of sleep for interrupts on the
// pins that you have interrupts, otherwise, taskManager will not work.
//
void loop() {
    // Here's an example of what you can do in low power situations to reduce power usage. You may want to cap the delay
    // at a certain maximum, or sometimes turn off task manager altogether.
    //auto microsToTask = taskManager.microsToNextTask();
    //myLowPowerDelayFunction(microsToTask);

    taskManager.runLoop();
}
