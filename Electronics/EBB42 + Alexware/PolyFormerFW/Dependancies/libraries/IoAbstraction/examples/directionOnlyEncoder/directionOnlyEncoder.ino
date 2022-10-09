/*
 This sketch gives an example of using a rotary encoder for directional use only, it also captures
 the push-button that's on the rotary encoder.
 
 Switch input is designed to work with the task manager class which
 makes scheduling tasks trivial.

 Circuit / detail: https://www.thecoderscorner.com/products/arduino-downloads/io-abstraction/arduino-switches-handled-as-events/

*/

#include<IoAbstraction.h>
#include <TaskManagerIO.h>

// The pin onto which we connected the rotary encoders switch
const int spinwheelClickPin = 4;

// The two pins where we connected the A and B pins of the encoder. I recomend you dont change these
// as the pin must support interrupts.
const int encoderAPin = 5;
const int encoderBPin = 6;

//
// When the spinwheel is clicked, this function will be run as we registered it as a callback
//
void onSpinwheelClicked(pinid_t pin, bool heldDown) {
  Serial.print("Button pressed ");
  Serial.println(heldDown ? "Held" : "Pressed");
}

//
// Each time the encoder value changes, this function runs, in this case as we are in directional
// mode it will only register -1 (down) or 1 (up).
//
void onEncoderChange(int newValue) {
  Serial.print("Encoder change ");
  if(newValue > 0) {
      Serial.println("up");
  }
  else if(newValue < 0) {
      Serial.println("down");
  }
}

void setup() {

  Serial.begin(115200);

  // First we set up the switches library, giving it the task manager and tell it to use arduino pins
  // We could also of chosen IO through an i2c device that supports interrupts.
  // If you want to use PULL DOWN instead of PULL UP logic, change the true to false below.
  switches.initialise(ioUsingArduino(), true);

  // now we add the switches, we dont want the spinwheel button to repeat, so leave off the last parameter
  // which is the repeat interval (millis / 20 basically) Repeat button does repeat as we can see.
  switches.addSwitch(spinwheelClickPin, onSpinwheelClicked);

  // now we set up the rotary encoder, first we give the A pin and the B pin.
  // we only want directional indications so we set the precision to max 0, current 0.
  // when we do this, the callback either gets 0 no change, -1 down or 1 up.
  setupRotaryEncoderWithInterrupt(encoderAPin, encoderBPin, onEncoderChange);
  switches.changeEncoderPrecision(0, 0);
}

void loop() {
  taskManager.runLoop();  
}
