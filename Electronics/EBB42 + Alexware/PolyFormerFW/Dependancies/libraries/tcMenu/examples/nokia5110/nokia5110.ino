/*
 * Shows how to use adagraphics with a mono buffered panel and an ethernet
 * module.This is an 8 bit example, which by default targets the mega 2560.
 * For ethernet control it uses UIPEthernet library instead of Ethernet2.
 * Be careful using UIP in production boards as it is GPL. Fine for small
 * home projects. TcMenu has an Apache license but UIP does not. Prefer
 * Ethernet2 for production boards as it is LGPL.
 * 
 * For more details see the README.md file in this directory.
 */

#include "nokia5110_menu.h"
#include <Adafruit_PCD8544.h>
#include <EepromAbstraction.h>
#include <UIPEthernet.h>
#include <RemoteAuthentication.h>
#include <RemoteMenuItem.h>
#include <stockIcons/wifiAndConnectionIcons8x7.h>

// you can turn on and off tcmenu logging in the below file (within IoAbstraction)
#include <IoLogging.h>

// a few forward references
void addWidgetToTitleArea();

// we are going to allow control of the menu over local area network
// so therefore must configure ethernet..
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xF0, 0x0D
};

// used the the dialog further down, dialog headers are always from progmem / constant.
const char warningPgm[] PROGMEM = "Warning!";

void setup() {
    // start serial, second line for hardware usb on 32 bit boards.
    while(!Serial);
    Serial.begin(115200);

    addWidgetToTitleArea();

    // initialise the menu
    setupMenu();

    // we can load the menu back from eeprom, the second parameter is an
    // optional override of the magic key. This key is saved out with the
    // menu, and the values are only loaded when the key matches.
    menuMgr.load(0xd00d);

    // spin up the Ethernet library, get the IP address from the menu
    byte* rawIp = menuIP.getIpAddress();
    IPAddress ip(rawIp[0], rawIp[1], rawIp[2], rawIp[3]);
    Ethernet.begin(mac, ip);

    // because we are using the simple adafruit configuration option, tcMenu will set up the display for us.
    // it's less configurable but supports most of the popular choices. You can tweak the settings after setupMenu().
    gfx.setContrast(60);

    // and print out the IP address
    char sz[20];
    menuIP.copyValue(sz, sizeof(sz));
    Serial.print("Device IP is: "); Serial.println(sz);

    taskManager.scheduleFixedRate(500, [] {
        // here we simulate a changing field, where we are monitoring the voltage and current
        menuVoltsIn.setFloatValue(240.0 + (float(random(100) / 100.0)));
        menuCurrent.setFloatValue(0.5 + (float(random(100) / 100.0)));

        // every now and again we pop up a dialog randomly
        // it simulates a zone being triggered
        if((random(1000) < 5)) {
            BaseDialog* dlg = renderer.getDialog();
            dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
            dlg->show(warningPgm, true);
            char sz[20];
            strcpy(sz, "Zone");
            fastltoa(sz, random(6), 1, NOT_PADDED, sizeof(sz));
            strcat(sz, " trigger");
            dlg->copyIntoBuffer(sz);
        }
    });
}

//
// If you had a power down detection circuit on your board you'd call this when
// the power down scenario started. Notice we use the same key as load.
// https://www.thecoderscorner.com/electronics/microcontrollers/psu-control/detecting-power-loss-in-powersupply/ 
//
void CALLBACK_FUNCTION onPowerDownDetected(int) {
    menuMgr.save(0xd00d);
}

void loop() {
    // all sketches using task manager must call this very frequently and
    // never use delay(). See IoAbstraction.
    taskManager.runLoop();
    
    // When using the UIP ethernet driver we must call maintain frequently
    Ethernet.maintain();
}

//
// These three methods are called back when the menu changes, see in the designer
// where we define these menu items.
//

void CALLBACK_FUNCTION onHallLight(int /*id*/) {
    Serial.print("Hall light is now ");
    Serial.println(menuHall.getCurrentValue());
}

void CALLBACK_FUNCTION onLivingRoomLight(int /*id*/) {
    Serial.print("Living Room light is now ");
    Serial.println(menuLiving.getCurrentValue());
}

void CALLBACK_FUNCTION onKitchenLight(int /*id*/) {
    Serial.print("Kitchen light is now ");
    Serial.println(menuKitchen.getCurrentValue());
}

//
// Here we define a widget that is rendered in the title area, it shows if there is
// a remote connection. First we declare the bitmaps, then the widget, and then
// put the widget in the renderer.
//

TitleWidget connectedWidget(iconsConnection, 2, 8, 7);

//
// below we ask the remote connector to inform us of any changes in connection state. This is
// the callback we pass.
//
void onCommsChange(CommunicationInfo info) {
    if(info.remoteNo == 0) {
        connectedWidget.setCurrentState(info.connected ? 1 : 0);
    }
    // this relies on logging in IoAbstraction's ioLogging.h, to turn it on visit the file for instructions.
    serdebugF4("Comms notify (rNo, con, enum)", info.remoteNo, info.connected, info.errorMode);
}

void addWidgetToTitleArea() {
    menuIoTMonitor.registerCommsNotification(onCommsChange);

    // and give the renderer our widget.
    renderer.setFirstWidget(&connectedWidget);
}
