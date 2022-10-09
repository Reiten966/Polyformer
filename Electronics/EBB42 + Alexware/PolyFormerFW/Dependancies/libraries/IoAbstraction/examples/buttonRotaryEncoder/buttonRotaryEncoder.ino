/*
 This sketch gives an example of using a rotary encoder along with its inbuilt push button with
 the switch input facilities. In addition to the encoder, we also connect another switch that
 will be used to 
 
 Switch input is designed to work with the task manager class which
 makes scheduling tasks trivial.

 Circuit / detail: https://www.thecoderscorner.com/products/arduino-downloads/io-abstraction/arduino-switches-handled-as-events/

*/

#include<IoAbstraction.h>
#include <TaskManagerIO.h>

// The pin onto which we connected the rotary encoders switch
const pinid_t spinwheelClickPin = 0;

// The pin onto which we connected the repeat button switch
const pinid_t repeatButtonPin = PC9;

// The two pins where we connected the A and B pins of the encoder, the A pin must support interrupts.
const pinid_t encoderAPin = PC8;
const pinid_t encoderBPin = PC10;

// the maximum (0 based) value that we want the encoder to represent.
const int maximumEncoderValue = 128;

// an LED that flashes as the encoder changes
const int ledOutputPin = LED_BLUE;

// You can change the step rate of the encoder, it defaults to 1, but can be changed during a precision change
const int stepSize = 2;

// You can set the encoder to wrap around at min/max values, or just to stop there.
const bool wrapAround = true;

auto boardIo = internalDigitalIo();

//
// When the encoder button is clicked, this function will be run as we registered it as a callback
//
void onSpinwheelClicked(pinid_t pin, bool heldDown) {
    Serial.print("Button pressed ");
    Serial.print(pin);
    Serial.println(heldDown ? " Held" : " Pressed");
}

//
// When the repeat button is pressed, this function will be repeatedly called. It's also a callback
//
void onRepeatButtonClicked(pinid_t /*pin*/, bool /*heldDown*/) {
    Serial.println("Repeat button pressed");
}

//
// Each time the encoder value changes, this function runs, as we registered it as a callback
//
void onEncoderChange(int newValue) {
    Serial.print("Encoder change ");
    Serial.println(newValue);

    // here we turn the LED on and off as the encoder moves.
    ioDeviceDigitalWriteS(boardIo, ledOutputPin, newValue % 2);
}

void setup() {

    Serial.begin(115200);
    Serial.println("Starting rotary encoder example");

    // We want to make the onboard LED flash, so set the pin to be output
    ioDevicePinMode(boardIo, ledOutputPin, OUTPUT);

    // our next task is to initialise swtiches, do this BEFORE doing anything else with switches.
    // We choose to initialise in poll everything (requires no interrupts), but there are other modes too:
    // (SWITCHES_NO_POLLING - interrupt only) or (SWITCHES_POLL_KEYS_ONLY - encoders on interrupt)
    switches.init(boardIo, SWITCHES_POLL_KEYS_ONLY, true);

    // now we add the switches, we don't want the spin-wheel button to repeat, so leave off the last parameter
    // which is the repeat interval (millis / 20 basically) Repeat button does repeat as we can see.
    switches.addSwitch(spinwheelClickPin, onSpinwheelClicked, NO_REPEAT);
    switches.addSwitch(repeatButtonPin, onRepeatButtonClicked, 25);

    // now we set up the rotary encoder, first we give the A pin and the B pin.
    // we give the encoder a max value of 128, always minimum of 0.
    auto hwEncoder = new HardwareRotaryEncoder(encoderAPin, encoderBPin, onEncoderChange);
    switches.setEncoder(0, hwEncoder);
    hwEncoder->changePrecision(maximumEncoderValue, 100, wrapAround, stepSize);
}

void loop() {
    taskManager.runLoop();
}
