#include "esp32Lcd_menu.h"

//                   0         1         2         3
//                   0123456789012345678901234567890123456789
char* ramChoices = "Pot NoodleTakeaways FishNchipsPepperoni  ";

//
// Creating a grid layout just for a specific menu item. The flags menu under additional is laid out in a grid format,
// where the menu items are presented two per row.
//
void prepareLayout() {
    auto& factory = renderer.getLcdDisplayPropertiesFactory();
    // we now create a grid for the two led controls in the submenu, this shows how to override the default grid for a few items.
    // In most cases the automatic rendering works fine when a few items are overriden as long as the row you request hasn't yet been taken.
    factory.addGridPosition(&menuGridLED1,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, 2, 1, 2, 1));
    factory.addGridPosition(&menuGridLED2,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_RIGHT_WITH_VALUE, 2, 2, 2, 1));
    factory.addGridPosition(&menuGridUp,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_LEFT_NO_VALUE, 2, 1, 3, 1));
    factory.addGridPosition(&menuGridDown,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_RIGHT_NO_VALUE, 2, 2, 3, 1));
}

void setup() {
    Serial.begin(115200);
    Wire.begin(4, 15);

    // define the number of rows in the list, lists really don't like 0 rows much.
    menuMyList.setNumberOfRows(5);

    // prepare the grid layout for our grid example
    prepareLayout();

    // now call setupMenu to prepare the menu system.
    setupMenu();
}

void loop() {
    taskManager.runLoop();
}


// see tcMenu list documentation on thecoderscorner.com
int CALLBACK_FUNCTION fnMyListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        // TODO - your code to invoke goes here - row is the index of the item
        return true;
    case RENDERFN_NAME:
        // TODO - each row has it's own name - 0xff is the parent item
        ltoaClrBuff(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_VALUE:
        // TODO - each row can has its own value - 0xff is the parent item
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xffff; // lists are generally not saved to EEPROM
    default: return false;
    }
}

void CALLBACK_FUNCTION onGridLed2(int id) {
}

void CALLBACK_FUNCTION onGridLed1(int id) {
}

void CALLBACK_FUNCTION onGridDown(int id) {
    menuGridFloatValue.setFloatValue(menuGridFloatValue.getFloatValue() - 0.333F);
}

void CALLBACK_FUNCTION onGridUp(int id) {
    menuGridFloatValue.setFloatValue(menuGridFloatValue.getFloatValue() + 0.333F);
}
