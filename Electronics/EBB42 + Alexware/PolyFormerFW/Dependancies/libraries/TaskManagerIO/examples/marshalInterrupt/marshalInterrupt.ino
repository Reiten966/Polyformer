/**
 * This example shows how to marshal an interrupt via task manager. Task manager can register an interrupt on any
 * supported pin, including handling interrupts from external IO devices such as PCF8574 and MCP23017. It does so
 * by registering a raw interrupt handler on your behalf, and then creating a high priority interrupt task when it
 * is triggered.
 *
 * NOTE: This means that there is slight latency on your code being triggered, and it should not be used for anything
 * that is even remotely time critical. For time critical items, do the work that must be done immediately in a direct
 * ISR, then trigger task manager using an event. Never add a task directly in a raw ISR.
 */

#include <TaskManagerIO.h>
#include <BasicInterruptAbstraction.h>

// choose any pin on your board to register an interrupt against.
const int interruptPin = 2;

// now we create an InterruptAbstraction, for this example we use the simple inbuilt ArduinoInterruptExample, but you
// can also use any IoAbstractionRef from IoAbstraction library too.
BasicArduinoInterruptAbstraction interruptAbstraction;

//
// Here we register the interrupt handler task, it will not be called in an ISR, so it's safe to call most functions
// apart from delay here.
//
void interruptTask(pintype_t thePin) {
    Serial.println("Interrupt triggered");
}

//
// Here we set the interrupt task handler, and add the interrupt, the syntax is very similar to attachInterrupt.
//
void setup() {
    Serial.begin(115200);
    Serial.println("Starting interrupt example");

    pinMode(interruptPin, INPUT_PULLUP);

    taskManager.setInterruptCallback(interruptTask);
    taskManager.addInterrupt(&interruptAbstraction, interruptPin, CHANGE);
}

//
// This is the regular loop method for a taskManager sketch, just repeatedly calls runLoop. If you want to use
// this library in a low power situation, then you can use a low power library for your board, see the comments
// within loop. If you use a low power delay function, it must be able to come out of sleep for interrupts on the
// pins that you have interrupts, otherwise, taskManager will not work.
//
void loop() {
    taskManager.runLoop();

    // Here's an example of what you can do in low power situations to reduce power usage.
    //auto microsToTask = taskManager.microsToNextTask();
    //myLowPowerDelayFunction(microsToTask);
}
