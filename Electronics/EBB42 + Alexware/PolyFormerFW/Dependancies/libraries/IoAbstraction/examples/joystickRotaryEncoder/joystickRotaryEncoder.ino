/**
 * This is a simple example of using the Joystick rotary encoder. It assumes you have a potentiometer
 * based analog joystick connected to an analog input port.
 * 
 * Shows the value of a rotary encoder over the serial port. Wiring, wire the joystick as per instructions
 * and take note of the analog input pin you've used. 
 */

#include<TaskManagerIO.h>
#include <IoAbstraction.h>
#include <JoystickSwitchInput.h>

#define ANALOG_INPUT_PIN 25
#define ANALOG_LEFT_RIGHT_PIN 37
#define BUTTON_PIN 2
#define MULTI_IO_ARDUINO_PIN_MAX 100
// we need to create an analog device that the joystick encoder will use to get readings.
// In this case on arduino analog pins.
AnalogDevice* analogDevice;

// We now want to receive button input from both Arduino pins, and two extra simulated button pins that
// are acutally when the joystick moves left and right. So we need a multiIoAbstraction.
MultiIoAbstractionRef multiIo = multiIoExpander(MULTI_IO_ARDUINO_PIN_MAX);

void onEncoderChange(int newValue) {
    Serial.print("New joystick value: ");
    Serial.print(newValue);
    Serial.print(", analog in ");
    Serial.print(analogDevice->getCurrentValue(ANALOG_INPUT_PIN));
    Serial.print(", switch ");
    Serial.println(analogDevice->getCurrentValue(ANALOG_LEFT_RIGHT_PIN));

}

void setup() {
    Serial.begin(115200);

    analogDevice = internalAnalogIo();
    
    // MKR boards require the line below to wait for the serial port, uncomment if needed
    // However, it doesn't work on some other boards and locks them up.
    //while(!Serial);

    Serial.println("Starting joystick rotary encoder example");

    // now we add an expander that handles the left and right function of the joystick as two buttons, these could
    // be useful for next and back functions for example. The mid point for calibration will be half max value in
    // our case, if your potentiometer differs, set the calibration accordingly. Joystick pins will start at 100.
    multiIoAddExpander(multiIo, joystickTwoButtonExpander(analogDevice, ANALOG_LEFT_RIGHT_PIN, .5F), 10);

    // first initialise switches using the multi io expander and pull up switch logic.
    switches.initialise(multiIo, true);

    // now register the joystick
    setupAnalogJoystickEncoder(analogDevice, ANALOG_INPUT_PIN, onEncoderChange);

    // once you've registed the joystick above with switches, you can then alter the mid point and tolerance if needed
    // here we set the midpoint to 65% and the tolerance (or point at which we start reading) at +-5%.
    reinterpret_cast<JoystickSwitchInput*>(switches.getEncoder())->setTolerance(.5F, 0.05F);

    // now set the range to 500 and current value to 250
    switches.changeEncoderPrecision(500, 250);

    // now we set up the left function to operate like a switch, it's on the joystick expander on the multi io
    // so it's addressed with the offset we provided earlier
    switches.addSwitch(MULTI_IO_ARDUINO_PIN_MAX + ANALOG_JOYSTICK_LOWER_PIN, [](pinid_t pin, bool held) {
        Serial.print("Left direction, held="); Serial.println(held);
    }, 20);

    // now we set up the right function to operate like a switch, it's on the joystick expander on the multi io
    // so it's addressed with the offset we provided earlier
    switches.addSwitch(MULTI_IO_ARDUINO_PIN_MAX + ANALOG_JOYSTICK_HIGHER_PIN, [](pinid_t pin, bool held) {
        Serial.print("Right direction, held="); Serial.println(held);
    }, 20);

    // now we set up the joystick click switch, it's on a regular device pin, so it's pin is addressed as usual.
    switches.addSwitch(BUTTON_PIN, [](pinid_t pin, bool held) {
        Serial.print("Joystick Clicked, held="); Serial.println(held);
    });

    // and that's it, task manager and switches does the register
    Serial.println("Started joystick example");
}

void loop() {
    // we must always call this every loop cycle and as frequently as possible
    taskManager.runLoop();
}
