/**
 * This presents an amplifier control panel onto an ESP32. The example assumes an ESP32 with an ILI9341 screen driven
 * using TFT_eSPI, a resistive touch screen and WiFi in STA mode.
 *
 * YPOS_PIN 32
 * XNEG_PIN 33
 * XPOS_PIN 2
 * YNEG_PIN 0
 *
 * Screen setup for 320x240 LANDSCAPE.
 */

#include <Arduino.h>
#include "esp32Amplifier_menu.h"
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <ArduinoEEPROMAbstraction.h>
#include <graphics/MenuTouchScreenEncoder.h>
#include <BaseDialog.h>
#include <tcMenuVersion.h>
#include "AmplifierController.h"
#include "app_icondata.h"
#include "TestingDialogController.h"
#include <extras/DrawableTouchCalibrator.h>

using namespace tcremote;

const char pgmVersionHeader[] PROGMEM = "tcMenu Version";

bool connectedToWifi = false;
TitleWidget wifiWidget(iconsWifi, 5, 16, 12, nullptr);
AmplifierController controller;

const char* simFilesForList[] = { "SuperFile1.txt",  "CustomFile.cpp", "SuperDuper.h", "File303.cpp", "File123443.h",
                                  "Another.file", "TestMe.txt", "Program.txt" };
#define SIM_FILES_ARRAY_SIZE 8

void prepareWifiForUse();

void setup() {
    SPI.setFrequency(20000000);
    SPI.begin();
    Serial.begin(115200);
    EEPROM.begin(512);

    renderer.setFirstWidget(&wifiWidget);

    setupMenu();

    menuMgr.load(MENU_MAGIC_KEY, [] {
        // when the eeprom is not initialised, put sensible defaults in there.
        menuMgr.getEepromAbstraction()->writeArrayToRom(150, reinterpret_cast<const uint8_t *>(pgmDefaultChannelNames), sizeof pgmDefaultChannelNames);
        // At this point during setup, things are not initialised so use the silent version of the menu set methods.
        // Otherwise, you'll potentially be calling code that isn't initialised.
        menuVolume.setCurrentValue(20, true);
        menuDirect.setBoolean(true, true);
    });
    prepareWifiForUse();

    controller.initialise();
    touchScreen.calibrateMinMaxValues(0.250F, 0.890F, 0.09F, 0.88F);

    renderer.setCustomDrawingHandler(new tcextras::TouchScreenCalibrator(&touchScreen, &renderer));

    // first we get the graphics factory
    auto & factory = renderer.getGraphicsPropertiesFactory();

    // now we add the icons that we want to use with certain menu items
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);
    factory.addImageToCache(DrawableIcon(menuSettings.getId(), iconSize, DrawableIcon::ICON_XBITMAP, settingsIcon40Bits));
    factory.addImageToCache(DrawableIcon(menuStatus.getId(), iconSize, DrawableIcon::ICON_XBITMAP, statusIcon40Bits));
    factory.addImageToCache(DrawableIcon(menuMute.getId(), iconSize, DrawableIcon::ICON_XBITMAP, muteOffIcon40Bits, muteOnIcon40Bits));

    // and now we define that row 3 of the main menu will have three columns, drawn as icons
    factory.addGridPosition(&menuSettings, GridPosition(GridPosition::DRAW_AS_ICON_ONLY,
                                                        GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 1, 4, 49));
    factory.addGridPosition(&menuStatus, GridPosition(GridPosition::DRAW_AS_ICON_ONLY,
                                                      GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 2, 4, 49));
    factory.addGridPosition(&menuMute, GridPosition(GridPosition::DRAW_AS_ICON_ONLY,
                                                    GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 3, 4, 49));

    // here is how we completely redefine the drawing of a specific item, you can also define for submenu or default
    color_t specialPalette[] { ILI9341_WHITE, ILI9341_RED, ILI9341_BLACK, ILI9341_BLUE};
    factory.setDrawingPropertiesAllInSub(ItemDisplayProperties::COMPTYPE_TITLE, menuStatus.getId(), specialPalette,
                                        MenuPadding(4), nullptr, 4, 10, 36,
                                        GridPosition::JUSTIFY_CENTER_WITH_VALUE , MenuBorder(2));
    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();

    setTitlePressedCallback([](int) {
        BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
        if(!dlg || dlg->isInUse()) return;
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
        char sz[25];
        tccore::copyTcMenuVersion(sz, sizeof sz);
        dlg->copyIntoBuffer(sz);
        dlg->show(pgmVersionHeader, false);
    });

    menuStatusDataList.setNumberOfRows(SIM_FILES_ARRAY_SIZE);

    // If your app relies on getting the callbacks after a menuMgr.load(..) has finished then this does the callbacks
    triggerAllChangedCallbacks();
}

void powerDownCapture() {
    menuMgr.save(MENU_MAGIC_KEY);
    EEPROM.commit();
}

