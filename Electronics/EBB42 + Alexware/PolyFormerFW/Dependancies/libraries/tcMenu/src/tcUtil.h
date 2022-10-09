/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TCUTIL_H
#define _TCUTIL_H

#include <PlatformDetermination.h>
#ifdef IOA_USE_MBED
#include <mbed.h>
#ifndef TCMENU_MBED_NO_MINMAX
#define min(x, y) (((x) < (y))?(x):(y))
#define max(x, y) (((x) > (y))?(x):(y))
#endif //TCMENU_MBED_NO_MINMAX
#define highByte(x) ((x) >> 8)
#define lowByte(x) ((x) & 0xff)
#define ltoa(a,b,c) itoa(a,b,c)
#define strcmp_P(x,y) strcmp(x,y)
#define strncpy_P(x,y,z) strncpy(x,y,z)
#define strcpy_P(x,y) strcpy(x,y)
#define strlen_P(x) strlen(x)
#endif

#ifdef IOA_ARDUINO_MBED
#define ltoa(a,b,c) itoa(a,b,c)
#endif

#include <BasicIoAbstraction.h>
#include "RemoteTypes.h"

// forward reference.
class MenuItem;

/**
 * @file tcUtil.h
 * 
 * A series of utilities that used thoughout tcMenu
 */

/**
 * This structure is created in program memory and passed to all RemoteConnector instances.
 * It contains the name that this device should be identified by and it's UUID.
 */
struct ConnectorLocalInfo {
    char name[30];
    char uuid[38];
};

/**
 * appends a character at the end of the string, if there is space according to len
 */
void appendChar(char* str, char val, int len);

/**
 * used with the below ltoa functions, pass this in padChar when not padded.
 */
#define NOT_PADDED 0

/**
 * A fast long to ascii function that more feature complete than the standard library. 
 * Supports zero padding and maximum number of decimal places. This version always
 * starts at position 0 in the provided buffer and goes up to position len always leaving
 * space for a terminator. The other two versions below support appending instead.
 * 
 * @param str the buffer to be output to
 * @param val the value to be converted
 * @param divisor the power of 10 largest value (eg 10000, 1000000L etc)
 * @param padChar the character to pad with (or NOT_PADDED which is 0)
 * @param len the length of the buffer passed in, it will not be exceeded.
 */
void ltoaClrBuff(char* str, long val, uint8_t dp, char padChar, int len);

/**
 * A fast long to ascii function that more feature complete than the standard library. 
 * Supports zero padding and the largest actual value to use normally a power of 10.
 * Absolute largest value displayable is 1000000000 - 1. NOTE that this function will
 * append at the end of the current string. Use ltoaClrBuff to start at position 0.
 * This call will not exceed the length provided and will properly terminate the string.
 * 
 * @param str the buffer to be appended to
 * @param val the value to be converted
 * @param divisor the power of 10 largest value (eg 10000, 1000000L etc)
 * @param padChar the character to pad with (or NOT_PADDED which is 0)
 * @param len the length of the buffer passed in, it will not be exceeded.
 */
void fastltoa_mv(char* str, long val, long divisor, char padChar, int len);

/**
 * A fast long to ascii function that more feature complete than the standard library. 
 * Supports zero padding and the number of decimal places to use. Maximum number of
 * decimal places is 9. NOTE that this function will append at the end of the current
 * string and will not exceed the length provided, it will also properly terminate the
 * string. Use ltoaClrBuff to start at position 0 in the buffer.
 * 
 * @param str the buffer to be appended to
 * @param val the value to be converted
 * @param dp the number of decimal places allowed
 * @param padChar the character to pad with (or NOT_PADDED which is 0)
 * @param len the length of the buffer passed in, it will not be exceeded.
 */
void fastltoa(char* str, long val, uint8_t dp, char padChar, int len);

/**
 * A very simple floating point string function based on the fastltoa above. It can print
 * floating point values up to 9 whole digits and 9 decimal places. Note this function
 * appends the floating point value at the end of the string, to put the value at the
 * beginning, ensure the string is zero length.
 * @param sz the string to append to,
 * @param fl the float to convert
 * @param dp the numer of decimal places (max 9)
 * @param strSize the string maximum length (usually from sizeof)
 */
void fastftoa(char* sz, float fl, int dp, int strSize);

/**
 * converts decimal places into a suitable divisor, eg: 2 -> 100, 4 -> 10000
 */
long dpToDivisor(int dp);

/**
 * Indicates how many integers are needed to represent the value and negative flag if provided
 * @param value the value to represent as unsigned.
 * @param negative if the value is negative
 * @return the number of characters including the sign needed
 */
long valueToSignificantPlaces(unsigned long value, bool negative);

inline float tcFltAbs(float f1) {
    return f1 > 0.0F ? f1 : -f1;
}

// There now follows pretty much internal code, boolean negative for dealing with program memory
// never use direct program memory commands, always prefer these, it allows us
// to compile it out much easier.

/**
 * This is always safe to call, if there's a string that's in program mem on AVR
 * but not on other boards, this always does the right thing.
 * @param dst the destination buffer
 * @param pgmSrc the source to be copied (program mem on AVR)
 * @param size the size of dst.
 */
uint8_t safeProgCpy(char* dst, const char* pgmSrc, uint8_t size);

// for AVR only definitions
#ifdef __AVR__
#include <avr/pgmspace.h>
#define get_info_callback(x) ((MenuCallbackFn)pgm_read_ptr_near(x))
#define TCMENU_DEFINED_PLATFORM PLATFORM_ARDUINO_8BIT
#endif

// for ESP only definitions
#ifdef ESP_H
#include <pgmspace.h>
#define get_info_callback(x) ((MenuCallbackFn)(*x))
#define TCMENU_DEFINED_PLATFORM PLATFORM_ARDUINO_32BIT
#endif

// for things that are the same between AVR and ESP
#if (defined __AVR__ || defined ESP_H) && !defined __MBED__
#define PGM_TCM PROGMEM
extern char szGlobalBuffer[];
inline char* potentialProgramMemory(const char *x) {
    safeProgCpy(szGlobalBuffer, x, 16);
    return szGlobalBuffer;
}
#define get_info_char(x) ((char) pgm_read_byte_near(x)) 
#define get_info_int(x) ((int16_t)pgm_read_word_near(x))
#define get_info_uint(x) ((unsigned int)pgm_read_word_near(x))
#define safeProgStrLen(x) (strlen_P(x))
#else 
#define TCMENU_DEFINED_PLATFORM PLATFORM_ARDUINO_32BIT
#define PGM_TCM
#define potentialProgramMemory(x) (x)
#define get_info_char(x) ((char)(*x)) 
#define get_info_int(x) ((int16_t)(*x))
#define get_info_uint(x) ((unsigned int)(*x))
#define get_info_callback(x) ((MenuCallbackFn)(*x))
#define safeProgStrLen(x) (strlen(x))
#endif

#endif
