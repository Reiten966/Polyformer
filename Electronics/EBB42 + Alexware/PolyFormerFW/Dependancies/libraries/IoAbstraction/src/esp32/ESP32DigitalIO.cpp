/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <IoLogging.h>
#include "PlatformDetermination.h"
#include "BasicIoAbstraction.h"

#if defined(IOA_USE_ARDUINO) && defined(ESP32) && defined(IOA_USE_ESP32_EXTRAS)

IoAbstractionRef arduinoAbstraction = nullptr;
IoAbstractionRef ioUsingArduino() {
    if (arduinoAbstraction == nullptr) {
        arduinoAbstraction = new BasicIoAbstraction();
    }
    return arduinoAbstraction;
}

volatile bool esp32InterruptDriverLoaded = false;

gpio_mode_t toEsp32Mode(uint8_t mode) {
    if(mode == INPUT || mode == INPUT_PULLUP || mode == INPUT_PULLDOWN) return GPIO_MODE_INPUT;
    else return GPIO_MODE_OUTPUT;
}

void BasicIoAbstraction::pinDirection(pinid_t pin, uint8_t mode) {
    // pins 32 onwards are input only and do not have pull functions
    if(pin >= 32 && (mode == INPUT_PULLDOWN || mode == INPUT_PULLUP)) {
        mode = INPUT;
    }

    gpio_config_t config;
    config.pin_bit_mask = uint64_t(1U) << uint64_t(pin);
    config.mode = toEsp32Mode(mode);
    config.pull_up_en = (mode == INPUT_PULLUP) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    config.pull_down_en = (mode == INPUT_PULLDOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;
    if(ESP_OK != gpio_config(&config)) {
        serlogF3(SER_ERROR, "ESP digital config error on ", pin, mode);
    }
}

void BasicIoAbstraction::writeValue(pinid_t pin, uint8_t value) {
    gpio_set_level(static_cast<gpio_num_t>(pin), value);
}

uint8_t BasicIoAbstraction::readValue(pinid_t pin) {
    return gpio_get_level(static_cast<gpio_num_t>(pin));
}

void ISR_ATTR rawCbHandler(void* handler) {
    auto intCb = reinterpret_cast<RawIntHandler>(handler);
    intCb();
}

void BasicIoAbstraction::attachInterrupt(pinid_t pin, RawIntHandler interruptHandler, uint8_t mode) {
    if(!esp32InterruptDriverLoaded) {
        const auto defaultFlags = 0;
        if(ESP_OK == gpio_install_isr_service(defaultFlags)) {
            esp32InterruptDriverLoaded = true;
            serlogF(SER_IOA_INFO, "Interrupt driver loaded");
        }
        else {
            serlogF(SER_ERROR, "Interrupt driver did not load");
        }
    }
    auto gpioRef = static_cast<gpio_num_t>(pin);
    auto espIntMode = mode == CHANGE ? GPIO_INTR_ANYEDGE : mode == RISING ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE;
    bool ok = gpio_set_intr_type(gpioRef, espIntMode) == ESP_OK;
    ok = ok && gpio_isr_handler_add(gpioRef, rawCbHandler, (void*)interruptHandler) == ESP_OK;

    serlogF4(SER_ERROR, "Interrupt add for pin ", pin, mode, ok);
    serlogF2(SER_ERROR, "reg ", (long)interruptHandler);

}

void BasicIoAbstraction::writePort(pinid_t port, uint8_t portVal) {
    // not directly supported on esp32 functions
}

uint8_t BasicIoAbstraction::readPort(pinid_t port) {
    // not directly supported on esp32 functions
    return 0;
}

#endif
