#include "colorTftEthernet32_menu.h"
#include <IoAbstractionWire.h>
#include <EepromAbstractionWire.h>
#include <AnalogDeviceAbstraction.h>
#include <RemoteAuthentication.h>
#include <RemoteMenuItem.h>
#include <Ethernet.h>
#include <SPI.h>
#include <IoLogging.h>
#include "ColorEtherenetCustomDraw.h"
#include <tcMenuVersion.h>

// contains the graphical widget title components.
#include "stockIcons/wifiAndConnectionIcons16x12.h"

/*
 * Shows how to use adafruit graphics with a TFT panel and an ethernet module.
 * This is a 32 bit example, which by default targets 32 bit devices.
 * Assumed board for this is a SAMD based MKR board.
 * 
 * For more details see the README.md file in this directory.
 */

// we are going to allow control of the menu over local area network
// so therefore must configure ethernet..
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// and we create an analog device with enhanced range because we are using a 32bit board.
ArduinoAnalogDevice analogDevice(12, 10);

// We add a title widget that shows when a user is connected to the device. Connection icons
// are in the standard icon set we included at the top.
TitleWidget connectedWidget(iconsConnection, 2, 16, 12);

// We also want to take over the display whenever the screen becomes idle, here we use the CustomDrawing
// technique described in the documentation. See ColorEthernetCustomDraw.h
ColorEthernetCustomDraw myCustomDraw;

// Here we declare a string in const/PROGMEM for use with a dialog later. Note that this works even on ARM
// boards that don't have separated program memory.
const char pgmHeaderSavedItem[] PROGMEM = "Rom Item Saved";

// Start 2nd Encoder
// Here, we use a second rotary encoder to adjust one of the menu items, the button toggles a boolean item
// Here are the fields for the second rotary encoder
// If you only want one encoder you can comment these fields out.
const int encoder2Click = 0;
const int encoder2APin = 2;
const int encoder2BPin = 3;
HardwareRotaryEncoder *secondEncoder;
const int ledPin = 1;
// End 2nd Encoder

// when there's a change in communication status (connection or disconnection for example) this gets called.
// see further down in the code where we add this to the remote IoT monitor.
void onCommsChange(CommunicationInfo info) {
    if(info.remoteNo == 0U) {
        connectedWidget.setCurrentState(info.connected ? 1 : 0);
    }
    // this relies on logging in IoAbstraction's ioLogging.h, to turn it on visit the file for instructions.
    serdebugF4("Comms notify (rNo, con, enum)", info.remoteNo, info.connected, info.errorMode);
}

void setup() {
    // we used an i2c device (io8574) so must initialise wire too
    Wire.begin();
    Serial.begin(115200);

    // here we make all menu items except menu voltage wrap around. Wrap means that when the value hits maximum it
    // goes back to 0, and when it hits 0 it goes back to maximum. Default is all wrapping off.
    menuMgr.setUseWrapAroundEncoder(true);
    menuMgr.addEncoderWrapOverride(menuVoltage, false);

    // Often the easiest way to add a comms listener, is to add it to an IoT monitor menu item as we show
    // here, as it aggregates all the connection information together.
    menuIoTMonitor.registerCommsNotification(onCommsChange);

    // and set up the dac on the 32 bit board.
    analogDevice.initPin(A0, DIR_OUT);
    analogDevice.initPin(A1, DIR_IN);

    menuShowHidden.setBoolean(false);

    // here we make the menuitem "hidden item" invisible. It will not be displayed.
    // here it is done before setupMenu is called, so there's no need to refresh the
    // display. If it's done after initialisation, the menu must be reset by calling
    // menuMgr.setCurrentMenu(getRoot());
    menuHiddenItem.setVisible(false);

    // set up the widget to appear in the title.
    renderer.setFirstWidget(&connectedWidget);

    // set up the menu
    setupMenu();

    // increase SPI speed
    gfx.setSPISpeed(10000000UL);

    // and then load back the previous state
    menuMgr.load();

    // spin up the Ethernet library
    byte* rawIp = menuIpAddress.getIpAddress();
    IPAddress ipAddr(rawIp[0], rawIp[1], rawIp[2], rawIp[3]);
    Ethernet.begin(mac, ipAddr);

    char sz[20];
    menuIpAddress.copyValue(sz, sizeof(sz));
    Serial.print("Ethernet available on ");Serial.println(sz);

    taskManager.scheduleFixedRate(250, [] {
        float a1Value = analogDevice.getCurrentFloat(A1);
        menuVoltA1.setFloatValue(a1Value * 3.3F);
    });

    // register the custom drawing handler
    renderer.setCustomDrawingHandler(&myCustomDraw);

    ioDevicePinMode(switches.getIoAbstraction(), ledPin, OUTPUT);

    setTitlePressedCallback([](int id) {
        withMenuDialogIfAvailable([](MenuBasedDialog* dlg) {
            dlg->setButtons(BTNTYPE_CLOSE, BTNTYPE_NONE);
            dlg->showRam("ARM Example", false);
            char menuVer[10];
            tccore::copyTcMenuVersion(menuVer, sizeof menuVer);
            dlg->copyIntoBuffer(menuVer);
        });
    });

    // Start 2nd encoder
    Serial.println("Setting up second encoder now");

    // here we want the encoder set to the range of values that menuCurrent takes, this is just for example,
    // and you could set the range to anything that is required. We use the callback to update the menu item.
    secondEncoder = new HardwareRotaryEncoder(encoder2APin, encoder2BPin, [](int encoderValue) {
        menuCurrent.setCurrentValue(encoderValue);
    });
    secondEncoder->changePrecision(menuCurrent.getMaximumValue(), menuCurrent.getCurrentValue(), true);
    switches.setEncoder(1, secondEncoder); // put it into the 2nd available encoder slot.

    // now, when the additional encoder button is released, we toggle the state of menuPwrDelay.
    // it will repeat at 25 * 20 millis before acceleration kicks in.
    switches.addSwitch(encoder2Click, [](pinid_t pin, bool held) {
        menuPwrDelay.setBoolean(!menuPwrDelay.getBoolean());
        ioDeviceDigitalWriteS(switches.getIoAbstraction(), ledPin, menuPwrDelay.getBoolean());
    }, 25);
    // End 2nd encoder
}

