## IoAbstraction Fork of Liquid Crystal Library for Arduino and mbed
 
This is a fork of the popular LiquidCrystal library that works with a wide range of Arduino and mbed boards. It's been refactored to work alongside IOAbstraction framework (https://github.com/davetcc/IoAbstraction) so it supports direct pin connection, I2C via PCF8574, I2C via MCP23017, shift registers, and other supported expansion devices. All delays yield to TaskManager meaning that other tasks can run during these delays. As of V1.3 it is compatible with mbed RTOS framework as long as it is used with at least IoAbstraction 1.5.0, there is an example for mbed shipped with the library.

The main advantage of using this version is with task manager because it calls into task manager after every character is written, there are significant delays in the library code that are needed to wait for the display, these have been converted to yield operations in task manager, so that other tasks can run during these times. The only restriction this brings is that all rendering must be done in *one task only* to avoid causing display corruption. 

## Installation

### Install for Arduino
The easiest way to install is via Arduino IDE Library Manager. It will also install the dependant library IoAbstraction for you.

If you decide to manually install - not recommended, copy this library to your libraries folder and then make sure that you have IoAbstraction library installed too. (https://github.com/davetcc/IoAbstraction)

### Install for PlatformIO (Arduino or mbed):

Install [LiquidCrystalIO from the platformIO library manager](https://platformio.org/lib/show/7242/LiquidCrystalIO).

## Changes from the original version of LiquidCrystal

This library is 99% interchangeable with the original and we are working towards LCDAPI compliance as much as possible. Several extra examples have been added that demonstrate how to configure this library with various types of I2C device and a shift register. The difference are:

1. All internal delays beyond initialization are now performed using taskManager's yieldForMicros() method. Meaning that the delay does not interfere with task manager operation. There is no external difference for this. **However, you must make sure that only one task updates your display if you choose to use task manager.**
2. It is possible to change the IO between any IoAbstraction supported device, be it Arduino pins, shift register or supported I2C devices such as MCP23017 and PCF8574 backpacks. This is provided as an extra parameter to the constructor (defaults to Arduino pins). See examples HelloI2c, HelloShiftReg and Counter23017
3. Supports creating characters in program memory using createCharPgm(charNumber, pgmBytes), see the modified CustomCharacter example.
4. Inbuilt support for backlight via methods: configureBacklight(pin), backlight(), noBacklight() and setBacklight(onOrOff). The backlight can be PWM on Arduino.

## Testing

This fork has been tested on various 16x2 displays both directly connected and i2c, various 20x4 displays, again directly connected and i2c, using a shift register and also using a DfRobot shield.

## Extra features over the original

This libray has far greater control over the backlight than the original library. You can either use a pin on the same device (inverted or regular), or you can use PWM with an analog device, by default it would use the analog pin provided, but in future it could be an I2C DAC or potentiometer. See [davetcc/IoAbstraction] for more on analog devices.

With this function you can set the backlight pin and set the mode to one of the below modes:

    void configureBacklightPin(uint8_t backlightPin, BackLightPinMode mode = LiquidCrystal::BACKLIGHT_NORMAL);

For the backlight mode parameter, provide one of:

        BACKLIGHT_NORMAL, BACKLIGHT_INVERTED,  BACKLIGHT_PWM

This function gives more control over PWM/Analog mode:

    void configureAnalogBacklight(AnalogDevice* analogDevice, uint8_t backlightPin)
    
You can then call setBacklight to change the level for PWM, or turn on/off for digital pins.

    setBacklight(uint8_t level);

Simpler construction for regular I2C backpacks:

    LiquidCrystalI2C_RS_EN(globalVariableName, i2cAddress, isBacklightInverted)
    LiquidCrystalI2C_EN_RS(globalVariableName, i2cAddress, isBacklightInverted)

Simpler DfRobot shield construction by constructing the object with no parameters. For example:

    LiquidCrystal lcd;

## Original text:

This library allows an Arduino board to control LiquidCrystal displays (LCDs) based on the Hitachi HD44780 (or a compatible) chipset, which is found on most text-based LCDs.

For more information about this library please visit us at
http://www.arduino.cc/en/Reference/LiquidCrystal

### Original License (our modifications under same)

Copyright (C) 2006-2008 Hans-Christoph Steiner. All rights reserved.
Copyright (c) 2010 Arduino LLC. All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
