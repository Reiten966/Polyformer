#include <PlatformDetermination.h>

#ifdef IOA_USE_MBED
# include <cstdint>
# include <PrintCompat.h>
# define pgm_read_byte_near(x) *x;
# define delayMicroseconds(x) (wait_us(x))
#endif

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <AnalogDeviceAbstraction.h>
#include "LiquidCrystalIO.h"

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, BasicIoAbstraction *ioMethod) {
    init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7, ioMethod);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, BasicIoAbstraction *ioMethod) {
    init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7, ioMethod);
}

// df robot constructor need no args
LiquidCrystal::LiquidCrystal() {
    init(1, 8, 0xff, 9, 4, 5, 6, 7, 0, 0, 0, 0, nullptr);
    configureBacklightPin(10);
    backlight();
}

// I2C backback constructor
LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t en, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
              LiquidCrystal::BackLightPinMode mode, IoAbstractionRef ioMethod) {
    init(1, rs, rw, en, d0, d1, d2, d3, 0, 0, 0, 0, ioMethod);

    // and they all use PCF8574 which is limited to 100khz, the delay can be removed.
    setDelayTime(0x00, 0);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, BasicIoAbstraction *ioMethod) {
    init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0, ioMethod);
}

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable,
                             uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, BasicIoAbstraction *ioMethod) {
    init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0, ioMethod);
}

void LiquidCrystal::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
                         uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                         uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, BasicIoAbstraction *ioMethod) {
    // in the event of null, assume we will use arduino pins.
    _rs_pin = rs;
    _rw_pin = rw;
    _enable_pin = enable;
    _backlightPin = 0xff;
    _backlightMode = BACKLIGHT_NORMAL;
    _io_method = ioMethod != nullptr ? ioMethod : internalDigitalIo();
    _analog_device = nullptr;
    _delayTime = 40;

    _data_pins[0] = d0;
    _data_pins[1] = d1;
    _data_pins[2] = d2;
    _data_pins[3] = d3;
    _data_pins[4] = d4;
    _data_pins[5] = d5;
    _data_pins[6] = d6;
    _data_pins[7] = d7;

    if (fourbitmode)
        _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    else
        _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;

    // we cannot call begin from the CTOR, it is dangerous and uses pins before the
    // underlying device is fully initialised. Fails completely for i2c.
    //begin(16, 1);
}

void LiquidCrystal::configureBacklightPin(uint8_t backlightPin, BackLightPinMode mode) {
    if(mode == BACKLIGHT_PWM) {
        configureAnalogBacklight(internalAnalogIo(), backlightPin);
    }
    else {
        _backlightPin = backlightPin;
        _backlightMode = mode;
        ioDevicePinMode(_io_method, _backlightPin, OUTPUT);
        backlight();
    }
}

void LiquidCrystal::configureAnalogBacklight(AnalogDevice* analogDevice, uint8_t backlightPin) {
    _backlightPin = backlightPin;
    _backlightMode = BACKLIGHT_PWM;
    _analog_device = (analogDevice != nullptr) ? analogDevice : internalAnalogIo();
    _analog_device->initPin(backlightPin, DIR_PWM);
    backlight();
}

void LiquidCrystal::setBacklight(uint8_t state) {
    if (_backlightPin == 0xff) return;

    if (_backlightMode == BACKLIGHT_PWM && _analog_device) {
        _analog_device->setCurrentFloat(_backlightPin, float(state) / 255.0F);
    }
    else {
        if (_backlightMode == BACKLIGHT_INVERTED) state = !state;
        ioDeviceDigitalWriteS(_io_method, _backlightPin, state);
    }
}

