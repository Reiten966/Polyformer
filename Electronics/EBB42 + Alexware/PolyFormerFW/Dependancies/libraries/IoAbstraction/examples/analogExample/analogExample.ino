/**
 * This example demonstrates the analog abstraction that can use the same interface to
 * work with various analog devices such as DAC, ADC, PWM and POTs wherever they are
 * located. As more devices are added it will expand the possibilities of where This
 * common interface can be used.
 */

#include <IoAbstraction.h>
#include <AnalogDeviceAbstraction.h>
#include <TaskManagerIO.h>
#include <DeviceEvents.h>
#include <Wire.h>

// This is the output pin, where analog output will be sent.
// on SAMD MKR boards this is the DAC, for Uno, MEGA change to a PWM pin
#define PWM_OR_DAC_PIN 15

// This is the input pin where analog input is received.
#define ANALOG_IN_PIN A0

// Here we create an analog event that will be triggered when the the analog level exceeds 75%. it is triggered every
// 100 milliseconds and whwn triggered runs the code in the exec() method.
class MyAnalogExceedsEvent : public AnalogInEvent {
public:
    MyAnalogExceedsEvent(AnalogDevice* device, pinid_t pin) :
            AnalogInEvent(device, pin, 0.75, AnalogInEvent::ANALOGIN_EXCEEDS, 100000UL) {
    }

    void exec() override {
        Serial.print("Trigger AnalogInEvent Threshold ");
        Serial.print(analogThreshold);
        Serial.print(", value ");
        Serial.println(lastReading);
    }
};

// We keep a variable that counts the output waveform
float ledCycleValue = 0;
// the current direction of adjustment
float ledCycleAdj = 0.01;

AnalogDevice* analog = internalAnalogIo();

void setup() {
    Serial.begin(115200);

    // set up the device pin directions upfront.
    analog->initPin(ANALOG_IN_PIN, DIR_IN);
    analog->initPin(PWM_OR_DAC_PIN, DIR_OUT);

    // this is how to register an event with task manager
    taskManager.registerEvent(new MyAnalogExceedsEvent(internalAnalogIo(), ANALOG_IN_PIN), true);

    // we schedule a task to run every 500 millis that reads the value from A1 and prints it output
    // along with the largest possible value
    taskManager.scheduleFixedRate(500, [] {
        Serial.print("Analog input value is ");
        Serial.print(analog->getCurrentValue(ANALOG_IN_PIN));
        Serial.print("/");
        Serial.print(analog->getMaximumRange(DIR_IN, ANALOG_IN_PIN));
        Serial.print(" - ");
        Serial.print(analog->getCurrentFloat(ANALOG_IN_PIN) * 100.0F);
        Serial.println('%');

#ifdef ESP32
        auto* espAnalog = reinterpret_cast<ESP32AnalogDevice*>(analog);
        // On ESP32 boards, where the analogWrite function doesn't exist we use the underlying functions
        // to access either the DAC or LEDC subsystem, if you want to get hold of the ledc channel you can.
        EspAnalogOutputMode* outputMode = espAnalog->getEspOutputMode(PWM_OR_DAC_PIN);
        if(outputMode != nullptr) {
            Serial.print("ESP32 Output type: ");
            Serial.print(outputMode->isDac());
            Serial.print(", ledc (pwm channel): ");
            Serial.println(outputMode->getPwmChannel());
        }

        EspAnalogInputMode* inputMode = espAnalog->getEspInputMode(ANALOG_IN_PIN);
        if(inputMode != nullptr) {
            Serial.print("ESP32 Input on dac1: ");
            Serial.print(inputMode->isOnDAC1());
            Serial.print(", channel: ");
            Serial.println(inputMode->getChannel());
        }
#endif
    });

    // we also create a sawtooth waveform on one of the outputs. By default we are using the DAC
    // on A0 of most MKR boards. Change to PWM for AVR boards.
    taskManager.scheduleFixedRate(10, [] {
        ledCycleValue += ledCycleAdj;
        if(ledCycleValue >= 0.98) ledCycleAdj = -0.01;
        if(ledCycleValue <= 0.02) ledCycleAdj = 0.01;

        analog->setCurrentFloat(PWM_OR_DAC_PIN, ledCycleValue);
    }, TIME_MILLIS);
}

// and lastly the standard loop for use with task manager, basically does nothing but
// repeatedly call runLoop.
void loop() {
    taskManager.runLoop();
}