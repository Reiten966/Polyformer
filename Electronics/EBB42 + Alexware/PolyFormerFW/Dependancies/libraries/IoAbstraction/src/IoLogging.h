#ifndef _IO_LOGGING_H_
#define _IO_LOGGING_H_

/**
 * @file IoLogging.h
 *
 * Some very basic logging utilities for any IoAbstraction user that log to a chosen serial interface. Turned on
 * by un-commenting the define. Should NOT be used in production.
 */

#include "PlatformDetermination.h"

// START user adjustable section.

// the below definition controls logging, enable logging by either defining this build flag
// or uncommenting the line below.
//#define IO_LOGGING_DEBUG

// These are the default levels that will be enabled when logging starts, you can add them at
// runtime using serEnableLevel(level, true/false)
#ifndef IO_LOGGING_DEFAULT_LEVEL
#define IO_LOGGING_DEFAULT_LEVEL (SER_WARNING|SER_ERROR|SER_IOA_INFO|SER_TCMENU_INFO|SER_NETWORK_INFO|SER_DEBUG|SER_USER_1)
#endif

// END user adjustable section.

/**
 * This enumeration contains all the available logging levels, each logging level is a bit in the structure, and
 * we assume there can be up to 15 levels. With 4 user levels available.
 */
enum SerLoggingLevel {
    SER_WARNING = 0x0001,
    SER_ERROR = 0x0002,
    SER_DEBUG = 0x0004,
    SER_TCMENU_INFO = 0x0008,
    SER_NETWORK_INFO = 0x0010,
    SER_IOA_INFO = 0x0020,
    SER_USER_1 = 0x0040,
    SER_USER_2 = 0x0080,
    SER_USER_3 = 0x0100,
    SER_USER_4 = 0x0200,
    SER_USER_5 = 0x0400,
    SER_USER_6 = 0x0800,
    SER_TCMENU_DEBUG = 0x1000,
    SER_NETWORK_DEBUG = 0x2000,
    SER_IOA_DEBUG = 0x4000,
    SER_LOG_EVERYTHING = 0xffff
};

#ifdef IO_LOGGING_DEBUG

/** This uint16_t stores the enabled logging levels, don't use directly */
extern unsigned int enabledLevels;

#ifdef IOA_USE_MBED

#include "PrintCompat.h"
#include <FileHandle.h>
//
// On mbed you create an instance of this class called LoggingPort in your main class.
// see the mbed example.
//
class MBedLogger : public Print {
private:
    FileHandle& serial;
public:
    MBedLogger(FileHandle& serialName) : serial(serialName) {}

    size_t write(uint8_t ch) override {
        serial.write(&ch, 1);
        return 1;
    }

    size_t write(const char* sz) override {
        auto len = strlen(sz);
        serial.write(sz, len);
        return len;
    }
};
extern MBedLogger LoggingPort;
// a couple of definitions here to avoid including headers, F() macro not needed on mbed
unsigned long millis();
#define F(x) x
#else

// Arduino:
// You can change the logging serial port by defining LoggingPort to your chosen serial port.
#ifndef LoggingPort
#define LoggingPort Serial
#endif
#endif

const char* prettyLevel(SerLoggingLevel level);
#define logTimeAndLevel(title, lvl) LoggingPort.print(millis());LoggingPort.print('-');LoggingPort.print(prettyLevel(lvl));LoggingPort.print(':');LoggingPort.print(title)

/**
 * Check if a level is enabled
 * @param level the level to check
 * @return true if enabled, otherwise false
 */
inline bool serLevelEnabled(SerLoggingLevel level) { return (enabledLevels & level) != 0; }

/**
 * Turn on or off logging for a particular level
 * @param level the level to turn on or off
 * @param active true if it is to be turned on, otherwise false
 */
inline void serEnableLevel(SerLoggingLevel level, bool active) {
    if(active) {
        enabledLevels |= level;
    } else {
        enabledLevels ^= level;
    }
}

