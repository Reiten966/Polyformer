/**
 * This script demonstrates the use of a matrix keyboard with a rotary encoder also attached, this allows for the
 * best of both worlds where the encoder is used for rotational type editing, and the keyboard where it is appropriate.
 * This sketch is setup for MEGA2560 AVR with a 20x4 LCD on I2C and also an encoder on I2C. It also assumes an I2C ROM.
 * However, you can take the ideas from this sketch and apply them in your own designs.
 *
 * More information about matrix keyboard support:
 * https://www.thecoderscorner.com/products/arduino-libraries/io-abstraction/matrix-keyboard-keypad-manager/

 */
#include "keyboardEthernetShield_menu.h"
#include <IoAbstractionWire.h>
#include <RemoteAuthentication.h>
#include <RemoteMenuItem.h>
#include <KeyboardManager.h>
#include <tcMenuKeyboard.h>
#include <EepromAbstraction.h>
#include <Ethernet.h>
#include <IoLogging.h>

using namespace tcremote;

// Set up ethernet, the usual default settings are chosen. Change to your preferred values or use DHCP.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};

//
// Some constant character definitions that we use around the code
//
const char pgmListPressed[] PROGMEM = "List Item Pressed";
const char pgmHeaderSavedItem[] PROGMEM = "Saved Item";
const char * romSpaceNames = "item 01item 02item 03item 04item 05item 06item 07item 08item 09item 10 ";

//
// Creating a grid layout just for a specific menu item. The flags menu under additional is laid out in a grid format,
// where the menu items are presented two per row.
//
void prepareLayout() {
    auto& factory = renderer.getLcdDisplayPropertiesFactory();
    // we now create a grid for the two led controls in the submenu, this shows how to override the default grid for a few items.
    // In most cases the automatic rendering works fine when a few items are overriden as long as the row you request hasn't yet been taken.
    factory.addGridPosition(&menuAdditionalBoolFlagFlag1,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, 2, 1, 1, 1));
    factory.addGridPosition(&menuAdditionalBoolFlagFlag2,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_RIGHT_WITH_VALUE, 2, 2, 1, 1));
    factory.addGridPosition(&menuAdditionalBoolFlagFlag3,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, 2, 1, 2, 1));
    factory.addGridPosition(&menuAdditionalBoolFlagFlag4,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_RIGHT_WITH_VALUE, 2, 2, 2, 1));
}

void setup() {
	//
	// If you are using serial (connectivity or logging) and wire they must be initialised 
	// before their first use.
	//
	Serial.begin(115200);
	Wire.begin();
    Wire.setClock(400000);
    lcd.setDelayTime(0, 20);

    serEnableLevel(SER_TCMENU_DEBUG, true);

    // now we turn off the title and change the editor characters
    renderer.setTitleRequired(false);
    
    // Here we set the character to be used for back, next and editing for the "cursor".
    renderer.setEditorChars(0b01111111, 0b01111110, '=');

	setupMenu();

	auto* authenticator = reinterpret_cast<EepromAuthenticatorManager*>(menuMgr.getAuthenticator());

	// Here you could have a button internal to the device somewhere that reset the pins and remotes, in this case
	// if the button is held at start up it trigger a key reset.
    if(ioDeviceDigitalReadS(ioexp_io23017, 5) == LOW) {
        Serial.println("Resetting all keys and pin");
        authenticator->resetAllKeys();
    }

	// here we use the EEPROM to load back the last set of values.
	menuMgr.load(0xf8f3);

	// and print out the IP address
	char sz[20];
	menuConnectivityIpAddress.copyValue(sz, sizeof(sz));
	Serial.print("Device IP is: "); Serial.println(sz);

	// spin up the Ethernet library, get the IP address from the menu
	byte* rawIp = menuConnectivityIpAddress.getIpAddress();
	IPAddress ip(rawIp[0], rawIp[1], rawIp[2], rawIp[3]);
	Ethernet.begin(mac, ip);

    // copy the pin from the authenticator into the change pin field.
    // and make it a password field so characters are not visible unless edited.
    authenticator->copyPinToBuffer(sz, sizeof(sz));
    menuConnectivityChangePin.setTextValue(sz);
    menuConnectivityChangePin.setPasswordField(true);

    menuLargeNum.getLargeNumber()->setFromFloat(1234.567);

    prepareLayout();
}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION onFiths(int /*id*/) {
	Serial.println("Fiths changed");
}


