#include <PlatformDetermination.h>
#include "ESP32TouchKeysAbstraction.h"
#include <driver/touch_sensor.h>

volatile int intCount = 0;

void esp32TouchKeyInterruptHandler(void* touchAbsAsVoid) {
    TaskManager::markInterrupted(0);
    intCount++;
}

bool ESP32TouchKeysAbstraction::runLoop() {
    return allOk;
}

void ESP32TouchKeysAbstraction::attachInterrupt(pinid_t pin, RawIntHandler interruptHandler, uint8_t mode) {
    interruptCodeNeeded = true;
}

uint8_t ESP32TouchKeysAbstraction::readValue(pinid_t pin) {
    if(!allOk) return 0;

    ensureInterruptRegistered();

    uint16_t val;
    touch_pad_read_filtered((touch_pad_t)pin, &val);
    return val <= pinThreshold;
}

void ESP32TouchKeysAbstraction::pinDirection(pinid_t pin, uint8_t mode) {
    if(!allOk) return;
    serdebugF2("Pin Direction ", pin);
    touch_pad_config((touch_pad_t) pin, pinThreshold);
    touch_pad_set_trigger_mode(TOUCH_TRIGGER_BELOW);
}

ESP32TouchKeysAbstraction::ESP32TouchKeysAbstraction(int defThreshold, touch_high_volt_t highVoltage,
                                                     touch_low_volt_t lowVoltage, touch_volt_atten_t attenuation) {
    allOk = touch_pad_init() == ESP_OK;
    serdebugF2("touch_pad_init ", allOk);
    touch_pad_set_voltage(highVoltage, lowVoltage, attenuation);
    serdebugF2("touch_pad set voltage ", allOk);
    interruptCodeNeeded = false;
    startedUp = false;
    pinThreshold = defThreshold;
    enabledPinMask = 0;
    taskManager.scheduleFixedRate(5, [] {serdebugF2("intCount=", intCount);}, TIME_SECONDS);
}

void ESP32TouchKeysAbstraction::ensureInterruptRegistered() {
    if(!startedUp) {
        startedUp = true;
        touch_pad_filter_start(DEFAULT_TOUCHKEY_FILTER_FREQ);
    }

    if(interruptCodeNeeded) {
        interruptCodeNeeded = false;
        touch_pad_isr_register(esp32TouchKeyInterruptHandler, this);
        allOk = touch_pad_intr_enable() == ESP_OK;
        serdebugF2("Enabled interrupts for touch sensor ok=", allOk);
    }
}
