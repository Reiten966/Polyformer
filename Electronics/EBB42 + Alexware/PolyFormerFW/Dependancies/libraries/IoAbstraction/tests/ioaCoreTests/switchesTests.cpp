#include <AUnit.h>
#include "MockIoAbstraction.h"
#include "SwitchInput.h"

bool pressed;
bool keyReleased;
uint8_t key;
bool held;
int callsMade;
int callsMade2;
int encoderCurrentVal;

using namespace aunit;

// this allows us to call the switches interrupt handler
extern void onSwitchesInterrupt(uint8_t);

void encoderCallback(int newValue) {
    encoderCurrentVal = newValue;
    callsMade++;
}

void encoderCallback2(int newVal) {
    encoderCurrentVal = newVal;
    callsMade2++;
}

void onSwitchPressed(uint8_t k, bool h) {
    callsMade++;
    key = k;
    held = h;
    pressed = true;
}

void onSwitchReleased(uint8_t, bool h) {
    keyReleased = true;
    held = h;
}

class SwitchesFixture : public TestOnce {
protected:
    MockedIoAbstraction mockIo;

public:
    SwitchesFixture() : mockIo(25) { }

    void setup() override {
        mockIo.resetIo();
        taskManager.reset();

        pressed = false;
        keyReleased = false;
        held = false;
        callsMade = 0;
        callsMade2 = 0;
        encoderCurrentVal = 0;
    }

    void teardown() override {
        switches.setEncoder(0, nullptr);
        switches.resetAllSwitches();
        mockIo.resetIo();
        taskManager.reset();
        switches.setEncoder(nullptr);
    }

    void assertPressedState(bool shouldBePressed) {
        // wait until the callback fires or we time out
        int loopCount = 0;
        while(pressed != shouldBePressed && ++loopCount < 200) {
            if(switches.isInterruptDriven()) mockIo.getInterruptFunction()();
            taskManager.yieldForMicros(2000);
        }
        // now check if we are pressed, and if there are any errors.
        assertEqual(pressed, shouldBePressed);
        assertFalse(keyReleased);
        assertEqual(mockIo.getErrorMode(), NO_ERROR);
    }

    void assertHeldState(bool shouldBeHeld) {
        int loopCount = 0;
        while(held != shouldBeHeld && ++loopCount < 500) {
            taskManager.yieldForMicros(1000);
        }

        // now check if we are held and for any errors
        assertEqual(held, shouldBeHeld);
        assertFalse(keyReleased);
        assertEqual(mockIo.getErrorMode(), NO_ERROR);
    }

    void assertReleasedState(bool expectedHeldState) {
        int loopCount = 0;
        while(!keyReleased && ++loopCount < 500) {
            taskManager.yieldForMicros(1000);
        }
        assertTrue(keyReleased);
        assertEqual(held, expectedHeldState);
    }

    void runInterruptLoopTimes(int times) {
        for(int i=0; i<times; i++) {
            mockIo.getInterruptFunction()();
            taskManager.yieldForMicros(10);
        }
    }
};

testF(SwitchesFixture, testPressingASingleButton) {
    switches.initialise(&mockIo, true);
    switches.addSwitch(2, onSwitchPressed, NO_REPEAT);
    switches.onRelease(2, onSwitchReleased);
    assertFalse(switches.isInterruptDriven());

    // simulate the button being pressed (with a bounce).
    mockIo.setValueForReading(0, 0x0000); // pressed
    mockIo.setValueForReading(1, 0x0000); // pressed
    assertFalse(pressed);
    mockIo.setValueForReading(2, 0x0004); // bounce

    // we now simulate the button being pressed down.
    for(int i=3; i<25;i++)  mockIo.setValueForReading(i, 0x0000);
    assertPressedState(true);
    assertFalse(keyReleased);

    auto millisStart = millis();

    // we now simulate the button being held, long press
    mockIo.resetIo();
    for(int i=0; i<25;i++)  mockIo.setValueForReading(i, 0x0000);
    assertHeldState(true);

    // we now simulate the button being released.
    mockIo.resetIo();
    for(int i=0; i<25;i++)  mockIo.setValueForReading(i, 0x0004);
    assertReleasedState(true);

    // make sure getting to held took near to 400 millis.
    assertMoreOrEqual(uint32_t(millis() - millisStart), (uint32_t)380);
    assertLess(uint32_t(millis() - millisStart), (uint32_t)450);
}

