/**
 * ESP8266 / ESP32 WiFi and OLED example. Works out of the box with Heltek Kits 8 & 32.
 * 
 * This example shows a very basic home automation system, in this case we simulate a greenhouse
 * growing tomatoes and cucumbers. We can check the temperature of the areas, check if windows
 * are opened, open the window, configure the heater and see if it's on.
 * 
 * It is not a full working automation project, but rather designed to show how easily such a thing
 * could be achieved with TcMenu.
 * 
 * The circuit uses a PCF8574 with both the input via rotary encoder and the outputs for the heater
 * and window opener driven by it too.
 */

#include <U8g2lib.h>
#include <Wire.h>
#include "esp8266WifiOled_menu.h"
#include <IoAbstractionWire.h>
#include <ArduinoEEPROMAbstraction.h>
#include <RemoteAuthentication.h>
#include <RemoteMenuItem.h>
#include <MockIoAbstraction.h>
#include <ESP8266WiFi.h>
#include <EepromItemStorage.h>
#include "appicons.h"

// contains the graphical widget title components.
#include "stockIcons/wifiAndConnectionIcons16x12.h"

char fileChoicesArray[255]{};

// here we define the heater and window pins on the PCF8574
// 0, 1 are A and B from encoder, 2 is OK button.
#define WINDOW_PIN 3
#define HEATER_PIN 4
#define MENU_WIFIMODE_STATION 0

// we add two widgets that show both the connection status and wifi signal strength
// these are added to the renderer and rendered upon any change.
// https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#titlewidget-for-presenting-state-in-icon-form
TitleWidget connectedWidget(iconsConnection, 2, 16, 12);
TitleWidget wifiWidget(iconsWifi, 5, 16, 12, &connectedWidget);

// state used by the sketch

bool connectedToWifi = false;

// when there's a change in communication status (client connects for example) this gets called.
void onCommsChange(CommunicationInfo info) {
    if(info.remoteNo == 0) {
        connectedWidget.setCurrentState(info.connected ? 1 : 0);
    }
    // this relies on logging in IoAbstraction's ioLogging.h, to turn it on visit the file for instructions.
    serdebugF4("Comms notify (rNo, con, enum)", info.remoteNo, info.connected, info.errorMode);
}

void startWiFi() {
    // You can choose between station and access point mode by setting the connectivity/Wifi Mode option to your
    // own choice
    if(menuWiFiMode.getCurrentValue() == MENU_WIFIMODE_STATION) {
        // we are in station mode
        WiFi.begin(menuSSID.getTextValue(), menuPwd.getTextValue());
        WiFi.mode(WIFI_STA);
        serdebugF("Connecting to Wifi using settings from connectivity menu");
    }
    else {
        // we are in access point mode
        WiFi.mode(WIFI_AP);
        char ssid[25];
        char pwd[25];
        copyMenuItemValueDefault(&menuSSID, ssid, sizeof ssid, "tcmenu");
        copyMenuItemValueDefault(&menuPwd, pwd, sizeof pwd, "secret");
        WiFi.softAP(ssid, pwd);
        serdebugF3("Started up in AP mode, connect with ", ssid, pwd);
    }

}

