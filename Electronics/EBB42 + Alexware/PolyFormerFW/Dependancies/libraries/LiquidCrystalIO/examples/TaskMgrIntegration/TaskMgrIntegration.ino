/*
 * An example of integrating TaskManager events with LiquidCrystalIO. Showing how the event handling can
 * be used to perform screen refreshes while other events continue to run. We compiled this for Dfrobot shield
 * but could be easily adapted for other combinations.
 */
#include <TaskManagerIO.h>
#include <LiquidCrystalIO.h>

const int outputPin = 11;

// This example initialises for DfRobot, it's the simplest case in LiquidCrystalIO and requires no parameters, because
// we know upfront what the pin configuration is. See other examples for how to provide pin parameters.
LiquidCrystal lcd;

/**
 * here we simulate a slow PWM for the heater, basically the timeOfNextCheck replaces delay in that exec() will
 * be called at interval because we always trigger the event. We never call delay in task manager. this gives us
 * a delay that can be customized each time around.
 */
class HeaterDriver : public BaseEvent {
private:
    int heaterPowerPercent;
    bool isOnNow;
public:
    /**
     * Called by task manager every time the number of microseconds previous returned expires. IE you can use this
     * like a delay, if you trigger in here, then exec() gets called.
     * @return the number of microseconds before calling this function again.
     */
    uint32_t timeOfNextCheck() override {
        // here you'd work out when you should next be called back
        setTriggered(true);
        return millisToMicros((heaterPowerPercent * 100) + 50); // never allow 0 return repeatedly
    }

    /**
     * Called when the event is triggered
     */
    void exec() override {
        digitalWrite(outputPin, isOnNow);
        isOnNow = !isOnNow;
    }

    void setHeaterPower(int percent) {
        heaterPowerPercent = percent;
    }
} heaterDriver;

/**
 * Here we create an event that handles all the drawing for an application, in this case printing out readings
 * of a sensor when changed. It uses polling and immediate triggering to show both examples
 */
class DrawingEvent : public BaseEvent {
private:
    int heaterTemperature;
    bool heaterIsOn;
    volatile bool emergency; // if an event comes from an external interrupt the variable must be volatile.
    bool hasChanged;
public:
    /**
     * This is called by task manager every time the number of microseconds returned expires, if you trigger the
     * event it will run the exec(), if you complete the event, it will be removed from task manager.
     * @return the number of micros before calling again.
     */
    uint32_t timeOfNextCheck() override {
        setTriggered(hasChanged);
        return millisToMicros(500); // no point refreshing more often on an LCD, as its unreadable
    }

    /**
     * This is called when the event is triggered, it prints all the data onto the screen.
     */
    void exec() override {
        hasChanged = false;

        lcd.setCursor(10, 0);
        lcd.print("     ");
        lcd.setCursor(10, 0);
        lcd.print(heaterTemperature);

        lcd.setCursor(10, 1);
        lcd.print(heaterIsOn ? " ON" : "OFF");

        lcd.setCursor(14, 1);
        lcd.print(emergency ? "!!" : "  ");
    }

    /**
     * This sets the latest temperature and heater status, but only marks the event changed, so it will need
     * to poll in order to trigger. This prevents excessive screen updates.
     * @param temp the new temperature
     * @param on if the heater is on
     */
    void setLatestStatus(int temp, bool on) {
        heaterTemperature = temp;
        heaterIsOn = on;
        hasChanged = true;// we are happy to wait out the 500 millis
    }

    /**
     * Triggers an emergency that requires immediate update of the screen
     * @param isEmergency if there is an urgent notification
     */
    void triggerEmergency(bool isEmergency) {
        emergency = isEmergency;
        markTriggeredAndNotify(); // get on screen asap.
    }
};

// create an instance of the above class
DrawingEvent drawingEvent;

void setup() {
    lcd.begin(16, 2);
    lcd.setCursor(0,0);
    lcd.print("Heater:");
    lcd.setCursor(0,1);
    lcd.print("Temp:");

    pinMode(outputPin, OUTPUT);

    // we create a watchdog task that simulates reading a sensor state and updating the drawing event.
    taskManager.scheduleFixedRate(50, [] {
        // this simulates a reading from your system that needs to be displayed, we run this in it's own task
        int temp = (rand() % 200) - 100;
        bool isOn = rand() % 2;
        drawingEvent.setLatestStatus(temp, isOn);
    }, TIME_MILLIS);

    // here we create a couple of tasks that represent triggering and clearing an emergency.
    taskManager.scheduleOnce(10, [] {
        drawingEvent.triggerEmergency(true);
    }, TIME_SECONDS);

    taskManager.scheduleOnce(30, [] {
        drawingEvent.triggerEmergency(false);
    }, TIME_SECONDS);

    heaterDriver.setHeaterPower(10);

    taskManager.scheduleFixedRate(10, [] {
        heaterDriver.setHeaterPower(rand() % 100);
    }, TIME_SECONDS);

    // create any other tasks that you need here for your sketch

    taskManager.registerEvent(&drawingEvent);
    taskManager.registerEvent(&heaterDriver);
}

void loop() {
    taskManager.runLoop();
}