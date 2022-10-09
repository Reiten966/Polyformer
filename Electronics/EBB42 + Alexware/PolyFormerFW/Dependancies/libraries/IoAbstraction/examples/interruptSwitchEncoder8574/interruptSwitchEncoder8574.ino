/*
 This sketch is an example of using either one or two rotary encoders along with two switches and an LED over an
 i2c 8574 IO expander.

 One switch is the encoders push switch, configured as non-repeating. The other switch is repeating and will toggle
 the LED. In this example the switches library is set up in interrupt mode, which means unless something is pressed
 down, there is no polling.

 Switch input is designed to work with the task manager class which makes scheduling tasks trivial.

 Circuit / detail: https://www.thecoderscorner.com/products/arduino-libraries/io-abstraction/rotary-encoder-switches-interrupt-pcf8574/

 2020-11-29: We've added the possibility of an additional encoder connected to the PCF8574, to give you two encoders
             on the same device. Each section of code for the 2nd encoder is wrapped with comments, to make it both
             easy to identify what you do to add another encoder, and secondly, to make it easy to comment out.
             We've also added an example of how to register a listener style callback for a key-press.

*/

// We have a direct dependency on Wire and Arduino ships it as a library for every board
// therefore to ensure compilation we include it here.
#include <Wire.h>

#include<IoAbstraction.h>
#include<IoAbstractionWire.h>
#include <TaskManagerIO.h>

// The pin onto which we connected the rotary encoders switch
const int spinWheelClickPin = 5;

// The pin onto which we connected the repeat button switch
const int repeatButtonPin = 4;

// the led pin on the IO ioDevice
const int ledPin = 1;

// The two pins where we connected the A and B pins of the encoder. I recommend you dont change these
// as the pin must support interrupts.
const int encoderAPin = 6;
const int encoderBPin = 7;

// the maximum value (starting from 0) that we want the encoder to represent. This range is inclusive.
const int maximumEncoderValue = 128;

// used when we toggle the LED state further down.
bool currentLedState;

// Start 2nd Encoder
// Here are the fields for the second rotary encoder
// If you only want one encoder you can comment these fields out.
const int encoder2Click = 0;
const int encoder2APin = 2;
const int encoder2BPin = 3;
HardwareRotaryEncoder *secondEncoder;
// End 2nd Encoder

//
// When the spinwheel is clicked and released, the following two functions will be run
//
void onSpinWheelClicked(uint8_t /*pin*/, bool heldDown) {
    Serial.print("Encoder button pressed ");
    Serial.println(heldDown ? "Held" : "Pressed");
}

void onSpinWheelButtonReleased(uint8_t /*pin*/, bool heldDown) {
    Serial.print("Encoder released - previously ");
    Serial.println(heldDown ? "Held" : "Pressed");
}

//
// When the repeat button is pressed, this listener class is notified. Every press and repeat is sent to
// onPressed while the release action is sent to onReleased. This method allows you to keep state between
// calls, and also to integrate the listener into an existing class if needed.
//
class RepeatButtonListener : public SwitchListener {
private:
    int counter;
public:
    void onPressed(pinid_t pin, bool held) override {
        ++counter;
        Serial.print("Repeat button pressed ");
        Serial.println(counter);
        ioDeviceDigitalWriteS(switches.getIoAbstraction(), ledPin, counter & 0x01);
    }

    void onReleased(pinid_t pin, bool held) override {
        Serial.print("Released after presses ");
        Serial.println(counter);
        counter = 0;
    }
} repeatListener;

//
// Each time the encoder value changes, this function runs, as we registered it as a callback
//
void onEncoderChange(int newValue) {
    Serial.print("Encoder change ");
    Serial.println(newValue);
}

void setup() {
    Serial.println("Starting interrupt switch PCF8574 example now");

    // Before doing anything else, we must initialise the wire and serial libraries, as we are using both.
    Serial.begin(115200);
    while(!Serial);
    Wire.begin();

    // First we set up the switches library, giving it the task manager and tell it where the pins are located
    // We could also of chosen IO through an i2c device that supports interrupts.
    // the second parameter is a flag to use pull up switching, (true is pull up).
    switches.initialiseInterrupt(ioFrom8574(0x20, 0), true);

    ioDevicePinMode(switches.getIoAbstraction(), ledPin, OUTPUT);

    // now we add the switches, we dont want the spin wheel button to repeat, so leave off the last parameter
    // which is the repeat interval (millis / 20 basically) Repeat button does repeat as we can see.
    switches.addSwitch(spinWheelClickPin, onSpinWheelClicked);
    switches.onRelease(spinWheelClickPin, onSpinWheelButtonReleased);
    switches.addSwitchListener(repeatButtonPin, &repeatListener, 25);

    // now we set up the rotary encoder, first we give the A pin and the B pin.
    // we give the encoder a max value of 128, always minimum of 0.
    setupRotaryEncoderWithInterrupt(encoderAPin, encoderBPin, onEncoderChange);
    switches.changeEncoderPrecision(0, maximumEncoderValue, 100, true);

    // Start 2nd encoder
    Serial.println("Setting up second encoder now");

    // here we add a second encoder, you can comment out the below lines if you only want to use one encoder
    secondEncoder = new HardwareRotaryEncoder(encoder2APin, encoder2BPin, [](int direction) {
        Serial.print("Encoder direction: ");
        Serial.println(direction);
    });
    secondEncoder->changePrecision(0, 0); // this means record direction changes only
    switches.setEncoder(1, secondEncoder); // put it into the 2nd available encoder slot.

    // Now add the 2nd encoders button for release notification only, you can use onRelease without first calling
    // addSwitch(..), and it will default everything, but you cannot invert logic this way.
    switches.onRelease(encoder2Click, [](pinid_t pin, bool held) {
        Serial.println("encoder 2 released");
    });
    // End 2nd encoder
}

void loop() {
    taskManager.runLoop();
}
