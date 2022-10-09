/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _WIFI_AND_CONNECTION_ICONS_8X7
#define _WIFI_AND_CONNECTION_ICONS_8X7

#include <PlatformDetermination.h>

/**
 * @file wifiAndConnectionIcons8x8.h
 * 
 * Contains definitions for standard icons including the connection active icon and the wifi signal strength icon.
 * Icons in this file are all 8 x 8 pixels for low res displays.
 */


const uint8_t iconConnectionNone[] PROGMEM = { 0x7e, 0xab, 0xd5, 0xab, 0xd5, 0xab, 0xd5, 0x7e };

const uint8_t iconConnected[] PROGMEM = { 0x7e, 0x81, 0x81, 0x81, 0x81, 0x81, 0x99, 0x7e };

const uint8_t iconWifiNone[] PROGMEM = { 0x00, 0x3e, 0x41, 0x41, 0x22, 0x22, 0x14, 0x08 };

const uint8_t iconWifiLow[] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 };

const uint8_t iconWifiMed[] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x22, 0x08 };

const uint8_t iconWifiGood[] PROGMEM = { 0x00, 0x00, 0x00, 0x3e, 0x41, 0x1c, 0x22, 0x08 };

const uint8_t iconWifiStrong[] PROGMEM = { 0x00, 0x3e, 0x41, 0x3e, 0x41, 0x1c, 0x22, 0x08 };



/** 
 * Defines a set of 5 icons for wifi, not connected and then various signal strength. 0 is no connection, 4 is good signal.
 * Usually used with a TitleWidget on a low resolution display. the icons are 8 x 8 pixels.
 */
const uint8_t* const iconsWifi[] PROGMEM = { iconWifiNone, iconWifiLow, iconWifiMed, iconWifiGood, iconWifiStrong };

/** 
 * Defines a set of 2 icons for connection active, a boolean state of either active (1) or not active (0).
 * Usually used with a TitleWidget on a low resolution display. the icons are 8 x 8 pixels.
 */
const uint8_t* const iconsConnection[] PROGMEM = { iconConnectionNone, iconConnected };

#endif //_WIFI_AND_CONNECTION_ICONS_8X7