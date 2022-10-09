
#include <Arduino.h>
#include <AUnit.h>
#include <Wire.h>
#include <STM32FreeRTOS.h>

using namespace aunit;

volatile bool mainRunning = true;

void appMain(void* /*arg*/) {
    Serial.println("Start II");
    TestRunner::setTimeout(60);
    while(mainRunning) {
        TestRunner::run();
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Start");

    xTaskCreate(appMain, "main", configMINIMAL_STACK_SIZE * 4U, nullptr, 2, nullptr);
    vTaskStartScheduler();
}

void loop() {

}