testF(SwitchesFixture, testInterruptButtonRepeating) {
    // initialise the switches library using interrupt based initialisation.
    switches.initialiseInterrupt(&mockIo, true);
    switches.addSwitch(2, onSwitchPressed, 10);
    assertTrue(mockIo.isIntRegisteredAs(2, CHANGE));
    assertTrue(switches.isInterruptDriven());

    // simulate the button being pressed (with a double bounce - debounce 2).
    mockIo.setValueForReading(0, 0x0000);
    mockIo.setValueForReading(1, 0x0000);
    mockIo.setValueForReading(2, 0x0004);
    mockIo.setValueForReading(3, 0x0000);
    mockIo.setValueForReading(4, 0x0004);

    // we now simulate the button being pressed down (and the interrupt handler).
    for(int i=4; i<25;i++)  mockIo.setValueForReading(i, 0x0000);
    assertPressedState(true);

    // the first time around, the repeat threshold is 400milli, after that it is 200 millis
    // because we set the repeat interval to 10 of the switch, and the loop delay is 20millis.
    int threshold = 400;

    // make sure we are not already in the held state - we should not be at the moment.
    assertFalse(held);

    // try a few repeat keys..
    for(int i=0; i<4; i++) {
        // capture when we started
        auto timeThen = millis();

        // we now simulate the button being held for a while
        mockIo.resetIo();
        for(int j=0; j<25;j++)  mockIo.setValueForReading(j, 0x0000);
        assertHeldState(true);

        // ensure it is repeating - IE the calls made should be near to the count.
        assertMoreOrEqual(callsMade, i);

        // ensure the timing is close to what we expect.
        assertMoreOrEqual(uint32_t(millis() - timeThen), (uint32_t)threshold  - 50);
        assertLess(uint32_t(millis() - timeThen), (uint32_t)threshold + 50);

        // clear held after each go, the timing of each repeat should be 200.
        held = false;
        threshold = 200;
    }

    // keep track of the number of calls before button release
    int callsWhenButtonReleased = callsMade;

    // now simulate letting go of the button on the IO device.
    mockIo.resetIo();
    for(int j=0; j<25;j++)  mockIo.setValueForReading(j, 0x0004);
    mockIo.getInterruptFunction()();

    // wait long enough for switches to have triggered at least once.
    taskManager.yieldForMicros(32000);

    // we should not have received any more events.
    assertEqual(callsMade, callsWhenButtonReleased);
}


testF(SwitchesFixture, testUpDownEncoder) {
    switches.initialise(&mockIo, true);
    EncoderUpDownButtons upDownEncoder(1, 2, encoderCallback);

    // set up the encoder and add to switches. 1 is up, 2 down.
    switches.setEncoder(0, &upDownEncoder);

    // set the range to be 0 to 10
    switches.changeEncoderPrecision(10, 0);

    // do more than 10 up presses
    switches.pushSwitch(1, false);
    for(int i=0; i<20; i++) switches.pushSwitch(1, true);

    // the encoder should limit at 10 making only 10 calls.
    assertEqual(encoderCurrentVal, 10);

    // now do more than 10 down presses.
    switches.pushSwitch(2, false);
    for(int i=0; i<20; i++) switches.pushSwitch(2, true);

    // again the encoder should limit out at another 10 calls (20 in total)
    // and zero reading.
    assertEqual(encoderCurrentVal, 0);

    // make sure the IO was used correctly
    assertEqual(mockIo.getErrorMode(), NO_ERROR);
}

testF(SwitchesFixture, testChangingCallbacks) {
    switches.initialise(&mockIo, true);
    switches.addSwitch(6, onSwitchPressed, NO_REPEAT);
    switches.pushSwitch(6, false);

    assertEqual(0, callsMade2);
    assertTrue(callsMade != 0);
    auto oldNumberOfCalls = callsMade;

    switches.replaceOnPressed(6, [](pinid_t pin, bool held) {
        callsMade2++;
    });
    switches.pushSwitch(6, false);
    assertTrue(callsMade2 != 0);
    assertEqual(callsMade, oldNumberOfCalls);
    switches.setEncoder(0, nullptr);
}

class MyTestSwitchListener : public SwitchListener {
private:
    int numPressed;
    int numReleased;
public:
    MyTestSwitchListener() : numPressed(0), numReleased(0) {
    }

    void reset() {
        numPressed = numReleased = 0;
    }

    void onPressed(pinid_t pin, bool h) override {
        numPressed++;
    }

    void onReleased(pinid_t pin, bool h) override {
        numReleased++;
    }

    bool wasActivated() { return numPressed !=0 || numReleased != 0; }
} testSwitchListener;

testF(SwitchesFixture, testChangingFromCallbackToListener) {
    testSwitchListener.reset();
    switches.addSwitch(6, onSwitchPressed, NO_REPEAT);
    switches.replaceSwitchListener(6, &testSwitchListener);
    switches.pushSwitch(6, false);
    assertEqual(callsMade, 0);
    assertTrue(testSwitchListener.wasActivated());
}

testF(SwitchesFixture, testChangingFromListenerToCallback) {
    testSwitchListener.reset();
    switches.addSwitchListener(6, &testSwitchListener, NO_REPEAT);
    switches.replaceOnPressed(6, onSwitchPressed);
    switches.pushSwitch(6, false);
    assertEqual(callsMade, 1);
    assertFalse(testSwitchListener.wasActivated());
}

