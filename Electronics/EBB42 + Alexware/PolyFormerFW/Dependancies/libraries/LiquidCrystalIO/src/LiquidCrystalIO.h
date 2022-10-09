#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#ifdef IOA_USE_MBED
#include "PrintCompat.h"
#else

#include "Print.h"

#endif

#include <IoAbstraction.h>
#include <inttypes.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

/**
 * @file LiquidCrystalIO.h
 * A fork of the LiquidCrystal library to take advantage of IoAbstraction, supporting I2C IoExpanders, Shift registers
 * and device pins with the same library. Heavily based on the original Arduino version of the library.
 */

// a forward reference to the analog device
class AnalogDevice;

/**
 * LiquidCrystalIO library works with displays that are compatible with the HD44780 driver. It is derived from the
 * Arduino version, but it supports I2C backpacks and many other configurations. In addition it also uses taskmanager
 * to manage any delays needed for the display hardware, thus reducing the overhead.
 */
class LiquidCrystal : public Print {
public:
    /**
     * We support several backlight options, when setting the backlight configuration choose one of the below
     */
    enum BackLightPinMode {
        /** the backlight is active HIGH */
        BACKLIGHT_NORMAL,
        /** the backlight is active LOW */
        BACKLIGHT_INVERTED,
        /** the backlight is connected directly to a PWM pin */
        BACKLIGHT_PWM
    };

    /**
     * Sets up the library to work with a  DfRobot shield connected on regular pins. For this no configuration
     * is needed.
     */
    LiquidCrystal();

