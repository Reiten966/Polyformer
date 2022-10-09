/**
 * A simple example of how to use a resistive touch screen directly with IoAbstraction.
 * This example shows how to integrate directly with a resistive touch screen without any additional devices, using
 * analog inputs. This example takes the readings and reports them to the serial port every few hunderd millis.
 * It's possible to extend the touch base class too and be event-driven, but that's not discussed here.
 */

#include <Arduino.h>
#include <IoAbstraction.h>
#include <ResistiveTouchScreen.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define YPOS_PIN 32
#define XNEG_PIN 33
#define XPOS_PIN 2
#define YNEG_PIN 0
#define MY_CS 22
#define MY_DC 21
#define MY_RST 23

using namespace iotouch;

// the touch screen itself
ResistiveTouchInterrogator interrogator(XPOS_PIN, XNEG_PIN, YPOS_PIN, YNEG_PIN);
ValueStoringResistiveTouchScreen touchScreen(interrogator, TouchInterrogator::PORTRAIT);

// couple of display options here, anything with a touch interface attached!
//Adafruit_ST7735 gfx(MY_CS, MY_DC, MY_RST);
Adafruit_ILI9341 gfx(MY_CS, MY_DC);

int oldX = 0, oldY = 0;

void setup() {
    Serial.begin(115200);
    // step 1. run with calibration off and get the actual min and max values if corrections need to be made
    // step 2. put the corrections into the value below, xmin, xmax, ymin, ymax and try the program again.
    touchScreen.calibrateMinMaxValues(0.15F, 0.75F, 0.06F, 0.91F);
    touchScreen.start();

    SPI.begin();
    gfx.begin();
    gfx.fillScreen(0);

    taskManager.scheduleFixedRate(500, [] {
        serdebugF4("Touch: x, y, touched", touchScreen.getLastX(), touchScreen.getLastY(), touchScreen.getTouchPressure());
        serdebugF2("touchMode: ", touchScreen.getTouchState());

        if(touchScreen.getTouchState() == TOUCHED || touchScreen.getTouchState() == HELD) {
            gfx.fillCircle(oldX, oldY, 16, 0);
            oldX = touchScreen.getLastX() * 240.0F;
            oldY = touchScreen.getLastY() * 320.0F;
            gfx.fillCircle(oldX, oldY, 16, 0xffff);
        }
    });
}

void loop() {
    taskManager.runLoop();
}