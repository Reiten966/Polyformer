/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _WIFI_STOCK_CONNECTION_LCD_H
#define _WIFI_STOCK_CONNECTION_LCD_H

#include <PlatformDetermination.h>

/**
 * @file wifiAndConnectionIconLCD.h
 * 
 * Contains definitions for standard icons including the connection active icon and the wifi signal strength icon.
 * Icons in this file are for use on LCD display and are arranged to show as custom characters.
 */

const uint8_t iconWifiNotConnected[8] PROGMEM = {
    0b00000,
    0b01110,
    0b10001,
    0b10001,
    0b01010,
    0b01010,
    0b00100,
    0    
};

const uint8_t iconWifiLowSignal[8] PROGMEM = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00100,
    0    
};

const uint8_t iconWifiMedSignal[8] PROGMEM = {
    0b00000,
    0b00000,
    0b00000,
    0b00100,
    0b01010,
    0b00000,
    0b00100,
    0    
};

const uint8_t iconWifiStrongSignal[8] PROGMEM = {
    0b00000,
    0b01110,
    0b10001,
    0b00100,
    0b01010,
    0b00000,
    0b00100,
    0    
};

const uint8_t iconWifiBestSignal[8] PROGMEM = {
    0b01110,
    0b10001,
    0b01110,
    0b10001,
    0b00100,
    0b01010,
    0b00100,
    0    
};

const uint8_t iconConnected[8] PROGMEM = {
    0b00001110,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00011111,
    0b00001110,
    0
};

const uint8_t iconDisconnected[8] PROGMEM = {
    0b00011111,
    0b00010011,
    0b00010011,
    0b00010101,
    0b00010101,
    0b00011001,
    0b00011111,
    0
};

/** 
 * Defines a set of 5 icons for wifi, not connected and then various signal strength. 0 is no connection, 4 is good signal.
 * Usually used with a TitleWidget on LCD display units, note that each entry will take a custom character (5 in this case)
 */
const uint8_t* const iconsWifi[] PROGMEM = { iconWifiNotConnected, iconWifiLowSignal, iconWifiMedSignal, iconWifiStrongSignal, iconWifiBestSignal };

/** 
 * Defines a set of 2 icons for connection active, a boolean state of either active (1) or not active (0).
 * Usually used with a TitleWidget on LCD display units, note that each entry will take a custom character (2 in this case)
 */
const uint8_t* const iconsConnection[] PROGMEM = { iconDisconnected, iconConnected };

#endif //_WIFI_STOCK_CONNECTION_LCD_H