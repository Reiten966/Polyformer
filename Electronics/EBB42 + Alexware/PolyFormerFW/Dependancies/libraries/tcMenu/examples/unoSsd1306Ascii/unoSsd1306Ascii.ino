#include "unoSsd1306Ascii_menu.h"
// default CPP main file for sketch
#include <PlatformDetermination.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#include <TaskManagerIO.h>

//
// The below display configuration is more or less copied from the example, you can adjust as needed
//

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
SSD1306AsciiAvrI2c gfx;

void setup() {
    Serial.begin(115200);

    // during setup and before calling setupMenu() you need to initialise the display
#if RST_PIN >= 0
    gfx.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
    gfx.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

    menuIntList.setNumberOfRows(10);

    setupMenu();
}

void loop() {
    taskManager.runLoop();
}

// here we declare a program memory string for later use.
const char pgmListItemName[] PROGMEM = "My List";

//
// This callback is called when the list item needs data, it is called for the items name, values and upon item press
// see tcMenu list documentation on thecoderscorner.com
//
int CALLBACK_FUNCTION fnIntListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        Serial.print("Select "); Serial.println(row);
        return true;
    case RENDERFN_NAME:
        // TODO - each row has it's own name - LIST_PARENT_ITEM_POS is the parent item
        if(row == LIST_PARENT_ITEM_POS) {
            strcpy_P(buffer, pgmListItemName);
        } else {
            ltoaClrBuff(buffer, row, 2, NOT_PADDED, bufferSize);
        }
        return true;
    case RENDERFN_VALUE:
        // TODO - each row can has its own value - LIST_PARENT_ITEM_POS is the parent item
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xffff; // lists are generally not saved to EEPROM
    default: return false;
    }
}