#define serlogF(lvl, x) if(serLevelEnabled(lvl)) { logTimeAndLevel(F(x), lvl); LoggingPort.println(); }
#define serlogF2(lvl, x1, x2) if(serLevelEnabled(lvl)) { logTimeAndLevel(F(x1), lvl); LoggingPort.print(x2);LoggingPort.println(); }
#define serlogF3(lvl, x1, x2, x3) if(serLevelEnabled(lvl)) { logTimeAndLevel(F(x1), lvl); LoggingPort.print(x2); LoggingPort.print(' '); LoggingPort.print(x3);LoggingPort.println(); }
#define serlogF4(lvl, x1, x2, x3, x4) if(serLevelEnabled(lvl)) { logTimeAndLevel(F(x1), lvl); LoggingPort.print(x2); LoggingPort.print(' '); LoggingPort.print(x3); LoggingPort.print(' '); LoggingPort.print(x4);LoggingPort.println(); }
#define serlogFHex(lvl, x1, x2) if(serLevelEnabled(lvl)) { logTimeAndLevel(F(x1), lvl); LoggingPort.print(x2, HEX);LoggingPort.println(); }
#define serlogFHex2(lvl, x1, x2, x3) if(serLevelEnabled(lvl)) { logTimeAndLevel(F(x1), lvl); LoggingPort.print(x2, HEX); LoggingPort.print(','); LoggingPort.print(x3, HEX);LoggingPort.println(); }
#define serlog(lvl, x) if(serLevelEnabled(lvl)) { logTimeAndLevel(x, lvl);LoggingPort.println(); }
#define serlog2(lvl, x1, x2) if(serLevelEnabled(lvl)) { logTimeAndLevel(x1, lvl); LoggingPort.print(x2);LoggingPort.println(); }
#define serlog3(lvl, x1, x2, x3) if(serLevelEnabled(lvl)) { logTimeAndLevel(x1, lvl); LoggingPort.print(x2); LoggingPort.print(' '); LoggingPort.print(x3);LoggingPort.println(); }
#define serlogHex(lvl, x1, x2) if(serLevelEnabled(lvl)) { logTimeAndLevel(x1, lvl); LoggingPort.print(x2, HEX);LoggingPort.println(); }

void serlogHexDump(SerLoggingLevel level, const char *title, const void* data, size_t strlen);
inline void serdebugHexDump(const char *title, const void* data, size_t len) { serlogHexDump(SER_DEBUG, title, data, len);}

#define serdebugF(x) serlogF(SER_DEBUG, x)
#define serdebugF2(x1, x2) serlogF2(SER_DEBUG, x1, x2);
#define serdebugF3(x1, x2, x3) serlogF3(SER_DEBUG, x1, x2, x3)
#define serdebugF4(x1, x2, x3, x4) serlogF4(SER_DEBUG, x1, x2, x3, x4)
#define serdebugFHex(x1, x2) serlogFHex(SER_DEBUG, x1, x2)
#define serdebugFHex2(x1, x2, x3) serlogFHex2(SER_DEBUG, x1, x2, x3)
#define serdebug(x) serlog(SER_DEBUG, x)
#define serdebug2(x1, x2) serlog2(SER_DEBUG, x1, x2)
#define serdebug3(x1, x2, x3) serlog3(SER_DEBUG, x1, x2, x3)
#define serdebugHex(x1, x2) serlogHex(SER_DEBUG, x1, x2)

#else
// all loging to no operations (commenting out the above define of IO_LOGGING_DEBUG to remove in production builds).
#define serdebugF(x) 
#define serdebugF2(x, y) 
#define serdebugF3(x, y, z) 
#define serdebugF4(a, b, c, d) 
#define serdebugFHex(x, y) 
#define serdebugFHex2(x, y, z) 
#define serdebug(x) 
#define serdebug2(x, y) 
#define serdebug3(x, y, z) 
#define serdebugHex(x, y) 
#define serdebugHexDump(x, str, strlen)
#define serlogHexDump(l, x, str, len)
#define serlogF(lvl, x)
#define serlogF2(lvl, x1, x2)
#define serlogF3(lvl, x1, x2, x3)
#define serlogF4(lvl, x1, x2, x3, x4)
#define serlogFHex(lvl, x1, x2)
#define serlogFHex2(lvl, x1, x2, x3)
#define serlog(lvl, x)
#define serlog2(lvl, x1, x2)
#define serlog3(lvl, x1, x2, x3)
#define serlogHex(lvl, x1, x2)

#endif // IO_LOGGING_DEBUG

#endif // _IO_LOGGING_H_