//
// here we just start serial for debugging and try to initialise the display and menu
//
void setup() {
    Serial.begin(115200);
    serdebugF("Starting NodeMCU example");
    Wire.begin();
    //Wire.setClock(400000);
    EEPROM.begin(512); // set up the inbuilt ESP rom to use for load and store.

    // here we set the first widget that will be displayed in the title area.
    renderer.setFirstWidget(&wifiWidget);

    // initialise the menu.
    setupMenu();

    // load back the core menu items
    menuMgr.load();

    startWiFi();
    menuIoTMonitor.registerCommsNotification(onCommsChange);

    // now we add three icons into the icon cache, we should add one for each menu item that we give a grid position
    // that draws as an image.
    auto& props = static_cast<ConfigurableItemDisplayPropertiesFactory&>(renderer.getDisplayPropertiesFactory());
    props.addImageToCache(DrawableIcon(menuElectricHeater.getId(), Coord(APPICON_HEAT_WIDTH, APPICON_HEAT_HEIGHT), DrawableIcon::ICON_XBITMAP, appIconHeatOff, appIconHeatOn));
    props.addImageToCache(DrawableIcon(menuLockDoor.getId(), Coord(APPICON_LOCK_WIDTH, APPICON_LOCK_HEIGHT), DrawableIcon::ICON_XBITMAP, appIconLockOpen, appIconLockClosed));
    props.addImageToCache(DrawableIcon(menuSetup.getId(), Coord(APPICON_SETTINGS_WIDTH, APPICON_SETTINGS_HEIGHT), DrawableIcon::ICON_XBITMAP, appIconSettings));

    // here we override the drawing grid for some menu items, we tell the renderer to draw as icons in a three position
    // grid, with a fixed height.
    props.addGridPosition(&menuElectricHeater, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE,3, 2, 3, 26));
    props.addGridPosition(&menuLockDoor, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 3, 3, 26));
    props.addGridPosition(&menuSetup, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 1, 3, 26));

    // after changing the drawing properties, always refresh the cache to ensure it draws properly.
    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();

    // now monitor the wifi level every second and report it in a widget.
    taskManager.scheduleFixedRate(1000, [] {
        if(WiFi.status() == WL_CONNECTED) {
            if(!connectedToWifi) {
                IPAddress localIp = WiFi.localIP();
                Serial.print("Now connected to WiFi");
                Serial.println(localIp);
                menuIpAddress.setIpAddress(localIp[0], localIp[1], localIp[2], localIp[3]);
                connectedToWifi = true;
            }
            wifiWidget.setCurrentState(fromWiFiRSSITo4StateIndicator(WiFi.RSSI()));
        }
        else {
            connectedToWifi = false;
            wifiWidget.setCurrentState(0);
        }
    });

    //
    // here we simulate the temperature changing.
    // temperature doesn't change that often, even 5 seconds is probably too short.
    // in a normal system you'd probably do something other than call random..
    //
    taskManager.scheduleFixedRate(5000, [] {
        menuTomatoTemp.setCurrentValue(random(255));
        menuCucumberTemp.setCurrentValue(random(255));        
    });

    //
    // Now we configure our simulated heater and window.. We use inverse
    // logic on the 8574 because it's better at pulling down to GND than up.
    //
    ioDevicePinMode(ioexp_io8574, WINDOW_PIN, OUTPUT);
    ioDevicePinMode(ioexp_io8574, HEATER_PIN, OUTPUT);
    ioDeviceDigitalWrite(ioexp_io8574, WINDOW_PIN, HIGH);
    ioDeviceDigitalWriteS(ioexp_io8574, HEATER_PIN, HIGH);
}

//
// as with all apps based off IoAbstraction's taskManager we must call loop
// very frequently. On ESP8266 it's absolutely essential that you don't do
// anything here that takes a long time as the board will get reset by it's
// internal watchdog.
//
void loop() {
    taskManager.runLoop();
}


// used by the below function to store state.
bool windowOpen = false;

//
// here we have the function that is called initially when the window is open
// and then repeatedly called by rescheduling itself until the window is closed
//
void windowOpenFn() {
    if(menuWinOpening.getCurrentValue() > 0) {
        windowOpen = !windowOpen;
        // 0 is narrow opening, 1 is wide. We simulate this by adjusting the speed of the call.
        int windowDelay = menuWinOpening.getCurrentValue() == 0 ? 500 : 250;
        serdebugF2("Setting window delay to ", windowDelay);
        taskManager.scheduleOnce(windowDelay, windowOpenFn);
    }
    else windowOpen = false;

    ioDeviceDigitalWriteS(ioexp_io8574, WINDOW_PIN, windowOpen);
}

