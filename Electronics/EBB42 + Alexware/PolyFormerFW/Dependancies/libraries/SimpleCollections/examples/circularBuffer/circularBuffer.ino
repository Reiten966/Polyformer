/**
 * The circular buffer allows a writer to write into a buffer independently of the reader. It is thread and interrupt safe
 * to a large extent on a wide range of boards - ESP32, ESP8266, mbed, Arduino mbed based boards, AVR and SAMD*.
 *
 * To use it you create a CircularBuffer object, and then put items into it and take them out. It is very important
 * to note that the writer can exceed the reader position, so you CAN lose data. However, the this buffer is completely
 * non blocking and is even safe to use from interrupts with one condition - if the interrupt handler is the writer, do
 * not have other tasks or threads writing as well, as this could cause CAS spinning during the update cycle.
 */

#include <Arduino.h>
#include <SimpleCollections.h>
#include <SCCircularBuffer.h>

#define INTERRUPT_PIN 1

SCCircularBuffer buffer(32);

volatile uint8_t counter = 0;

void interruptHasOccurred() {
    // whenever there is an interrupt put an item in our queue, this is safe because it is non-blocking in all cases
    // only make sure the interrupt is the only writer to avoid blocking
    buffer.put(counter++);
}

void setup() {
    // start up the serial port
    Serial.begin(115200);
    while(!Serial);
    Serial.println("Starting circular buffer example");

    // enable interrupts on a pin so we can "put" items in the buffer
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interruptHasOccurred, CHANGE);
}

void loop() {
    // whenever data is available we read it, otherwise we wait a bit in our example, in a real system this would
    // be within task manager, or within a busy loop.
    if(buffer.available()) {
        Serial.print("Buffer read ");
        Serial.println(buffer.get());
    }

    delay(1);
}