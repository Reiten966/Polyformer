#ifndef _TCMENU_FIXTURES_H_
#define _TCMENU_FIXTURES_H_

#include "tcUtil.h"
#include "MenuItems.h"
#include "RuntimeMenuItem.h"

const PGM_TCM AnalogMenuInfo minfoNumTwoDp = { "TwoDpNum", 23, 32, 30000, NO_CALLBACK, 0, 100, "" };
AnalogMenuItem menuNumTwoDp(&minfoNumTwoDp, 0, NULL);

const PGM_TCM AnalogMenuInfo minfoNumHalvesOffs = { "Halves", 22, 30, 255, NO_CALLBACK, -100, 2, "dB" };
AnalogMenuItem menuHalvesOffs(&minfoNumHalvesOffs, 0, &menuNumTwoDp);


RENDERING_CALLBACK_NAME_INVOKE(fnIpAddrRenderFn, ipAddressRenderFn, "Ip Addr", 22, NULL)
IpAddressMenuItem menuIpAddr(fnIpAddrRenderFn, 99, &menuHalvesOffs);

const PGM_TCM AnalogMenuInfo minfoAnalogSub = { "SubAnalog", 10, 20, 255, NO_CALLBACK, 0, 1, "SU" };
AnalogMenuItem menuSubAnalog(&minfoAnalogSub, 0, &menuIpAddr);

RENDERING_CALLBACK_NAME_INVOKE(backSubFixturesFn, backSubItemRenderFn, "Settings", 0xffff, NULL)
BackMenuItem menuBackSub(backSubFixturesFn, &menuSubAnalog);
const PGM_TCM SubMenuInfo minfoSub = { "Settings", 7, 0xffff, 0, NO_CALLBACK };
SubMenuItem menuSub(&minfoSub, &menuBackSub, NULL);

const PGM_TCM AnalogMenuInfo minfoAnalog = { "Analog", 1, 2, 255, NO_CALLBACK, 0, 1, "AB" };
AnalogMenuItem menuAnalog(&minfoAnalog, 0, &menuSub);

const PGM_TCM AnalogMenuInfo minfoAnalog2 = { "Analog2", 2, 4, 100, NO_CALLBACK, -10, 10, "" };
AnalogMenuItem menuAnalog2(&minfoAnalog2, 0, &menuAnalog);

const char enumStrItem1[] PGM_TCM = "ITEM1";
const char enumStrItem2[] PGM_TCM = "ITEM2";
const char enumStrItem3[] PGM_TCM = "ITEM3";
const char* const enumStrPort2Dir[] PGM_TCM  = { enumStrItem1, enumStrItem2, enumStrItem3 };
const PGM_TCM EnumMenuInfo minfoEnum1 = { "Enum1", 3, 6, 2, NO_CALLBACK , enumStrPort2Dir };
EnumMenuItem menuEnum1(&minfoEnum1, false, &menuAnalog2);

int idOfCallback = 0;
void testCallback(int id) {
	idOfCallback = id;
}

const PGM_TCM BooleanMenuInfo boolMenu1 = {"Bool1", 4, 8, 1, testCallback, NAMING_TRUE_FALSE };
BooleanMenuItem boolItem1(&boolMenu1, false, &menuEnum1);

RENDERING_CALLBACK_NAME_INVOKE(textMenuItem1Callback, textItemRenderFn, "Text1", 9, NULL)
TextMenuItem textMenuItem1(textMenuItem1Callback, 5, 10, &boolItem1);

#endif // defined