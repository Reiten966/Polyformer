/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display when the display is fitted
 with an I2C backpack. The LiquidCrystal library works with all LCD
 displays that are compatible with the Hitachi HD44780 driver. 

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi
modified by Dave Cherry in 2018 to demo I2C backpack support.
 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include <Arduino.h>
#include <LiquidCrystalIO.h>

// When using the I2C version, these two extra includes are always needed. Doing this reduces the memory slightly for
// users that are not using I2C.
#include <IoAbstractionWire.h>
#include <Wire.h>

// For most standard I2C backpacks one of the two helper functions below will create you a liquid crystal instance
// that's ready configured for I2C. Important Note: this method assumes a PCF8574 running at 100Khz. If otherwise
// use a custom configuration as you see in many other examples.

// If your backpack is wired RS,RW,EN then use this version
LiquidCrystalI2C_RS_EN(lcd, 0x27, false)

// If your backpack is wired EN,RW,RS then use this version instead of the above.
//LiquidCrystalI2C_EN_RS(lcd, 0x27, false)

void setup() {
    Serial.begin(115200);
    Serial.println("Starting LCD example");
    // for i2c variants, this must be called first.
    Wire.begin();

    Serial.println("Configure LCD");

    // set up the LCD's number of columns and rows, must be called.
    lcd.begin(16, 2);

    // most backpacks have the backlight on pin 3.
    lcd.configureBacklightPin(3);
    lcd.backlight();

    // Print a message to the LCD.
    lcd.print("hello over i2c!");

    //
    // when using this version of liquid crystal, it interacts (fairly) nicely with task manager.
    // instead of doing stuff in loop, we can schedule things to be done. But just be aware than
    // only one task can draw to the display. Never draw to the display in two tasks.
    //
    // You don't have to use the library with task manager like this, it's an option.
    //
    taskManager.scheduleFixedRate(250, [] {
        Serial.println("Set cursor & print");

        // set the cursor to column 0, line 1
        // (note: line 1 is the second row, since counting begins with 0):
        lcd.setCursor(0, 1);
        // print the number of seconds since reset:
        float secondsFraction = millis() / 1000.0F;
        lcd.print(secondsFraction);
    });
}

void loop() {
    taskManager.runLoop();
}


