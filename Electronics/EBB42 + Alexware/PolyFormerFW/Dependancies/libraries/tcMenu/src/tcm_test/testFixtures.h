/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TESTFIXTURES_H_
#define _TESTFIXTURES_H_

#include <PlatformDetermination.h>
#include "../tcMenu.h"
#include "../RuntimeMenuItem.h"

/**
 * @file testFixtures.h 
 * 
 * A full complete menu tree structure suitable for basic unit testing. Contains most types.
 * Also contains a simpler menu for testing too.
 */

//
// Complete structure containing most types:
// root: menuVolume
//

#ifndef PRESSMECALLBACK
#define PRESSMECALLBACK NULL
#endif // PRESSMECALLBACK

const PROGMEM AnalogMenuInfo minfoCaseTemp = { "Case Temp", 103, 0xffff, 255, NO_CALLBACK, 0, 2, "C" };
AnalogMenuItem menuCaseTemp(&minfoCaseTemp, 0, NULL);

const PROGMEM FloatMenuInfo minfoFloatItem = { "FloatItem", 102, 0xffff, 4, NO_CALLBACK };
FloatMenuItem menuFloatItem(&minfoFloatItem, NULL);
const PROGMEM AnyMenuInfo minfoPressMe = { "Press Me", 101, 0xffff, 0, PRESSMECALLBACK };
ActionMenuItem menuPressMe(&minfoPressMe, &menuFloatItem);

RENDERING_CALLBACK_NAME_INVOKE(backSubSecondLevelFn, backSubItemRenderFn, "Second Level", 0xffff, NULL)
BackMenuItem menuBackSecondLevel(backSubSecondLevelFn, &menuPressMe);
const PROGMEM SubMenuInfo minfoSecondLevel = { "SecondLevel", 100, 0xffff, 0, NO_CALLBACK };
SubMenuItem menuSecondLevel(&minfoSecondLevel, &menuBackSecondLevel, &menuCaseTemp);

const PROGMEM AnalogMenuInfo minfoRHSTemp = { "R HS Temp", 8, 0xffff, 255, NO_CALLBACK, 0, 2, "C" };
AnalogMenuItem menuRHSTemp(&minfoRHSTemp, 0, &menuSecondLevel);
const PROGMEM AnalogMenuInfo minfoLHSTemp = { "L HS Temp", 7, 0xffff, 255, NO_CALLBACK, 0, 2, "C" };
AnalogMenuItem menuLHSTemp(&minfoLHSTemp, 0, &menuRHSTemp);
RENDERING_CALLBACK_NAME_INVOKE(backSubStatusFn, backSubItemRenderFn, "Status", 0xffff, NULL)
BackMenuItem menuBackStatus(backSubStatusFn, &menuLHSTemp);
const PROGMEM SubMenuInfo minfoStatus = { "Status", 5, 0xffff, 0, NO_CALLBACK };
SubMenuItem menuStatus(&minfoStatus, &menuBackStatus, NULL);
const PROGMEM AnalogMenuInfo minfoContrast = { "Contrast", 10, 6, 255, NO_CALLBACK, 0, 2, "" };
AnalogMenuItem menuContrast(&minfoContrast, 0, NULL);
const PROGMEM BooleanMenuInfo minfo12VStandby = { "12V Standby", 4, 0xffff, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menu12VStandby(&minfo12VStandby, false, &menuContrast);
RENDERING_CALLBACK_NAME_INVOKE(backSubSettingsFn, backSubItemRenderFn, "Settings", 0xffff, NULL)
BackMenuItem menuBackSettings(backSubSettingsFn, &menu12VStandby);
const PROGMEM SubMenuInfo minfoSettings = { "Settings", 3, 0xffff, 0, NO_CALLBACK };
SubMenuItem menuSettings(&minfoSettings, &menuBackSettings, &menuStatus);
const char enumStrChannel_0[] PROGMEM = "CD Player";
const char enumStrChannel_1[] PROGMEM = "Turntable";
const char enumStrChannel_2[] PROGMEM = "Computer";
const char* const enumStrChannel[] PROGMEM = { enumStrChannel_0, enumStrChannel_1, enumStrChannel_2 };
const PROGMEM EnumMenuInfo minfoChannel = { "Channel", 2, 4, 2, NO_CALLBACK, enumStrChannel };
EnumMenuItem menuChannel(&minfoChannel, 0, &menuSettings);
const PROGMEM AnalogMenuInfo minfoVolume = { "Volume", 1, 2, 255, NO_CALLBACK, -190, 2, "dB" };
AnalogMenuItem menuVolume(&minfoVolume, 0, &menuChannel);


//
// A second very simple menu with no submenus and only two analog items.
// root: menuSimple1
//

const PROGMEM AnalogMenuInfo minfoSimple2 = { "simple2", 1, 2, 255, NO_CALLBACK, -190, 2, "dB" };
AnalogMenuItem menuSimple2(&minfoSimple2, 0, NULL);
const PROGMEM AnalogMenuInfo minfoSimple1 = { "simple1", 1, 2, 255, NO_CALLBACK, -190, 2, "dB" };
AnalogMenuItem menuSimple1(&minfoSimple1, 0, &menuSimple2);

#endif //_TESTFIXTURES_H_