void CALLBACK_FUNCTION onInteger(int /*id*/) {
	Serial.println("Integer changed");
}


void CALLBACK_FUNCTION onAnalog1(int /*id*/) {
	Serial.println("Analog1 changed");
}


const char pgmSavedText[] PROGMEM = "Saved all setttings";

void CALLBACK_FUNCTION onSaveToEeprom(int /*id*/) {
	menuMgr.save(0xf8f3);
	auto dlg = renderer.getDialog();
	if(dlg && !dlg->isInUse()) {
	    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
	    dlg->show(pgmSavedText, true, nullptr);
	    dlg->copyIntoBuffer("");
	}
}

const char pgmPinTooShort[] PROGMEM = "Pin too short";

void CALLBACK_FUNCTION onChangePin(int /*id*/) {
    // Here we check if the pin that's just been entered is too short.
    // Diallowing setting and showing a dialog if it is.
    const char* newPin = menuConnectivityChangePin.getTextValue();
    if(strlen(newPin) < 4) {
        BaseDialog* dlg = renderer.getDialog();
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dlg->show(pgmPinTooShort, false);
        dlg->copyIntoBuffer(newPin);
    }
    else {
        auto* authenticator = reinterpret_cast<EepromAuthenticatorManager*>(menuMgr.getAuthenticator());
        authenticator->changePin(newPin);
    }
}

void CALLBACK_FUNCTION onItemChange(int id) {
    auto itemNo = menuRomChoicesItemNum.getCurrentValue();
    char sz[12];
    auto itemSize = menuAdditionalRomChoice.getItemWidth();
    menuMgr.getEepromAbstraction()->readIntoMemArray((uint8_t*)sz, menuAdditionalRomChoice.getEepromStart() + (itemNo * itemSize), 10);
    menuRomChoicesValue.setTextValue(sz);
}


void CALLBACK_FUNCTION onSaveValue(int id) {
    auto itemNo = menuRomChoicesItemNum.getCurrentValue();
    auto itemSize = menuAdditionalRomChoice.getItemWidth();
    auto position = menuAdditionalRomChoice.getEepromStart() + (itemNo * itemSize);
    menuMgr.getEepromAbstraction()->writeArrayToRom(position, (const uint8_t*)menuRomChoicesValue.getTextValue(), itemSize);

    if(renderer.getDialog()->isInUse()) return;
    renderer.getDialog()->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
    renderer.getDialog()->show(pgmHeaderSavedItem, true);
    renderer.getDialog()->copyIntoBuffer(menuRomChoicesValue.getTextValue());
}

//
// Here we handle the custom rendering for the number choices Scroll Choice menu item. We basically get called back
// every time it needs more data. For example when the name is required, when any index value is required or when
// it will be saved to EEPROM or invoked.
//
int CALLBACK_FUNCTION fnAdditionalNumChoicesRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
    switch(mode) {
        case RENDERFN_EEPROM_POS:
           return 32;
        case RENDERFN_INVOKE:
            serdebugF2("Num Choice changed to ", row);
            return true;
        case RENDERFN_NAME:
            strcpy(buffer, "Num Choices");
            return true;
        case RENDERFN_VALUE:
            buffer[0] = 'V'; buffer[1]=0;
            fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
            return true;
        default: return false;
    }
}

//
// Here we handle the custom rendering for the runtime list menu item that just counts. We basically get called back
// every time it needs more data. For example when the name is required, when any index value is required or when
// it will be saved to EEPROM or invoked.
//
int CALLBACK_FUNCTION fnAdditionalCountListRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
    switch(mode) {
        case RENDERFN_INVOKE: {
            // when the user selects an item in the list, we get this callback. Row is the offset of the selection
            renderer.getDialog()->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
            renderer.getDialog()->show(pgmListPressed, true);
            char sz[10];
            ltoaClrBuff(sz, row, 3, NOT_PADDED, sizeof sz);
            renderer.getDialog()->copyIntoBuffer(sz);
            return true;
        }
        case RENDERFN_NAME:
            // Called whenever the name of the menu item is needed
            strcpy(buffer, "List Item");
            return true;
        case RENDERFN_VALUE:
            // Called whenever the value of a given row is needed.
            buffer[0] = 'V'; buffer[1]=0;
            fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
            return true;
        case RENDERFN_EEPROM_POS: return 0xFFFF; // lists are generally not saved to EEPROM
        default: return false;
    }
}
