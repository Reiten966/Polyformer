/**
 * This example shows how to use the matrix keyboard support that's built into IoAbstraction,
 * it can be used out the box with either a 3x4 or 4x4 keypad, but you can modify it to use
 * any matrix keyboard quite easily.
 * It just sends the characters that are typed on the keyboard to Serial. The keyboard in This
 * example is connected directly to Arduino pins, but could just as easily be connected over
 * a PCF8574, MCP23017 or other IoAbstraction. For interrupt mode, you cannot use a PCF8574
 * because the interrupt on the device would be triggered by the output changes when scanning.
 * Only MCP23017 and device pins can be used in interrupt mode.
 */

#include <Wire.h>
#include <IoAbstraction.h>
#include <IoAbstractionWire.h>
#include<TaskManagerIO.h>
#include <KeyboardManager.h>

//
// We need to make a keyboard layout that the manager can use. choose one of the below.
// The parameter in brackets is the variable name.
//
//MAKE_KEYBOARD_LAYOUT_3X4(keyLayout)
MAKE_KEYBOARD_LAYOUT_4X4(keyLayout)

//
// We need a keyboard manager class too
//
MatrixKeyboardManager keyboard;

// this examples connects the pins directly to an arduino but you could use
// IoExpanders or shift registers instead.
IoAbstractionRef arduinoIo = ioUsingArduino();

//
// We need a class that extends from KeyboardListener. this gets notified when
// there are changes in the keyboard state.
//
class MyKeyboardListener : public KeyboardListener {
public:
    void keyPressed(char key, bool held) override {
        Serial.print("Key ");
        Serial.print(key);
        Serial.print(" is pressed, held = ");
        Serial.println(held);
    }

    void keyReleased(char key) override {
        Serial.print("Released ");
        Serial.println(key);
    }
} myListener;

/**
 * In this method we initialise the keyboard for I2C operation, we assume a 4x4 keyboard was enabled at the top of
 * the sketch, and as we are using an MCP23017, we can run in interrupt mode to avoid polling the I2C bus. Note that
 * you cannot enable interrupt mode on PCF8574.
 */
void initialiseKeyboard4X4ForInterrupt23017() {
    Wire.begin();

    keyLayout.setRowPin(0, 11);
    keyLayout.setRowPin(1, 10);
    keyLayout.setRowPin(2, 9);
    keyLayout.setRowPin(3, 8);
    keyLayout.setColPin(0, 15);
    keyLayout.setColPin(1, 14);
    keyLayout.setColPin(2, 13);
    keyLayout.setColPin(3, 12);

    keyboard.initialise(ioFrom23017(0x20, ACTIVE_LOW_OPEN, 10), &keyLayout, &myListener, true);
}

/**
 * In this method we initialise the keyboard to use the arduino pins directly. We assume a 4x3 keyboard was set at the
 * top. We use the keyboard in polling mode in this case. Polling mode can be used on any device.
 */
void initialiseKeyboard3X4ForPollingDevicePins() {
    keyLayout.setRowPin(0, 22);
    keyLayout.setRowPin(1, 23);
    keyLayout.setRowPin(2, 24);
    keyLayout.setRowPin(3, 25);
    keyLayout.setColPin(0, 26);
    keyLayout.setColPin(1, 27);
    keyLayout.setColPin(2, 28);

    // create the keyboard mapped to arduino pins and with the layout chosen above.
    // it will callback our listener
    keyboard.initialise(arduinoIo, &keyLayout, &myListener);
}

void setup() {
    while(!Serial);
    Serial.begin(115200);

    tm_internal::setLoggingDelegate([](tm_internal::TmErrorCode errorCode, int task) {
        serdebugF3("TMLog ", errorCode, task);
    });

    // here you can choose between two stock configurations or you could alter one of the
    // methods to meet your hardware requirements.
    initialiseKeyboard4X4ForInterrupt23017();
    //initialiseKeyboard3X4ForPollingDevicePins();

    // now set up the repeat key start and interval
    keyboard.setRepeatKeyMillis(850, 350);

    Serial.println("Keyboard is initialised!");
}

void loop() {
    // as this indirectly uses taskmanager, we must include this in loop.
    taskManager.runLoop();
}
