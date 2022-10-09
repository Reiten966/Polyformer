/**
 * ESP8266 example of the *simplest* possible menu on u8g2.
 * 
 * This example shows about the most basic example possible and should be useful for anyone
 * trying to get started with either adafruit graphics or u8g2. It is missing title widgets,
 * remote capabilities,  and many other things but makes for the simplest possible starting 
 * point for a graphical build.
 * 
 */

#include "simpleU8g2_menu.h"
#include <Wire.h>
#include <IoAbstractionWire.h>

// this is the interrupt pin connection from the PCF8574 back to the ESP8266 board.
#define IO_INTERRUPT_PIN 12

const char pgmCommittedToRom[] PROGMEM = "Saved to ROM";

void setup() {
    // If you use i2c and serial devices, be sure to start wire / serial.
    Wire.begin();
    Serial.begin(115200);

    // here we initialise the EEPROM class to 512 bytes of storage
    // don't commit often to this, it's in FLASH
    EEPROM.begin(512);

    // This is added by tcMenu Designer automatically during the first setup.
    setupMenu();

    // lastly we load state from EEPROM.
    menuMgr.load();
}

//
// In any IoAbstraction based application you'll normally use tasks via taskManager
// instead of writing code in loop. You are free to write code here as long as it
// does not delay or block execution. Otherwise task manager will be blocked. You
// can easily integrate low power libraries with task manager, see the examples in
// the TaskManagerIO library.
//
void loop() {
    taskManager.runLoop();
}

//
// this is the callback function that we declared in the designer for action
// "Start Toasting". This will be called when the action is performed. Notice
// instead of using callbacks for every toaster setting, we just get the value
// associated with the menu item directly.
//
void CALLBACK_FUNCTION onStartToasting(int id) {
    Serial.println("Let's start toasting");
    Serial.print("Power:  "); Serial.println(menuToasterPower.getCurrentValue());
    Serial.print("Type:   "); Serial.println(menuType.getCurrentValue());
    Serial.print("Frozen: "); Serial.println(menuFrozen.getCurrentValue());
}

//
// Called when the name field been has changed
//
void CALLBACK_FUNCTION onNameChanged(int id) {
    Serial.print("Name changed to ");
    Serial.println(menuSettingsUserName.getTextValue());
}

//
// This is attached to the save action on settings, in a real system we may have a
// low voltage detector or other solution for saving.
//
void CALLBACK_FUNCTION onSaveSettings(int id) {
  menuMgr.save();
  EEPROM.commit();

  // here is a brief example of how to show a dialog, usually for information
  // or yes/no answers.
  auto* dlg = renderer.getDialog();
  if(dlg && !dlg->isInUse()) {
    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
    dlg->show(pgmCommittedToRom, false);
    dlg->copyIntoBuffer("just so you know");
  }
}
