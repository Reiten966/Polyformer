#include <IoAbstraction.h>
#include <TaskManagerIO.h>

#define READ_CLOCK_PIN 28
#define READ_DATA_PIN 26
#define READ_LATCH_PIN 25

#define WRITE_CLOCK_PIN 22
#define WRITE_DATA_PIN 23
#define WRITE_LATCH_PIN 24

// here we create a shift register abstraction that has 1 input and 1 output shift register.
IoAbstractionRef shiftRegister = inputOutputFromShiftRegister(READ_CLOCK_PIN, READ_DATA_PIN, READ_LATCH_PIN, 1, WRITE_CLOCK_PIN, WRITE_DATA_PIN, WRITE_LATCH_PIN, 1);

// If you are using a 74HC165 for input, you can also use the following instead:
//IoAbstractionRef inputFrom74HC165ShiftRegister(pinid_t readClkPin, pinid_t dataPin, pinid_t latchPin, pinid_t numOfDevices);


void setup() {
	// although not technically needed for the shift register we should always call pinDirection
	// as it makes it possible to switch in future to either arduino direct or IO expander.

	// 0-31 are always input and 32 onwards are always output with the shift register abstraction
	// this allows for up to 4 input and 4 output devices to be chained together.
	for (int i = 32; i < 40; ++i) {
		ioDevicePinMode(shiftRegister, i, OUTPUT);
	}
	for (int i = 0; i < 8; ++i) {
		ioDevicePinMode(shiftRegister, i, INPUT);
	}
}

uint8_t counter = 0;

void loop() {
	delay(1);
	ioDeviceSync(shiftRegister);
	ioDeviceDigitalWrite(shiftRegister, 32, ioDeviceDigitalRead(shiftRegister, 1));
	ioDeviceDigitalWrite(shiftRegister, 33, ioDeviceDigitalRead(shiftRegister, 2));
	ioDeviceDigitalWrite(shiftRegister, 34, ioDeviceDigitalRead(shiftRegister, 3));
	counter++;
	ioDeviceDigitalWrite(shiftRegister, 35, (counter > 128));
}