void LiquidCrystal::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
    if (_io_method == nullptr) _io_method = internalDigitalIo();

    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lines;
    setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    _io_method->pinDirection(_rs_pin, OUTPUT);
    // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
    if (_rw_pin != 255) {
        _io_method->pinDirection(_rw_pin, OUTPUT);
    }
    _io_method->pinDirection(_enable_pin, OUTPUT);

    // Do these once, instead of every time a character is drawn for speed reasons.
    for (int i = 0; i < ((_displayfunction & LCD_8BITMODE) ? 8 : 4); ++i) {
        _io_method->pinDirection(_data_pins[i], OUTPUT);
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
    taskManager.yieldForMicros(50000);
    // Now we pull both RS and R/W low to begin commands

    _io_method->writeValue(_rs_pin, LOW);
    _io_method->writeValue(_enable_pin, LOW);
    if (_rw_pin != 255) {
        _io_method->writeValue(_rw_pin, LOW);
    }
    _io_method->runLoop();

    //put the LCD into 4 bit or 8 bit mode
    if (!(_displayfunction & LCD_8BITMODE)) {
        // this is according to the hitachi HD44780 datasheet
        // figure 24, pg 46

        // we start in 8bit mode, try to set 4 bit mode
        write4bits(0x03);
        taskManager.yieldForMicros(4500); // wait min 4.1ms

        // second try
        write4bits(0x03);
        taskManager.yieldForMicros(4500); // wait min 4.1ms

        // third go!
        write4bits(0x03);
        taskManager.yieldForMicros(150);

        // finally, set to 4-bit interface
        write4bits(0x02);
    } else {
        // this is according to the hitachi HD44780 datasheet
        // page 45 figure 23

        // Send function set command sequence
        command(LCD_FUNCTIONSET | _displayfunction);
        taskManager.yieldForMicros(4500);  // wait more than 4.1ms

        // second try
        command(LCD_FUNCTIONSET | _displayfunction);
        taskManager.yieldForMicros(150);

        // third go
        command(LCD_FUNCTIONSET | _displayfunction);
    }

    // finally, set # lines, font size, etc.
    command(LCD_FUNCTIONSET | _displayfunction);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    // clear it off
    clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal::setRowOffsets(int row0, int row1, int row2, int row3) {
    _row_offsets[0] = row0;
    _row_offsets[1] = row1;
    _row_offsets[2] = row2;
    _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void LiquidCrystal::clear() {
    command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    taskManager.yieldForMicros(2000);  // this command takes a long time!
}

void LiquidCrystal::home() {
    command(LCD_RETURNHOME);  // set cursor position to zero
    taskManager.yieldForMicros(2000);  // this command takes a long time!
}

void LiquidCrystal::setCursor(uint8_t col, uint8_t row) {
    const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
    if (row >= max_lines) {
        row = max_lines - 1;    // we count rows starting w/0
    }
    if (row >= _numlines) {
        row = _numlines - 1;    // we count rows starting w/0
    }

    command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal::noDisplay() {
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::display() {
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal::noCursor() {
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::cursor() {
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal::noBlink() {
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal::blink() {
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft() {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LiquidCrystal::scrollDisplayRight() {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight() {
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft() {
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal::autoscroll() {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal::noAutoscroll() {
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar(uint8_t location, const uint8_t charmap[]) {
    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    for (int i = 0; i < 8; i++) {
        write(charmap[i]);
    }
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createCharPgm(uint8_t location, const uint8_t charmap[]) {
    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    for (int i = 0; i < 8; i++) {
        write(pgm_read_byte_near(&charmap[i]));
    }
}

/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystal::command(uint8_t value) {
    send(value, LOW);
}

inline size_t LiquidCrystal::write(uint8_t value) {
    send(value, HIGH);
    return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LiquidCrystal::send(uint8_t value, uint8_t mode) {
    _io_method->writeValue(_rs_pin, mode);

    // if there is a RW pin indicated, set it low to Write
    if (_rw_pin != 255) {
        _io_method->writeValue(_rw_pin, LOW);
    }

    if (_displayfunction & LCD_8BITMODE) {
        write8bits(value);
    } else {
        write4bits(value >> 4);
        write4bits(value);
    }
    taskManager.yieldForMicros(_delayTime);   // commands need > 37us to settle
}

void LiquidCrystal::pulseEnable() {
    _io_method->writeValue(_enable_pin, HIGH);
    _io_method->runLoop();
    delayMicroseconds(1);    // enable pulse must be >450ns
    _io_method->writeValue(_enable_pin, LOW);
    _io_method->runLoop();
}

void LiquidCrystal::write4bits(uint8_t value) {
    for (int i = 0; i < 4; i++) {
        _io_method->writeValue(_data_pins[i], (value >> i) & 0x01);
    }
    pulseEnable();
}

void LiquidCrystal::write8bits(uint8_t value) {
    for (int i = 0; i < 8; i++) {
        _io_method->writeValue(_data_pins[i], (value >> i) & 0x01);
    }
    pulseEnable();
}
