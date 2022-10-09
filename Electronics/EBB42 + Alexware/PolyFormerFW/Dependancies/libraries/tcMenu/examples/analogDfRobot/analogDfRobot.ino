/**
 * This example assumes you've got an Uno / MEGA with the DF robot board. It uses a the switches that are connected to
 * analog 0 and tries to keep as much in line with the DF robot spec as possible.
 * See the readme file for more info.
 */
#include "analogDfRobot_menu.h"
#include "AnalogDeviceAbstraction.h"

ArduinoAnalogDevice analog;

//using namespace tcgfx;

void setup() {
    // first we setup the menu
    setupMenu();

    // we are going to toggle the built in LED, so set it as output.
    pinMode(LED_BUILTIN, OUTPUT);
    analog.initPin(A1, DIR_IN);

    // now we read the value of A0 every 200millis and set it onto a menu item
    taskManager.scheduleFixedRate(200, [] {
        menuValueA0.setCurrentValue(int(analog.getCurrentFloat(A1) * 100.0));
    });

    // This registers a special callback, unlike the usual one, this callback is
    // only called when editing is completely finished, you register it once and
    // it triggers for every menu item.    
    menuMgr.setItemCommittedHook([](int id) {
        menuCommits.setCurrentValue(menuCommits.getCurrentValue() + 1);
    });
}

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onLed1(int id) {
    digitalWrite(LED_BUILTIN, menuL1.getBoolean());
}

void CALLBACK_FUNCTION onLed2(int id) {
    // TODO: write your own second LED function..
    // Called whenever you change the LED2 menu item..
}


// When dealing with custom rendering, either using lists or custom choices, we have to do everything from scratch. We need
// to handle the EEPROM storage location, the item name, the value etc.
// see tcMenu list documentation on thecoderscorner.com
int CALLBACK_FUNCTION fnChooseItemRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        // we don't want to do anything on a selection, ignore it.
        return true;
    case RENDERFN_NAME:
        // here we return the name for the scroll item        
        strcpy(buffer, "Choose");
        return true;
    case RENDERFN_VALUE:
        // here we define the text value for each possible setting, we just set it to V followed by the row
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 18; // Choices are saved to rom, we copy the value from the designer. 
    default: return false;
    }
}
