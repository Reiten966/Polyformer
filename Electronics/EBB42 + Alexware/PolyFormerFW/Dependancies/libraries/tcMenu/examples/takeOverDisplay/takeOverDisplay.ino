/**
 * ## TcMenu Example: Take over the display
 * This example shows the use of a MCP23017 IO expander that has a HD44780 LCD 20x4 display connected on port B and the
 * rotary encoder on port A, although could easily be changed to other kinds of IO expanders by redefining the two
 * IoExpander variables in the code generator properties. I've assumed this will be built on either a MEGA board, but
 * should work equally well on any other Arduino board with only minor changes.
 *
 * When the application starts up, it immediately takes over the display, and takes over the display every time the menu
 * detects that it is idle. This is done by setting up a reset handler with the renderer class. You can also manually
 * take over the display by pressing on the take display menu option.
 *
 * There are also two dialogs, dialogs are small display independent rendering utilities, that allow to inform the user
 * of a condition, or ask a question locally on the device. For example they are used heavily in the remote authentication.
 * See the two action menu items that bring up an informational text and also a question. Open the serial port at 115200
 * and you'll see the button you pressed there for the question one.
 *
 * There are two remote capabilities started by this application, the first is on Serial, to use this ensure that
 * IoLogging is not logging to the serial port, it also starts the Ethernet library too.
 *
 * This example also shows how to use an i2c AT24 EEPROM instead of the AVR one, comment in/out the one you want in the
 * sketch.
 *
 */

#include "takeOverDisplay_menu.h"
#include <EepromAbstractionWire.h>
#include <IoAbstractionWire.h>
#include <TaskManager.h>
#include <RemoteAuthentication.h>
#include <RemoteMenuItem.h>
#include <IoLogging.h>

// contains the graphical widget title components.
#include "stockIcons/wifiAndConnectionIconsLCD.h"

// Set up ethernet, the usual default settings are chosen. Change to your preferred values or use DHCP.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// In the designer UI we configured io23017 as an IoExpander variable for both the input and display.
// We must now create it as an MCP23017 expander. Address is 0x20 with interrupt pin connected to pin 2.
// make sure you've arranged for !RESET pin to be held HIGH!!
IoAbstractionRef io23017 = ioFrom23017(0x20, ACTIVE_LOW_OPEN, 2);

// a counter that we use in the display function when we take over the display.
int counter = 0;

// We add a title widget that shows when a user is connected to the device. Connection icons
// are in the standard icon set we included at the top.
// Yes even on LCD we now support title widgets, but they eat up a few of your custom chars.
// The width must always be 1, and the height is the first custom character that is used.
TitleWidget connectedWidget(iconsConnection, 2, 1, 0);


// when there's a change in communication status (client connects for example) this gets called.
void onCommsChange(CommunicationInfo info) {
    if(info.remoteNo == 0) {
        connectedWidget.setCurrentState(info.connected ? 1 : 0);
    }
    // this relies on logging in IoAbstraction's ioLogging.h, to turn it on visit the file for instructions.
    serdebugF4("Comms notify (rNo, con, enum)", info.remoteNo, info.connected, info.errorMode);
}

void setup() {
    //
    // If you are using serial (connectivity or logging) and wire they must be initialised 
    // before their first use.
    //
    Serial.begin(115200);
    Wire.begin();

    // When the renderer times out and is about to reset to main menu, you can get a callback.
    // For example if the menu should only be displayed during configuration.
    //
    // Call BEFORE setupMenu to ensure it takes effect immediately, call AFTER setupMenu if you
    // want to start in menu mode, but then apply the reset handler from that point onwards.
    renderer.setResetCallback([] {
        counter = 0;
        renderer.takeOverDisplay(myDisplayFunction);
    });

    serdebug("Added the reset callback");

    // Here we add a widget to the display, it will display connected status.
    renderer.setFirstWidget(&connectedWidget);

    // this is put in by the menu designer and must be called (always ensure devices are setup first).
    setupMenu();

    menuMgr.load();

    // spin up the Ethernet library, get the IP address from the menu
    byte* rawIp = menuConnectivityIPAddress.getIpAddress();
    IPAddress ip(rawIp[0], rawIp[1], rawIp[2], rawIp[3]);
    Ethernet.begin(mac, ip);

    // and print out the IP address
    char sz[20];
    menuConnectivityIPAddress.copyValue(sz, sizeof(sz));
    serdebugF2("Device IP is: ", sz);

    auto* authMgr = reinterpret_cast<EepromAuthenticatorManager*>(menuMgr.getAuthenticator());
    authMgr->copyPinToBuffer(sz, sizeof(sz));
	menuConnectivityChangePin.setTextValue(sz);
	menuConnectivityChangePin.setPasswordField(true);

    switches.addSwitch(4, [](uint8_t, bool held) {
        serdebugF2("Extra switch ", (held ? "held" : "pressed"));
    }, 20, true);

    menuConnectivityIoTMonitor.registerCommsNotification(onCommsChange);
}

//
// standard setup for all taskManager based sketches. Always call runLoop in the loop.
// Never do anything long running in here.
//
void loop() {
    taskManager.runLoop();
}

