/*
Timed blink, this example shows how to build the simple Blink application using both IoAbstraction
and TaskManager. This gives an example of how quickly a simple application can be made to leverage
this library.

Because this example uses IoAbstraction, the LED could be on the end of an i2c expander or even
on a shift register. Further, if you then needed a second timed action, it would be trivial to
add.

*/

#include <Wire.h>
#include<IoAbstraction.h>
#include<TaskManagerIO.h>

// constant for the pin we will use
const int ledPin = LED_BUILTIN;

// the state of the pin, we will toggle it.
int ledOn = LOW;

// create an IO abstraction, so later we could put the led on a shift register or i2c.
IoAbstractionRef ioDevice = ioUsingArduino();

void setup() {
	// set the pin we are to use as output using the io abstraction
	ioDevicePinMode(ioDevice, ledPin, OUTPUT);

	// and create the task that toggles the led every second.
	taskManager.scheduleFixedRate(1000, toggle);
}

// this is the call back method that gets called once a second
// from the schedule above.
void toggle() {
	// now we write to the device, the 'S' version of the method automatically syncs.
	ioDeviceDigitalWriteS(ioDevice, ledPin, ledOn);

	ledOn = !ledOn; // toggle the LED state.
}


void loop() {
	// this is all we should do in loop when using task manager.
	taskManager.runLoop();
}