void prepareWifiForUse() {
    // this sketch assumes you've successfully connected to the Wifi before, does not
    // call begin.. You can initialise the wifi whichever way you wish here.
    if(strlen(menuConnectivitySSID.getTextValue())==0) {
        // no SSID come up as an access point
        WiFi.mode(WIFI_AP);
        WiFi.softAP("tcmenu", "secret");
        serdebugF("Started up in AP mode, connect with 'tcmenu' and 'secret'");
    }
    else {
        WiFi.begin(menuConnectivitySSID.getTextValue(), menuConnectivityPasscode.getTextValue());
        WiFi.mode(WIFI_STA);
        serdebugF("Connecting to Wifi using settings from connectivity menu");
    }

    // now monitor the wifi level every few seconds and report it in a widget.
    taskManager.scheduleFixedRate(3000, [] {
        if(WiFiClass::status() == WL_CONNECTED) {
            if(!connectedToWifi) {
                IPAddress localIp = WiFi.localIP();
                Serial.print("Now connected to WiFi");
                Serial.println(localIp);
                menuConnectivityIPAddress.setIpAddress(localIp[0], localIp[1], localIp[2], localIp[3]);
                connectedToWifi = true;
            }
            wifiWidget.setCurrentState(fromWiFiRSSITo4StateIndicator(WiFi.RSSI()));
        }
        else {
            connectedToWifi = false;
            wifiWidget.setCurrentState(0);
        }
    });
}

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onVolumeChanged(int id) {
    controller.onVolumeChanged();
}

void CALLBACK_FUNCTION onChannelChanged(int id) {
    controller.onChannelChanged();
}

void CALLBACK_FUNCTION onAudioDirect(int id) {
    controller.onAudioDirect(menuDirect.getBoolean());
}

void CALLBACK_FUNCTION onMuteSound(int id) {
    controller.onMute(menuMute.getBoolean());
}

void CALLBACK_FUNCTION onShowDialogs(int id) {
    showDialogs();
}

int CALLBACK_FUNCTION fnStatusDataListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    if(row > SIM_FILES_ARRAY_SIZE && row != LIST_PARENT_ITEM_POS) {
        // you wouldn't need this in a production build, it is for our debugging to do bounds checking.
        serdebugF2("picked row > max!! ", row);
        return false;
    }
   switch(mode) {
    case RENDERFN_INVOKE:
        if(row != LIST_PARENT_ITEM_POS && renderer.getDialog() && !renderer.getDialog()->isInUse()) {
            renderer.getDialog()->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
            renderer.getDialog()->showRam("List select", false);
            char sz[10];
            ltoaClrBuff(sz, row, 3, NOT_PADDED, sizeof sz);
            renderer.getDialog()->copyIntoBuffer(sz);
        }
        return true;
    case RENDERFN_NAME:
        if(row == LIST_PARENT_ITEM_POS) {
            strcpy(buffer, "Sim Files");
        }
        else{
            ltoaClrBuff(buffer, row, 3, NOT_PADDED, bufferSize);
        }
        return true;
    case RENDERFN_VALUE:
        if(row == LIST_PARENT_ITEM_POS) {
            strcpy(buffer, ">>");
        }
        else {
            strncpy(buffer, simFilesForList[row], bufferSize);
            buffer[bufferSize - 1] = 0;
        }
        return true;
    case RENDERFN_EEPROM_POS: return 0xffff; // lists are generally not saved to EEPROM
    default: return false;
    }
}

int CALLBACK_FUNCTION fnChannelSettingsChannelRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE: {
        auto *eeprom = menuMgr.getEepromAbstraction();
        auto romPos = menuChannels.getEepromStart() + (row * menuChannels.getItemWidth());
        eeprom->readIntoMemArray((uint8_t *) menuChannelSettingsName.getTextValue(), romPos, menuChannels.getItemWidth());
        menuChannelSettingsLevelTrim.setCurrentValue(controller.getTrim(row));
        return true;
    }
    case RENDERFN_NAME:
        strcpy(buffer, "Channel");
        return true;
    case RENDERFN_VALUE:
        strcpy(buffer, "Line");
        fastltoa(buffer, row + 1, 2, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xFFFF; // lists are generally not saved to EEPROM
    default: return false;
    }
}

void CALLBACK_FUNCTION onChannelSetttingsUpdate(int id) {
    auto *eeprom = menuMgr.getEepromAbstraction();
    auto romPos = menuChannels.getEepromStart() + (menuChannelSettingsChannel.getCurrentValue() * menuChannels.getItemWidth());
    eeprom->writeArrayToRom(romPos, (uint8_t *) menuChannelSettingsName.getTextValue(), menuChannels.getItemWidth());
    controller.setTrim(menuChannelSettingsChannel.getCurrentValue(), menuChannelSettingsLevelTrim.getCurrentValue());
    if(renderer.getDialog() && !renderer.getDialog()->isInUse()) {
        renderer.getDialog()->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
        renderer.getDialog()->showRam("Channel setting saved", true);
        char sz[5];
        ltoaClrBuff(sz, menuChannelSettingsChannel.getCurrentValue(), 2, NOT_PADDED, sizeof sz);
        renderer.getDialog()->copyIntoBuffer(sz);
    }
}

void CALLBACK_FUNCTION onSaveSettings(int id) {
    menuMgr.save();
    EEPROM.commit();

    if(renderer.getDialog() && !renderer.getDialog()->isInUse()) {
        renderer.getDialog()->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
        renderer.getDialog()->showRam("Committed all items", true);
    }
}
