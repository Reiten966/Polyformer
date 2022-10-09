/**
 * This is another example of using the DFRobot library, this time in conjunction with it's display. For the
 * simplest possible example, see dfRobotAnalogInSwitches example.
 * 
 * It expects that you include a liquid crystal library that you have available. I will assume you are using
 * the default one that's shipped with the IDE.
 * 
 * Shows the value of a rotary encoder on the display based on the UP and DOWN buttons. Select is also handled.
 */

// note you can switch this to include <LiquidCrystal.h> instead, just change the construction of lcd too.
#include <LiquidCrystalIO.h>
#include <IoAbstraction.h>
#include <DfRobotInputAbstraction.h>
#include <TaskManagerIO.h>

// As per the above wiki this uses the default settings for analog ranges.
IoAbstractionRef dfRobotKeys = inputFromDfRobotShield();

// Liquid crystal has an empty constructor for DfRobot.
LiquidCrystal lcd;
bool repaintNeeded = true;

// Here we use a task manager event to manage the screen.
// This event is triggered when the encoder changes, never faster than 250ms.
// If there are changes to be painted it will re-paint the rotary encoder value.
// Never attempt to repaint something to an LCD too frequently, they are very slow.
//
class PaintEvent : public BaseEvent {
public:
    enum PaintButtonState { NOT_PRESSED, PRESSED, BUTTON_HELD};
private:
    PaintButtonState selState;
    int encoderReading;
public:
    uint32_t timeOfNextCheck() override {
        // we are using a polled event, so that we restrict it's execution to about 4 times a second.
        return 250UL * 1000UL;
    }

    void exec() override {
        // print the values onto the screen, something has changed

        // a quick and quite ugly way to zero pad a numeric value to four letters.
        int reading = encoderReading;
        lcd.setCursor(0, 1);
        int divisor = 1000;
        while (divisor > 0) {
            lcd.print(char((reading / divisor) + '0'));
            reading = reading % divisor;
            divisor = divisor / 10;
        }

        // now we print the select button state into right corner.
        const char* btnState = "     ";
        if(selState == PRESSED) btnState = "PRESS";
        else if(selState == BUTTON_HELD) btnState = "HELD ";
        lcd.setCursor(10, 1);
        lcd.print(btnState);
    }

    void currentReading(int reading_) {
        // set the latest reading and trigger the event without notifying, so it waits out the time interval
        encoderReading = reading_;
        setTriggered(true); // we don't want to run the event until the next interval so dont use markAndNotify
    }

    void selectChanged(PaintButtonState state) {
        // set the latest button state and trigger the event without notifying, so it waits out the time interval
        selState = state;
        setTriggered(true); // we don't want to run the event until the next interval so dont use markAndNotify
    }
} paintEvent;

void setup() {
    //
    // set up the display and print our title on the first line
    //
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Rotary encoder:");

    // set up switches to use the DfRobot input facilities
    switches.init(dfRobotKeys, SWITCHES_POLL_EVERYTHING, false);

    // we setup a rotary encoder on the up and down buttons
    setupUpDownButtonEncoder(DF_KEY_UP, DF_KEY_DOWN, [](int reading) {
        paintEvent.currentReading(reading);
    });

    // with a maximum value of 5000, starting at 2500.
    switches.changeEncoderPrecision(5000, 2500);

    // now we add a switch handler for the select button
    switches.addSwitch(DF_KEY_SELECT, [](pintype_t , bool held) {
        paintEvent.selectChanged(held ? PaintEvent::BUTTON_HELD : PaintEvent::PRESSED);
    });

    // and we also want to know when it's released.
    switches.onRelease(DF_KEY_SELECT, [](pintype_t , bool) {
        paintEvent.selectChanged(PaintEvent::NOT_PRESSED);
    });

    // lastly, we set up the event that does the drawing for to a maximum of 4 times a second, when needed.
    taskManager.registerEvent(&paintEvent);
}

void loop() {
    taskManager.runLoop();
}