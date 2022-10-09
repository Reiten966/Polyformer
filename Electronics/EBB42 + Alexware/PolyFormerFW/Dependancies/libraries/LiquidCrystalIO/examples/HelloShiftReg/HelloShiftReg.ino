/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD along with a counter
 when the LCD device is connected via a shift register. Connect the
 shift register and record the pins onto which it's connected. These
 will be used to configure the shift register IoAbstraction. 

 Be aware that the shift register support works by allowing you to
 chain input and output shift registers together (up to 4 devices).
 
 * Pins 0..31 represent the shift registers that are inputs
 * Pins 32 onwards are the outputs.
 * So the first output register's D0 pin would be pin 32.

 To avoid hardwiring this value and protect against future changes
 instead use the value SHIFT_REGISTER_OUTPUT_CUTOVER.

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
 modified 2018 by Dave Cherry for shift register support.

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include <LiquidCrystalIO.h>

//
// these are the pins onto which the shift register is connected to the arduino
//
#define WRITE_CLOCK_PIN 22
#define WRITE_DATA_PIN 23
#define WRITE_LATCH_PIN 24

//
// these are the shift register pins that you've connected your display to
//
const int rs = SHIFT_REGISTER_OUTPUT_CUTOVER + 0;
const int en = SHIFT_REGISTER_OUTPUT_CUTOVER + 1;
const int d4 = SHIFT_REGISTER_OUTPUT_CUTOVER + 2;
const int d5 = SHIFT_REGISTER_OUTPUT_CUTOVER + 3;
const int d6 = SHIFT_REGISTER_OUTPUT_CUTOVER + 4;
const int d7 = SHIFT_REGISTER_OUTPUT_CUTOVER + 5;

//
// Construction is very similar to the regular library, the only difference is that we can choose to provide
// a different means of reading and writing
//
LiquidCrystal lcd(rs, en, d4, d5, d6, d7, outputOnlyFromShiftRegister(WRITE_CLOCK_PIN, WRITE_DATA_PIN, WRITE_LATCH_PIN));

void setup() {
  // set up the LCD's number of columns and rows, must be called.
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}