// we are using a simulated low speed PWM to control the heater
// store it in a global variable
bool heaterOn;

void heaterOnFn() {
    if(menuElectricHeater.getBoolean()) {
        heaterOn = !heaterOn;
        // the power is low medium or high. We simulate by changing the delay
        int heaterDelay = min(500, (menuHeaterPower.getCurrentValue() + 1) * 500);
        serdebugF2("Setting heater delay to ", heaterDelay);
        taskManager.scheduleOnce(heaterDelay, heaterOnFn);
    }
    else heaterOn = false;

    ioDeviceDigitalWriteS(ioexp_io8574, HEATER_PIN, heaterOn);
}

//
// Below are the call back methods that are executed when a menu item changes.
//

const char heaterOnHighWarningPgm[] PROGMEM = "Set heater to";

//
// this method is called when the dialog is dismissed.
//
void onDialogFinished(ButtonType btnPressed, void* /*userdata*/) {
    // if user did not accept the change, set back to the first setting.
    if(btnPressed != BTNTYPE_OK) {
        menuHeaterPower.setCurrentValue(1);
    }
}

//
// Called when the heater power is changed, we draw a dialog when there is a 
// change that is greater that 1. The dialog is configured to call the method
// above when dismissed.
//
void CALLBACK_FUNCTION onHeaterPower(int /*id*/) {
    Serial.print("Heater power setting changed to ");
    int pwr = menuHeaterPower.getCurrentValue();
    Serial.println(pwr);
    if(pwr >= 1) {
        BaseDialog* dlg = renderer.getDialog();
        if(dlg) {
            dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL, 1);
            dlg->show(heaterOnHighWarningPgm, true, onDialogFinished); // true = shows on remote sessions.
            char sz[10];
            menuHeaterPower.copyEnumStrToBuffer(sz, sizeof(sz), pwr);
            dlg->copyIntoBuffer(sz);
        }
    }
}

void CALLBACK_FUNCTION onWindowOpening(int /*id*/) {
    uint16_t opening = menuWinOpening.getCurrentValue();
    Serial.print("Window setting changed");
    Serial.println(opening > 0 ? "Open" : "Closed");
    if(opening > 0) {
        windowOpenFn();
    }
}

void CALLBACK_FUNCTION onElectricHeater(int /*id*/) {
    Serial.print("Electric heater ");
    Serial.println(menuElectricHeater.getBoolean() ? "ON" : "OFF");
    if(menuElectricHeater.getBoolean()) {
        heaterOnFn();
    }
}

const char allSavedPgm[] PROGMEM = "All saved to EEPROM";
void CALLBACK_FUNCTION onSaveAll(int id) {
    Serial.println("Saving values to EEPROM");
    menuMgr.save();
    // on esp you must commit after calling save.
    EEPROM.commit();

    BaseDialog* dlg = renderer.getDialog();
    if(dlg) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dlg->show(allSavedPgm, false); // false = shows only on device
        dlg->copyIntoBuffer("Committed to FLASH");
    }

}

void CALLBACK_FUNCTION onFileChoice(int id) {
    Serial.print("file chosen: ");
    auto entryNum = menuFile.getCurrentValue();
    char sz[12];
    menuFile.valueAtPosition(sz, sizeof(sz), entryNum);
    Serial.println(sz);
}

void CALLBACK_FUNCTION onLoadFiles(int id) {
    strcpy(fileChoicesArray, "MyDocument");
    strcpy(&fileChoicesArray[10], "SecretFile");
    strcpy(&fileChoicesArray[20], "AnotherFle");
    strcpy(&fileChoicesArray[30], "MenuItem");
    strcpy(&fileChoicesArray[40], "MyDocument");
    menuFile.setNumberOfRows(5);
}

void CALLBACK_FUNCTION onLockDoor(int id) {
    // Here we'd actually handle the door lock
}
