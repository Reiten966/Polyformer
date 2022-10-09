#line 2 "remoteControlTests.ino"

#include <AUnit.h>
#include <tcMenu.h>

using namespace aunit;

// A small menu structure with a submenu.

const PROGMEM BooleanMenuInfo minfo12VStandby = { "12V Standby", 4, 0xffff, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menu12VStandby(&minfo12VStandby, false, nullptr);
RENDERING_CALLBACK_NAME_INVOKE(backSubSettingsFn, backSubItemRenderFn, "Settings", 0xffff, NULL)
BackMenuItem menuBackSettings(backSubSettingsFn, &menu12VStandby);
const PROGMEM SubMenuInfo minfoSettings = { "Settings", 3, 0xffff, 0, NO_CALLBACK };
SubMenuItem menuSettings(&minfoSettings, &menuBackSettings, nullptr);
const char enumStrChannel_0[] PROGMEM = "CD Player";
const char enumStrChannel_1[] PROGMEM = "Turntable";
const char enumStrChannel_2[] PROGMEM = "Computer";
const char* const enumStrChannel[] PROGMEM = { enumStrChannel_0, enumStrChannel_1, enumStrChannel_2 };
const PROGMEM EnumMenuInfo minfoChannel = { "Channel", 2, 4, 2, NO_CALLBACK, enumStrChannel };
EnumMenuItem menuChannel(&minfoChannel, 0, &menuSettings);
const PROGMEM AnalogMenuInfo minfoVolume = { "Volume", 1, 2, 255, NO_CALLBACK, -190, 2, "dB" };
AnalogMenuItem menuVolume(&minfoVolume, 0, &menuChannel);

NoRenderer noRenderer;

void setup() {
    Serial.begin(115200);
    while(!Serial);

    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    //TestRunner::exclude("*");
    //TestRunner::include("testPromoteWebSocket");

}

void loop() {
    TestRunner::run();
}