    /**
     * Used by the two helpers that can quickly create an instance of LiquidCrystal for most I2C backpacks. Normally
     * you would use LiquidCrystalI2C_RS_EN or LiquidCrystalI2C_EN_RS instead. An example of using of those helpers is
     * shown below.
     *
     * At global level (outside of any functions):
     *
     * `LiquidCrystalI2C_RS_EN(variableName, i2cAddr, invertBacklight)`
     *
     * where
     *   variableName is the variable name to assign to. For example `lcd`
     *   i2cAddr is the i2c address of the backpack, for example `0x20`
     *   invertBacklight true if the backlight function is inverted, otherwise false
     *
     * Example: variable name `lcd`, I2C backpack on addr 0x20, with EN-RW-RS arrangement, inverted backlight function:
     *
     * `LiquidCrystalI2C_EN_RS(lcd, 0x20, true)`
     *
     * @param rs the pin on which RS is connected
     * @param rw the pin on which RW is connected
     * @param enable the pin on which EN is connected
     * @param d0 data pin
     * @param d1 data pin
     * @param d2 data pin
     * @param d3 data pin
     * @param mode the backlight mode from the BackLightPinMode enum
     * @param ioMethod the IO device to use
     */
    LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t en, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                  LiquidCrystal::BackLightPinMode mode, IoAbstractionRef ioMethod);

    /**
     * Sets up liquid crystal in 8 bit mode, optionally providing an IoDevice abstraction
     * @param rs the pin on which RS is connected
     * @param enable the pin on which EN is connected
     * @param d0 data pin
     * @param d1 data pin
     * @param d2 data pin
     * @param d3 data pin
     * @param d4 data pin
     * @param d5 data pin
     * @param d6 data pin
     * @param d7 data pin
     * @param ioMethod optionally an IoAbstractionRef to a different IO device.
     */
    LiquidCrystal(uint8_t rs, uint8_t enable,
                  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, BasicIoAbstraction *ioMethod = NULL);

    /**
     * Sets up liquid crystal in 8 bit mode, optionally providing an IoDevice abstraction
     * @param rs the pin on which RS is connected
     * @param rw the pin on which RW is connected
     * @param enable the pin on which EN is connected
     * @param d0 data pin
     * @param d1 data pin
     * @param d2 data pin
     * @param d3 data pin
     * @param d4 data pin
     * @param d5 data pin
     * @param d6 data pin
     * @param d7 data pin
     * @param ioMethod optionally an IoAbstractionRef to a different IO device.
     */
    LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, BasicIoAbstraction *ioMethod = NULL);

    /**
     * Sets up liquid crystal in 4 bit mode, optionally providing an IoDevice abstraction
     * @param rs the pin on which RS is connected
     * @param rw the pin on which RW is connected
     * @param enable the pin on which EN is connected
     * @param d0 data pin
     * @param d4 data pin
     * @param d5 data pin
     * @param d6 data pin
     * @param d7 data pin
     * @param ioMethod optionally an IoAbstractionRef to a different IO device.
     */
    LiquidCrystal(uint8_t rs, uint8_t rw, uint8_t enable,
                  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, BasicIoAbstraction *ioMethod = NULL);

    /**
     * Sets up liquid crystal in 4 bit mode, optionally providing an IoDevice abstraction
     * @param rs the pin on which RS is connected
     * @param enable the pin on which EN is connected
     * @param d4 data pin
     * @param d5 data pin
     * @param d6 data pin
     * @param d7 data pin
     * @param ioMethod optionally an IoAbstractionRef to a different IO device.
     */
    LiquidCrystal(uint8_t rs, uint8_t enable,
                  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, BasicIoAbstraction *ioMethod = NULL);

    /**
     * Internal method not normally called by users of the library directly.
     * Called by all the other constructors to actually do the main initialisation of the LCD
     * @param fourbitmode if the device is operating in 4 or 8 bit method
     * @param rs the pin on which RS is connected
     * @param rw the pin on which RW is connected
     * @param enable the pin on which EN is connected
     * @param d0 data pin
     * @param d1 data pin
     * @param d2 data pin
     * @param d3 data pin
     * @param d4 data pin
     * @param d5 data pin
     * @param d6 data pin
     * @param d7 data pin
     * @param ioMethod the IoAbstractionRef or nullptr if not set.
     */
    void init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
              uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
              uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, BasicIoAbstraction *ioMethod);

    /**
     * Sets the device that will be used, IE where the LCD pins are connected to.
     * @param ioRef the reference to an IO device previously created.
     */
    void setIoAbstraction(IoAbstractionRef ioRef) {
        _io_method = ioRef;
    }

    /**
     * Configure a backlight pin that is used to control the backlight using the setBacklight(level) function.
     * Depending on the type of display you have this may be either:
     *  * LiquidCrystal::BACKLIGHT_NORMAL - logic high for backlight on
     *  * LiquidCrystal::BACKLIGHT_INVERTED - logic low for backlight on,
     *  * LiquidCrystal::BACKLIGHT_PWN backlight pin in connected to a PWM pin and can dim.
     * @param backlightPin the pin on which the backlight is connected
     * @param mode one of the modes specified above
     */
    void configureBacklightPin(uint8_t backlightPin, BackLightPinMode mode = LiquidCrystal::BACKLIGHT_NORMAL);

    /**
     * This function gives more control over how the PWM/DAC/AnalogOut based backlight is configured, you can
     * provide an analog device, you can specifically tell the analog device if it is PWM or DAC mode. For many
     * cases the regular configureBacklightPin would work.
     * @param analogDevice the analog device to use to set the backlight
     * @param backlightPin the pin on which the backlight is set.
     * @param isPinPwm if the pin on the provided device is a PWM or DAC pin
     */
    void configureAnalogBacklight(AnalogDevice* analogDevice, uint8_t backlightPin);

    /**
     * Sets the delay for a given command, at this moment, this can only set the settle time of a standard command.
     * @param command the command to set the delay for, presently only the settle time.
     * @param settleTime the new value of the delay in microseconds.
     */
    void setDelayTime(uint8_t command, uint8_t settleTime) { _delayTime = (settleTime > 1) ? settleTime : 1; }

    /**
     * Unlike the regular display where this is option, in this variant you absolutely must call this function
     * before use.
     * @param cols the number of colums
     * @param rows the number of rows
     * @param charsize the character size, defaulted to 5x8
     */
    void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

    /**
     * Clears the display completely using the clear command on the device, warning that this is a very slow
     * call taking at least 2000 micros.
     */
    void clear();

    /**
     * Moves the display back to the home position, calling setCursor(0,0) is much faster.
     */
    void home();

    /**
     * Turn on the backlight
     */
    void backlight() { setBacklight(HIGH); }

    /**
     * Turn off the backlight
     */
    void noBacklight() { setBacklight(LOW); }

    /**
     * set the backlight state depending on how it is configured. If it is set to PWM mode, then this can be a value
     * between 0 and the maximum PWM value.
     * @param state
     */
    void setBacklight(uint8_t state);

    /**
     * Turn the display off
     */
    void noDisplay();

    /**
     * Turn the display on
     */
    void display();

    /**
     * Turn off blinking text
     */
    void noBlink();

    /**
     * Turn on blinking text
     */
    void blink();

    /**
     * Turn off the screen cursor
     */
    void noCursor();

    /**
     * Turn on the screen cursor
     */
    void cursor();

    void scrollDisplayLeft();

    void scrollDisplayRight();

    void leftToRight();

    void rightToLeft();

    void autoscroll();

    void noAutoscroll();

    /**
     * Internal method that is not really for direct calling by users that initialises the instance with the right
     * memory locations to write rows into.
     * @param row1 the first display row memory
     * @param row2 the second display row memory
     * @param row3 the third display row memory
     * @param row4 the fourth display row memory
     */
    void setRowOffsets(int row1, int row2, int row3, int row4);

    /**
     * Create a 5x8 character from RAM. You can set any character between 0 and 7
     * @param charNo the character number to use
     * @param data the data for the custom character
     */
    void createChar(uint8_t charNo, const uint8_t data[]);

    /**
     * Create a 5x8 character from PROGMEM. You can set any character between 0 and 7
     * @param charNo the character number to use
     * @param data the data for the custom character - from PROGMEM
     */
    void createCharPgm(uint8_t, const uint8_t[]);

    /**
     * Sets the cursor position at which the next write will take place
     * @param x the x location to move to
     * @param y the y location to move to
     */
    void setCursor(uint8_t x, uint8_t y);

    /**
     * Write a single character to the display and moves the cursor position
     * @return always 1
     * @param ch the character to write
     */
    size_t write(uint8_t ch) override;

    void command(uint8_t);

    using Print::write;