//
// When the food choice option is changed on the menu, this function is called, it takes
// the value from menuFood and renders it as text in the menuText text item.
//
void CALLBACK_FUNCTION onFoodChoice(int /*id*/) {
    // copy the enum text for the current value
    char enumStr[20];
    int enumVal = menuFood.getCurrentValue();
    menuFood.copyEnumStrToBuffer(enumStr, sizeof(enumStr), enumVal);

    serdebugF2("Changed food choice to ", enumStr);
    
    // and put it into a text menu item
    menuText.setTextValue(enumStr);
}

//
// this is the function called by the renderer every 1/5 second once the display is
// taken over, we pass this function to takeOverDisplay below.
//
void myDisplayFunction(unsigned int encoderValue, RenderPressMode clicked) {
    // we initialise the display on the first call.
    if(counter == 0) {
        switches.changeEncoderPrecision(999, 50);
        lcd.clear();
        lcd.print("We have the display!");
        lcd.setCursor(0, 1);
        lcd.print("OK button for menu..");
    }

    // We are told when the button is pressed in by the boolean parameter.
    // When the button is clicked, we give back to the menu..
    if(clicked) {
        renderer.giveBackDisplay();
        counter = 0;
    }
    else {
        char buffer[5];
        // otherwise update the counter.
        lcd.setCursor(0, 2);
        ltoaClrBuff(buffer, ++counter, 4, ' ', sizeof(buffer));
        lcd.print(buffer);
        lcd.setCursor(12, 2);
        ltoaClrBuff(buffer, encoderValue, 4, '0', sizeof(buffer));
        lcd.print(buffer);
    }
}

//
// We have an option on the menu to take over the display, this function is called when that
// option is chosen.
//
void CALLBACK_FUNCTION onTakeOverDisplay(int /*id*/) {
    // in order to take over rendering onto the display we just request the display
    // at which point tcMenu will stop rendering until the display is "given back".
    // Don't forget that LiquidCrystalIO uses task manager and things can be happening
    // in the background. Always ensure all operations with the LCD occur on the rendering
    // call back.

    counter = 0;
    renderer.takeOverDisplay(myDisplayFunction);
}

const char pgmInfoHeader[] PROGMEM = "Information dialog";
const char pgmQuestionHeader[] PROGMEM = "Order Food?";

void CALLBACK_FUNCTION onInfoDlg(int /*id*/) {
    // every renderer apart from NoRenderer has a dialog, that can be used to present
    // very basic info locally onto any display. Used in situations where something
    // needs to be confirmed / printed onto the local display.
    BaseDialog* dlg = renderer.getDialog();
    if(!dlg) return;

    // first we set the buttons how we want them. BTNTYPE_NONE means no button.
    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);

    // then we show the dialog - 2nd boolean parameter is if dialog is local only
    dlg->show(pgmInfoHeader, true);

    // and then we set the second line (buffer) - must be after show.
    dlg->copyIntoBuffer("to be set..");

    // you can set the dialog buffer at some point later, it's safe, even if it's been dismissed.
    taskManager.scheduleOnce(1000, [] {
        BaseDialog* dlg = renderer.getDialog();
        dlg->copyIntoBuffer("now it's set..");
    });
}

//
// It's also possible to know when the dialog has finished, and what button was pressed.
// This is done by passing a function like below as second parameter to show.
//
void onFinished(ButtonType btn, void* /*userData*/) {
    if(btn == BTNTYPE_ACCEPT) {
        char sz[20];
        menuFood.copyEnumStrToBuffer(sz, sizeof(sz), menuFood.getCurrentValue());
        serdebugF2("Chosen food ", sz);
    }
    else {
        serdebugF("User did not choose to proceed.");
    }
}

void CALLBACK_FUNCTION onQuestionDlg(int /*id*/) {
    // yet another dialog, to ask a question this time.
    BaseDialog* dlg = renderer.getDialog();
    
    // this time we use two buttons and provide the selected index at the end (zero based)
    dlg->setButtons(BTNTYPE_ACCEPT, BTNTYPE_CANCEL, 1);
    
    // we can optionally set some data that will be given to us in the finished call back.
    dlg->setUserData(NULL); 
    
    // now we show the dialog (also giving the finished callback)
    dlg->show(pgmQuestionHeader, true, onFinished);

    // and lastly we set the text in the buffer area (2nd line)
    char sz[20];
    menuFood.copyEnumStrToBuffer(sz, sizeof(sz), menuFood.getCurrentValue());
    dlg->copyIntoBuffer(sz);
}

//
// We have a save option on the menu to save the settings. In a real system we could instead
// look at using a power down detection circuit to do this. For more info see below link.
// https://www.thecoderscorner.com/electronics/microcontrollers/psu-control/detecting-power-loss-in-powersupply/
//
void CALLBACK_FUNCTION onSaveSettings(int /*id*/) {
    menuMgr.save();
}

const char pgmPinTooShort[] PROGMEM = "Pin too short";
void CALLBACK_FUNCTION onChangePin(int id) {
	const char* sz = menuConnectivityChangePin.getTextValue();
	if (strlen(sz) < 4) {
		BaseDialog* dlg = renderer.getDialog();
		dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
		dlg->copyIntoBuffer(sz);
		dlg->show(pgmPinTooShort, false);
	}
	else {
	    auto* authMgr = reinterpret_cast<EepromAuthenticatorManager*>(menuMgr.getAuthenticator());
		authMgr->changePin(sz);
		serdebugF2("Pin changed to ", sz);
	}
}
