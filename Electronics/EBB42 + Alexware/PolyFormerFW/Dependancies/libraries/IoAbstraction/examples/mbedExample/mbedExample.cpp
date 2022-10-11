/**
 * mbedExample.cpp
 *
 * Presents an example of how to use the various parts of IoAbstraction with the mbed framework.
 */

#include "mbed.h"
#include "rtos.h"
#include <IoLogging.h>
#include <TaskManagerIO.h>
#include <SwitchInput.h>
#include <JoystickSwitchInput.h>
#include <AnalogDeviceAbstraction.h>
#include <EepromAbstractionWire.h>

int myCount = 0;
float currentOutputLevel = 0.0F;

// to be able to use IoLogging within your application add the following
BufferedSerial serPort(USBTX, USBRX);
MBedLogger LoggingPort(serPort);

#define DEVICE_PIN_RANGE 150
MultiIoAbstractionRef multiIo = multiIoExpander(DEVICE_PIN_RANGE);
MBedAnalogDevice analogDevice;

I2C i2c(PF_0, PF_1);
#define START_OFFS 3000

// if you want to run the eeprom tests, uncomment below
#define HAS_EEPROM_ATTACHED

volatile bool exitApp = false;

void buttonPressed(pinid_t pin, bool held) {
    serdebugF3("Button pressed: ", pin, held)
}

void buttonReleased(pinid_t pin, bool held) {
    serdebugF3("Button released: ", pin, held)
}

void doSomeLogging() {// IoAbstraction includes a very basic logging framework, here we write a few debug statements out
// See IoLogging.h for usage and options. It should NEVER be left ENABLED in production code.
// When not enabled it is compiled out of your code.
    serdebugF2("Debug double ", 15.943);
    serdebugFHex("Debug hex1 ", 255);
    serdebugFHex2("Debug hex2 ", 255, 0xaa);
    serdebugF4("Debug mixed: ", 255, "something", true);
}

class ExecutableTask : public Executable {
private:
    int someSharedState;
public:
    ExecutableTask(int someSharedState) : someSharedState(someSharedState) {}

    void exec() override {
        serdebugF2("exec() shared state: ", someSharedState);
    }
};

void checkTheEeprom();

ExecutableTask myTask(100);

void scheduleSomeTasks() {
    ioDevicePinMode(multiIo, LED3, OUTPUT);

    taskManager.scheduleFixedRate(1, [] {
        serdebugF2("Second counter: ", myCount);
        ioDeviceDigitalWriteS(multiIo, LED3, (myCount % 2) == 0);
    }, TIME_SECONDS);

    taskManager.scheduleFixedRate(100, [] {
        myCount++;
    }, TIME_MILLIS);

    taskManager.scheduleOnce(5, [] {
        taskManager.scheduleOnce(20, &myTask, TIME_SECONDS);
    }, TIME_SECONDS);
}

void runSomeAnalogTasks() {
    analogDevice.initPin(LED1, DIR_PWM);
    analogDevice.initPin(LED2, DIR_PWM);
    taskManager.scheduleFixedRate(1, [] {
        currentOutputLevel += 0.001F;
        if(currentOutputLevel > 1.0F) currentOutputLevel = 0.0F;
        analogDevice.setCurrentFloat(LED2, 1.0F - currentOutputLevel);
        analogDevice.setCurrentFloat(LED1, currentOutputLevel);
    });
}

int main() {
    serPort.set_baud(115200);

#ifdef HAS_EEPROM_ATTACHED
    checkTheEeprom();
#endif

    doSomeLogging();

    scheduleSomeTasks();

    multiIoAddExpander(multiIo, joystickTwoButtonExpander(&analogDevice, A1, .5F), 10);

    switches.initialise(multiIo, false);
    switches.addSwitch(USER_BUTTON, buttonPressed, 20);
    switches.onRelease(USER_BUTTON, buttonReleased);
    switches.addSwitch(D8, buttonPressed, NO_REPEAT, true);
    switches.addSwitch(DEVICE_PIN_RANGE + ANALOG_JOYSTICK_LOWER_PIN, [](pinid_t, bool held) {
        serdebugF2("Left on joystick", held);
    });
    switches.addSwitch(DEVICE_PIN_RANGE + ANALOG_JOYSTICK_HIGHER_PIN, [](pinid_t, bool held) {
        serdebugF2("Right on joystick", held);
    });

    setupAnalogJoystickEncoder(&analogDevice, A0, [](int value) {
        serdebugF2("JoystickValue: ", value);
    });
    switches.getEncoder()->changePrecision(250, 125);

    runSomeAnalogTasks();

    while(!exitApp) {
        taskManager.runLoop();
    }
    return 0;
}

const char stringSource[] = "This is a really long string that we need to write into the rom and read back";
void checkTheEeprom() {
    I2cAt24Eeprom rom(0xA0, PAGESIZE_AT24C128, &i2c);

    for(int i = START_OFFS; i < (START_OFFS + 100); i++) {
        rom.write8(i, 0);
        if(rom.read8(i) != 0 || rom.hasErrorOccurred()) {
            serdebugF2("Rom not cleared at ", i);
        }
    }

    rom.write8(START_OFFS + 0, 0x45);
    serdebugFHex("read8 - 0x45 is ", rom.read8(START_OFFS + 0));

    rom.write16(START_OFFS + 1, 0xa050);
    serdebugFHex("read16 - 0xa050 is ", rom.read16(START_OFFS + 1));
    serdebugFHex("read a - 0xa0 is ", rom.read8(START_OFFS + 1));
    serdebugFHex("read b - 0x50 is ", rom.read8(START_OFFS + 2));

    rom.write32(START_OFFS + 3, 0xd00dface);
    serdebugFHex("read32 - 0xd00dface is ", rom.read32(START_OFFS + 3));

    rom.writeArrayToRom(START_OFFS + 10, (const uint8_t *)stringSource, sizeof(stringSource));
    char data[128];
    rom.readIntoMemArray((uint8_t*)data, START_OFFS + 10, sizeof(stringSource));
    serdebugF2("read string: ", data);

    serdebugF2("ErrorOccurred: ",rom.hasErrorOccurred());
}