private:
    void send(uint8_t, uint8_t);

    void write4bits(uint8_t);

    void write8bits(uint8_t);

    void pulseEnable();

    uint8_t _rs_pin; // LOW: command.  HIGH: character.
    uint8_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
    uint8_t _enable_pin; // activated by a HIGH pulse.
    uint8_t _data_pins[8];

    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;

    uint8_t _delayTime;
    BackLightPinMode _backlightMode;
    uint8_t _backlightPin;

    uint8_t _numlines;
    uint8_t _row_offsets[4];

    BasicIoAbstraction *_io_method;
    AnalogDevice* _analog_device;
};

/**
 * Create a LiquidCrystal instance suitable for I2C backpacks that have the RS pin first. First parameter is the variable
 * name, followed by the i2c address of the device, and lastly if the backlight is inverted. Please ensure that you
 * include IoAbstractionWire.h before using.
 */
#define LiquidCrystalI2C_RS_EN(var, addr, backlightInv) \
    extern BasicIoAbstraction* ioFrom8574(uint8_t, pinid_t, WireType, bool); \
    auto macro_backlight_tc = (backlightInv) ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_NORMAL; \
    LiquidCrystal var(0, 1, 2, 4, 5, 6, 7, macro_backlight_tc, ioFrom8574(addr, 0xff, &Wire, false));

/**
 * Create a LiquidCrystal instance suitable for I2C backpacks that have the EN pin first. First parameter is the variable
 * name, followed by the i2c address of the device, and lastly if the backlight is inverted. Please ensure that you
 * include IoAbstractionWire.h before using.
 */
#define LiquidCrystalI2C_EN_RS(var, addr, backlightInv) \
    extern BasicIoAbstraction* ioFrom8574(uint8_t, pinid_t, WireType, bool); \
    auto macro_backlight_tc = (backlightInv) ? LiquidCrystal::BACKLIGHT_INVERTED : LiquidCrystal::BACKLIGHT_NORMAL; \
    LiquidCrystal var(2, 1, 0, 4, 5, 6, 7, macro_backlight_tc, ioFrom8574(addr, 0xff, &Wire, false));

#endif
