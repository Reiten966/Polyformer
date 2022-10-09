//
// Created by David Cherry on 24/07/2020.
//

#ifndef IOA_PRINT_COMPAT_H
#define IOA_PRINT_COMPAT_H

#include <cstdlib>

/**
 * @file PrintCompat.h
 * Compatibility with the Arduino Print API for mbed boards, should never be included in an Arduino build
 */

#ifdef IOA_USE_ARDUINO
# error "Print compatibility has been included on Arduino, this will cause problems, please report."
#endif

// These are definitions of the mode in which the integer print can work, either decimal, hex or binary.
#define DEC 10
#define HEX 16
#define BIN 2

/**
 * This is a print interface that is roughly compatible with the Arduino one, supporting printing of characters,
 * strings, integers, float, boolean and double. In order to provide a class that supports Print you must implement
 * at least write(char ch);
 */
class Print {
public:
    /**
     * this is the minimum that you must implement to provide the write interface. It writes a single character
     * to the underlying stream.
     * @param ch the character to write.
     * @return returns 1 if written, otherwise 0.
     */
    virtual size_t write(uint8_t ch) = 0;

    /**
     * You can optionally override this function, it writes a whole string to the underlying stream, returning the
     * number of characters written
     * @param sz the string data to write, zero terminated
     * @return the number of characters
     */
    virtual size_t write(const char* sz) {
        int written = 0;
        while(*sz) {
            write(*sz);
            sz++;
            written++;
        }
        return written;
    }

    /**
     * Prints a characcter to the stream without a newline
     * @param ch the character to print.
     */
    void print(char ch) {
        write(ch);
    }

    /**
     * Prints a character to the stream followed by a newline.
     * @param ch
     */
    void println(char ch) {
        write(ch);
        write('\n');
    }

    /**
     * Prints a zero terminated string to the stream
     * @param sz the string to write.
     */
    void print(const char* sz) { write(sz); }

    /**
     * Prints a zero terminated string followed by newline to the stream
     * @param sz the string to write
     */
    void println(const char* sz) {
        write(sz);
        write('\n');
    }

    /**
     * Prints an integer value, with an optional radix, default DEC, but either DEC, HEX or BIN
     * @param val the numeric value to be printed
     * @param radix the base, DEC, HEX or BIN
     */
    void print(int val, int radix = DEC) {
        char sz[33];
        itoa(val, sz, radix);
        write(sz);
    }

    /**
     * Prints an integer value, with an optional radix, default DEC, but either DEC, HEX or BIN, the integer
     * is followed by a new line character.
     * @param val the numeric value to be printed
     * @param radix the base DEC, HEX or BIN
     */
    void println(int val, int radix = DEC) {
        print(val, radix);
        write('\n');
    }

    /**
     * Prints an unsigned integer value, with an optional radix, default DEC, but either DEC, HEX or BIN
     * @param val the numeric value to be printed
     * @param radix the base, DEC, HEX or BIN
     */
    void print(unsigned int val, int radix = DEC) {
        char sz[33];
        // TODO, bring over tcUtil into IoAbstraction for better support
        itoa(int(val), sz, radix);
        write(sz);
    }

    /**
     * Prints an integer value, with an optional radix, default DEC, but either DEC, HEX or BIN, the integer
     * is followed by a new line character.
     * @param val the numeric value to be printed
     * @param radix the base DEC, HEX or BIN
     */
    void println(unsigned int val, int radix = DEC) {
        print(int(val), radix);
        write('\n');
    }

    /**
     * Prints a long value, with an optional radix, default DEC, but either DEC, HEX or BIN
     * @param val the numeric value to be printed
     * @param radix the base DEC, HEX or BIN
     */
    void print(long val, int radix = DEC) {
        print((int)val, radix);
    }

    /**
     * Prints a long value, with an optional radix, default DEC, but either DEC, HEX or BIN, the integer
     * is followed by a new line character.
     * @param val the numeric value to be printed
     * @param radix the base DEC, HEX or BIN
     */
    void println(long val, int radix = DEC) {
        print((int)val, radix);
        write('\n');
    }

    /**
     * Prints an unsigned long value, with an optional radix, default DEC, but either DEC, HEX or BIN
     * @param val the numeric value to be printed
     * @param radix the base, DEC, HEX or BIN
     */
    void print(unsigned long val, int radix = DEC) {
        char sz[33];
        // TODO, bring over tcUtil into IoAbstraction for better support
        itoa(int(val), sz, radix);
        write(sz);
    }

    /**
     * Prints an unsigned long value, with an optional radix, default DEC, but either DEC, HEX or BIN
     * @param val the numeric value to be printed
     * @param radix the base, DEC, HEX or BIN
     */
    void println(unsigned long val, int radix = DEC) {
        print(val, radix);
        write('\n');
    }

    /**
     * Prints a double value to a number of decimal places (maximum supported decimal places is 6).
     * @param dbl the double value to print
     * @param dp the number of decimal places, default 3. Max 6.
     */
    void print(double dbl, int dp = 3) {

        // TODO replace with tcUtil dpToDivisor method when tcUtil is put into IoAbstraction.
        double div = (dp < 2) ? 10 : (dp == 2) ? 100 : (dp == 3) ? 1000 : (dp==4) ? 10000 : (dp==5) ? 100000 : 1000000;

        char sz[20];
        if(dbl < 0) write('-');
        int whole = int(dbl);
        int fraction = int((dbl - double(whole)) * div);
        itoa((int)abs(dbl), sz, 10);
        write(sz);
        write('.');
        itoa((int)abs(fraction), sz, 10);
        write(sz);
    }

    /**
     * Prints a double value to a number of decimal places (maximum supported decimal places is 19). This is followed
     * by a newline.
     * @param dbl the double value to print
     * @param dp the number of decimal places, default 3. Max 19.
     */
    void println(double dbl, int dp = 3) {
        print(dbl, dp);
        write('\n');
    }

    /**
     * Prints a boolean value, either true or false.
     * @param b the boolean to print.
     */
    void print(bool b) {
        write(b ? "true" : "false");
    }

    /**
     * Prints a boolean value, either true or false, followed by a newline.
     * @param b the boolean to print.
     */
    void println(bool b) {
        write(b ? "true\n" : "false\n");
    }

    /**
     * Print a newline character.
     */
    void println() {
        write('\n');
    }

    //
    // the following are to handle cases where write is called with basically an invalid type. I'd personally
    // of liked not to port this, but too much code relies on it being there.
    //

    inline size_t write(short t) { return write((uint8_t)t); }
    inline size_t write(unsigned short t) { return write((uint8_t)t); }
    inline size_t write(int t) { return write((uint8_t)t); }
    inline size_t write(unsigned int t) { return write((uint8_t)t); }
    inline size_t write(long t) { return write((uint8_t)t); }
    inline size_t write(unsigned long t) { return write((uint8_t)t); }
    // Enable write(char) to fall through to write(uint8_t)
    inline size_t write(char c) { return write((uint8_t) c); }
    inline size_t write(int8_t c) { return write((uint8_t) c); }
};

//forward definition of yield() function
void yield();

#endif //IOA_PRINT_COMPAT_H
