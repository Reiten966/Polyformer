/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#if !defined(_ARDUNIO_EEPROM_ABS_H) && !defined(__MBED__)
#define _ARDUNIO_EEPROM_ABS_H

#include <Arduino.h>
#include "EEPROM.h"
#include "EepromAbstraction.h"

/**
 * @file ArduinoEEPROMAbstraction.h
 * A wrapper around the Arduino EEPROM support.
 */

/**
 * Provides a wrapper around the EEPROM class available on some Arduino boards. For AVR 8bit boards
 * such as Uno and Mega, there are no notes, it neatly wraps the class.
 * 
 * Extra notes for ESP8266, ESP32 and any other FLASH emulated implementation:
 * When the ESP EEPROM wrapper is used, then you need to ensure that you call begin(size) before use and
 * commit() before shutting down, otherwise changes will not be saved. Be very aware that you must not
 * commit too frequently; otherwise your board's FLASH will be damaged. I recommend the very cheap i2c
 * AT24Cxxx EEPROM devices which cost about $1 and you will not risk damaging your FLASH. Me having 
 * implemented this is not an indicator that I agree with using FLASH as EEPROM, I personally wouldn't
 * do that on a production board.
 */
class ArduinoEEPROMAbstraction : public EepromAbstraction {
private:
    EEPROMClass* eepromProxy;
public:
    ArduinoEEPROMAbstraction(EEPROMClass* proxy) {
        this->eepromProxy = proxy;
    }

   	uint8_t read8(EepromPosition position) override {
        return eepromProxy->read(position);
    }

   	uint16_t read16(EepromPosition pos) override {
        return eepromProxy->read(pos + 1) << 8 | eepromProxy->read(pos);
    }

   	uint32_t read32(EepromPosition pos) override {
        return (uint32_t)eepromProxy->read(pos + 3) << 24 | (uint32_t)eepromProxy->read(pos + 2) << 16 |
               (uint32_t)eepromProxy->read(pos + 1) << 8 | (uint32_t)eepromProxy->read(pos + 0);
    }

    void write8(EepromPosition pos, uint8_t val) override {
        eepromProxy->write(pos, val);
    }

    void write16(EepromPosition pos, uint16_t val) override {
        eepromProxy->write(pos, val);
        val >>= 8;
        eepromProxy->write(pos + 1, (uint8_t)val);
    }

   	void write32(EepromPosition pos, uint32_t val) override {
        eepromProxy->write(pos, (uint8_t)val);
        val >>= 8;
        eepromProxy->write(pos + 1, (uint8_t)val);
        val >>= 8;
        eepromProxy->write(pos + 2, (uint8_t)val);
        val >>= 8;
        eepromProxy->write(pos + 3, (uint8_t)val);
    }

	void readIntoMemArray(uint8_t* memDest, EepromPosition romSrc, uint8_t len) override {
        for(int i=0;i<len;i++) {
            *memDest = eepromProxy->read(romSrc + i);
            memDest++;
        }
    }

	void writeArrayToRom(EepromPosition romDest, const uint8_t* memSrc, uint8_t len) override {
        for(int i=0;i<len;i++) {
            eepromProxy->write(romDest + i, *memSrc);
            memSrc++;
        }
    }
};

#endif // !defined(_ARDUNIO_EEPROM_ABS_H) && !defined(_NO_EEPROM_CLASS_)
