/**
 * This example shows how to use an MCP23017 with interrupt support to control two LEDs
 * by turning a rotary encoder. If the direction is up, the first LED lights, if the
 * direction is down the other lights. When the button is down both LEDs light, and
 * turn off upon release.
 *
 * The interrupt support for these devices works by connecting the interrupt pin of
 * the device to an interrupt capable pin on the device. Further, you must ensure
 * that reset is held in the right state, otherwise instability will result.
 */

#include <IoAbstraction.h>
#include <IoLogging.h>
#include <TaskManagerIO.h>
#include <IoAbstractionWire.h>

// uncomment the line below for compilation on mbed, comment out for Arduino
//#define COMPILE_FOR_MBED

#ifdef COMPILE_FOR_MBED
#include <mbed.h>
BufferedSerial serPort(USBTX, USBRX);
MBedLogger LoggingPort(serPort);
I2C i2c(PF_0, PF_1);
#else
#include <Wire.h>
#endif

//
// we normally try and group input and output on different ports, it is more efficient and
// works better under load.
//

// Daves Test environment II the pins where the encoder is connected
//const int encoderA = 6;
//const int encoderB = 7;
//const int encoderOK = 5;
//const int ledA = 8;
//const int ledB = 9;
//const int attachedInterruptPin = 2;

const int encoderA = 12;
const int encoderB = 14;
const int encoderOK = 13;
const int ledA = 6;
const int ledB = 7;
const int attachedInterruptPin = 26;


// Arduino 23017 interrupt pin connection, and reset pin connection
const int resetPin23017 = 32;

IoAbstractionRef io23017;

//
// this function is called by switches whenever the button is pressed.
//
void onKeyPressed(pinid_t key, bool held) {
    serdebugF3("key pressed", key, held);
    ioDeviceDigitalWrite(io23017, ledA, HIGH);
    ioDeviceDigitalWriteS(io23017, ledB, HIGH);
}

void onKeyReleased(pinid_t key, bool held) {
    serdebugF3("key released", key, held);
    ioDeviceDigitalWrite(io23017, ledA, LOW);
    ioDeviceDigitalWriteS(io23017, ledB, LOW);
}

void onEncoderChange(int encoderValue) {
    serdebugF2("encoder = ", encoderValue);
    ioDeviceDigitalWrite(io23017, ledA, encoderValue < 0);
    ioDeviceDigitalWriteS(io23017, ledB, encoderValue > 0);
}

void setup() {

#ifdef __MBED__
    serPort.set_baud(115200);
    ioaWireBegin(&i2c);
#else
    Wire.begin(4, 15);
    Serial.begin(115200);
#endif

    // this is optional, in a real world system you could probably just connect the
    // reset pin of the device to Vcc, but when prototyping you'll want a reset
    // on every restart.
    auto* deviceIo = internalDigitalIo();
    ioDevicePinMode(deviceIo, resetPin23017, OUTPUT);
    ioDeviceDigitalWriteS(deviceIo, resetPin23017, LOW);
    taskManager.yieldForMicros(100);
    ioDeviceDigitalWriteS(deviceIo, resetPin23017, HIGH);

    io23017 = ioFrom23017(0x20, ACTIVE_LOW_OPEN, attachedInterruptPin);

    serdebugF("Starting LED example on 23017 example");

    ioDevicePinMode(io23017, ledA, OUTPUT);
    ioDevicePinMode(io23017, ledB, OUTPUT);

    // here we initialise switches in interrupt mode, using pull up logic by default.
    switches.init(io23017, SWITCHES_NO_POLLING, true);

    // we now add both a press and release and handler.
    switches.addSwitch(encoderOK, onKeyPressed, 20);
    switches.onRelease(encoderOK, onKeyReleased);

    // and set up an encoder on the same device.
    setupRotaryEncoderWithInterrupt(encoderA, encoderB, onEncoderChange);
}

void loop() {
    // switches needs task manager to run, we must therefore call it every loop and avoid using delays,
    // instead schedule stuff to be done.
    taskManager.runLoop();
}

#ifdef COMPILE_FOR_MBED
int main() {
    setup();
    while(true) {
        loop();
    }
}
#endif
