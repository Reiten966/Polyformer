/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _KEYBOARD_MANGER_H_
#define _KEYBOARD_MANGER_H_

#include "IoAbstraction.h"

/**
 * @file KeyboardManager.h contains the classes needed to deal with matrix keyboards
 */

/**
 * When there are changes in keyboard manager it will call the appropriate function on your
 * instance of this class to inform you of the change.
 */
class KeyboardListener {
public:
    /**
     * A key has been pressed or held down.
     * @param key the character code for the key
     * @param held if held down
     */
    virtual void keyPressed(char key, bool held)=0;
    /**
     * A key has been released
     * @param key the character code of the key
     */
    virtual void keyReleased(char key)=0;
};

/**
 * The keyboard manager can handle any layout of matrix keyboard by providing it a layout.
 * This class is used to tell the matrix manager the row and column arrangements of your keyboard
 * matrix. There are two standard ones defined in this file. They are LAYOUT_3X4, LAYOUT_4X4.
 * When creating a class of this type, be sure that your string of keyCode mappings is defined as
 * PROGMEM and at rows * cols in size. You can either use one of the standard defined layouts or
 * generate your own.
 */
class KeyboardLayout {
private:
    uint8_t rows;
    uint8_t cols;
    const char *pgmLayout;
    uint8_t *pins;
public:
    /**
     * Create a keyboard layout with a number of rows and columns, the characters that are associated
     * with each key are also provided in a char array.
     * @param rows the number of rows in the keyboard
     * @param cols the number of columns in the keyboard.
     * @param pgmLayout the character codes for each position in the keyboard
     */
    KeyboardLayout(uint8_t rows, uint8_t cols, const char* pgmLayout) {
        this->rows = rows;
        this->cols = cols;
        this->pgmLayout = pgmLayout;
        this->pins = new uint8_t[rows + cols];
    }

    ~KeyboardLayout() {
        delete pins;
    }

    int numColumns() { return cols; }

    int numRows() { return rows; }

    void setColPin(int col, int pin) {
        if(col < cols) pins[rows + col] = pin;
    }

    void setRowPin(int row, int pin) {
        if(row < rows) pins[row] = pin;
    }

    int getRowPin(int row) {
        return pins[row];
    }

    int getColPin(int col) {
        return pins[rows + col];
    }

    char keyFor(uint8_t row, uint8_t col) {
        if(row < rows && col < cols) return pgm_read_byte_near(&pgmLayout[(row * cols)+col]);
        else return 0;
    }
};

/**
 *  Internally used by the keyboard manager to manage the state of keys.
 */
enum KeyMode: uint8_t {
    KEYMODE_NOT_PRESSED,
    KEYMODE_DEBOUNCE,
    KEYMODE_DEBOUNCE1,
    KEYMODE_DEBOUNCE2,
    KEYMODE_PRESSED,
    KEYMODE_REPEATING
};

#define KEYBOARD_TASK_MILLIS 50

/**
 * A keyboard manager that can determine if a key is pressed or released for a given layout of keyboard. It is configured
 * during initialisation with an IoAbstraction that is used to access hardware, a specific keyboard layout and a listener
 * that will be called back when keys are pressed and released. The keyboard layout also sets what character is associated
 * with each key.
 *
 * You can decide between polling operation and interrupt operation, if interrupt operation is chosen then all the row
 * pins must be on interrupt capable pins, which on many boards is best achieved by using an MCP23017 for all the pins.
 * Do not enable interrupt mode on a PCF8574 as the changing of the output pins will trigger the interrupt.
 */
class MatrixKeyboardManager : public BaseEvent {
private:
    static MatrixKeyboardManager* INSTANCE;
    KeyboardListener* listener;
    KeyboardLayout* layout;
    IoAbstractionRef ioRef;
    uint8_t repeatTicks = 10;
    uint8_t repeatStartTicks = 30;
    char currentKey;
    volatile KeyMode keyMode;
    uint8_t counter;
    bool interruptMode;
public:
    MatrixKeyboardManager();
    void initialise(IoAbstractionRef ref, KeyboardLayout* layout, KeyboardListener* listener, bool interruptMode = false);
    void setRepeatKeyMillis(int startAfterMillis, int repeatMillis);

    uint32_t timeOfNextCheck() override;
    void exec() override;

    friend void rawKeyboardInterrupt();
private:
    void setToOutput(int i);
    void enableAllOutputsForInterrupt();

    void doDebounce(char time);
};

#define MAKE_KEYBOARD_LAYOUT_3X4(varName) const char KEYBOARD_STD_3X4_KEYS[] PROGMEM = "123456789*0#"; KeyboardLayout varName(4, 3, KEYBOARD_STD_3X4_KEYS);
#define MAKE_KEYBOARD_LAYOUT_4X4(varName) const char KEYBOARD_STD_4X4_KEYS[] PROGMEM = "123A456B789C*0#D"; KeyboardLayout varName(4, 4, KEYBOARD_STD_4X4_KEYS);

#endif // _KEYBOARD_MANGER_H_
