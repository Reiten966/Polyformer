/*
  LiquidCrystal Library - Hello World for 23017

  This assumes that an LCD display has been attached to the B port of a 23017 IO expander, connected over i2c.
  it will count up from 0 onto the display once per second.
*/

// include the library code:
#include <LiquidCrystalIO.h>
#include <IoAbstractionWire.h>
#include <Wire.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13;

// if you want to use the optional PWM contrast, you need to set the pin for it
//const int pwmContrastPin = 5;

// if you want to reset the 23017 other than hold it to Vcc, include this.
const int resetPin23017 = 30;

const int lcdWidth = 20;

// now construct the display using IO from a 23017
LiquidCrystal lcd(rs, en, d4, d5, d6, d7, ioFrom23017(0x20));

const byte smiley[8] PROGMEM = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b00000
};

int oldPos = 0;

void setup() {
  
  // Optional:
  // if you don't want to bother connecting a potentiometer for contrast, instead see my example of
  // creating a PWM contrast circuit for the board, it's very simple see:
  // https://www.thecoderscorner.com/electronics/microcontrollers/driving-displays/90-wiring-a-20x4-character-display-to-an-arduino-board/
  //pinMode(pwmContrastPin, OUTPUT);
  //analogWrite(pwmContrastPin, 10);
  // End PWM contrast.

  // this is optional, in a real world system you could probably just connect the
  // reset pin of the device to Vcc, but when prototyping you'll want a reset
  // on every restart.
  pinMode(resetPin23017, OUTPUT);
  digitalWrite(resetPin23017, LOW);
  delayMicroseconds(100);
  digitalWrite(resetPin23017, HIGH);
  // End reset optional code.

  Wire.begin();
 
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);

  // for the slower I2C bus we can remove the delay, as it takes longer than 40us to get there.
  // 0 for 100Khz, 20 for 400khz, 40 for anything else (defaults to 40)
  lcd.setDelayTime(0x00, 0);

  // now create a custom character
  lcd.createCharPgm(1, smiley);
  lcd.setCursor(0,0);
  lcd.print("Counter in seconds");

  //
  // when using this version of liquid crystal, it interacts (fairly) nicely with task manager. 
  // instead of doing stuff in loop, we can schedule things to be done. But just be aware than
  // only one task can draw to the display. Never draw to the display in two tasks.
  //
  // You don't have to use the library with task manager like this, it's an option.
  //
  taskManager.scheduleFixedRate(200, [] {
    // set the cursor to column 0, line 1
    lcd.setCursor(0, 1);

    // print the number of seconds since reset as a float:
    float fractionalMillis = millis() / 1000.0f;
    lcd.print(fractionalMillis);

    // now in row 2 we move our custom character across the screen
    // by first clearing the last position
    lcd.setCursor(oldPos,2);
    lcd.print(' ');

    // find the next position (reset to 0 if need be)
    oldPos++;
    if(oldPos == lcdWidth) oldPos = 0;

    // then print the character
    lcd.setCursor(oldPos,2);
    lcd.write(0x01);
  });
}


void loop() {
    // all sketches that use taskmanager need to call runloop very frequently.
    taskManager.runLoop();
}