void loop() {
    taskManager.runLoop();
}

void writeToDac() {
    float volts = menuVoltage.getAsFloatingPointValue();
    float curr = menuCurrent.getAsFloatingPointValue();
    
    float total = (volts / 64.0F) * (curr / 2.0F);
    analogDevice.setCurrentFloat(A0, total);
    menuVoltA0.setFloatValue(total);
}

void CALLBACK_FUNCTION onVoltageChange(int /*id*/) {
    writeToDac();
}

void CALLBACK_FUNCTION onCurrentChange(int /*id*/) {
    writeToDac();
}

void CALLBACK_FUNCTION onLimitMode(int /*id*/) {
    // TODO - your menu change code
}

void CALLBACK_FUNCTION onSaveRom(int /*id*/) {
    // save out the state, in a real system we could detect power down for this.
    menuMgr.save();
}

void CALLBACK_FUNCTION onTakeDisplay(int /*id*/) {
    renderer.takeOverDisplay();
}

void CALLBACK_FUNCTION onRgbChanged(int /*id*/) {
    auto colorData = menuRGB.getColorData();
    serdebugF4("RGB changed: ", colorData.red, colorData.green, colorData.blue);
}

void CALLBACK_FUNCTION onSaveItem(int /*id*/) {
    auto itemNo = menuRomLocation.getCurrentValue();
    auto itemSize = menuRomChoice.getItemWidth();
    auto position = menuRomChoice.getEepromStart() + (itemNo * itemSize);
    menuMgr.getEepromAbstraction()->writeArrayToRom(position, (const uint8_t*)menuRomText.getTextValue(), 10);

    if(renderer.getDialog()->isInUse()) return;
    renderer.getDialog()->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
    renderer.getDialog()->show(pgmHeaderSavedItem, true);
    renderer.getDialog()->copyIntoBuffer(menuRomText.getTextValue());
}

void CALLBACK_FUNCTION onRomLocationChange(int /*id*/) {
    auto itemNo = menuRomLocation.getCurrentValue();
    char sz[12];
    auto itemSize = menuRomChoice.getItemWidth();
    menuMgr.getEepromAbstraction()->readIntoMemArray((uint8_t*)sz, menuRomChoice.getEepromStart() + (itemNo * itemSize), 10);
    menuRomText.setTextValue(sz);
    serdebugF2("Rom data was ", sz)
}

//
// Here we handle the custom rendering for the runtime list menu item that just counts. We basically get called back
// every time it needs more data. For example when the name is required, when any index value is required or when
// it will be saved to EEPROM or invoked.
//
int CALLBACK_FUNCTION fnRomLocationRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        onRomLocationChange(int(item->getId()));
        return true;
    case RENDERFN_NAME:
        // TODO - each row has it's own name - 0xff is the parent item
        strncpy(buffer, "Rom Location", bufferSize);
        return true;
    case RENDERFN_VALUE:
        // TODO - each row can has its own value - 0xff is the parent item
        strncpy(buffer, "Item ", bufferSize);
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xFFFF; // lists are generally not saved to EEPROM
    default: return false;
    }
}

void CALLBACK_FUNCTION onShowHidden(int id) {
    menuHiddenItem.setVisible(menuShowHidden.getBoolean());
    menuMgr.notifyStructureChanged();
}
