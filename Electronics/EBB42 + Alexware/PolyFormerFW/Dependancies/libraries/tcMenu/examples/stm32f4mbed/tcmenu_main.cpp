/**
 * An example menu that works on mbed, we explicitly tested this on a STM32F439 device with ethernet capability. However,
 * there should be general applicability to any mbed device with similar features. This uses our OLED library to present
 * the menu, we defaulted to SPI, but you could change easily to i2c.
 *
 * Compilation works for both mbed 5 and 6. Add build flag -DBUILD_FOR_MBED_6 to build for V6. This is an RTOS example.
 */

#include <mbed.h>
#include "stm32f4mbed_menu.h"
#include "NTPTimeEvent.h"
#include "ScreenSaverCustomDrawing.h"
#include "mbed/HalStm32EepromAbstraction.h"
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <RemoteMenuItem.h>
#include <tcMenuVersion.h>

#ifdef BUILD_FOR_MBED_6
BufferedSerial serPort(USBTX, USBRX);
#else
Serial serPort(USBTX, USBRX);
#endif
MBedLogger LoggingPort(serPort);

// indicates that the process is still active and the run loop should continue.
bool isRunning = true;

// When we created the choices item in the designer, we said create choices in ram, and provided a variable name.
// So we must declare the variable here of type const char*
const char * choicesItems = "xmas\0     easter\0   whitsun\0  mayday\0    ";

// We said the OLED display was on SPI, we must set it up here.
SPI spi(PB_5, PB_4, PB_3);

// This is the actual title widget declaration - see setup
TitleWidget deviceConnectedWidget(iconsConnection, 2, 16, 12, nullptr);
TitleWidget ethernetConnectionWidget(iconsEthernetConnection, 2, 16, 12, &deviceConnectedWidget);
ScreenSaverCustomDrawing screenSaver;

// forward declaration
void prepareRealtimeClock();

void setup() {
#ifdef BUILD_FOR_MBED_6
    serPort.set_baud(115200);
#else
    serPort.baud(115200);
#endif

    // we add a simple widget that appears on the top right of the menu, do this before calling setup to ensure that
    // it draws immediately. There's three things to do here
    // 1. include the stock icons - see includes
    // 2. create one more title widgets (see tcMenu docs for how)
    // 3. add them to the renderer
    renderer.setFirstWidget(&ethernetConnectionWidget);

    // now we have a task that every few seconds checks if we are still connected to the network
    taskManager.scheduleFixedRate(2, [] {
        auto pConnection = remoteServer.getRemoteServerConnection(0);
        if(pConnection) ethernetConnectionWidget.setCurrentState(pConnection->getDeviceInitialisation().isInitialised() ? 1 : 0);
    }, TIME_SECONDS);

    // and register to receive updates from the connector when connectivity changes. When connected is true
    // we have a user connected to our remote endpoint.
    menuIoTMonitor.registerCommsNotification([](CommunicationInfo info) {
        deviceConnectedWidget.setCurrentState(info.connected ? 1 : 0);
    });

    // this was added by designer, it sets up the input, display and remote.
    setupMenu();

    menuMgr.load();

    // and now lets try and acquire time using our quick ntp time class
    prepareRealtimeClock();

    // here we register our screen saver class with the renderer, see ScreenSaverCustomDrawing.h
    renderer.setCustomDrawingHandler(&screenSaver);

    // we add another switch on the user button that just clears the screen saver.
    switches.addSwitch(USER_BUTTON, [](pinid_t /*pin*/, bool /*held*/) {
        screenSaver.removeScreenSaver();
    });

    // When the main title is pressed or touched, we can register a callback to be executed.
    // Here we just present a simple dialog.
    setTitlePressedCallback([](int id) {
        auto* dlg = renderer.getDialog();
        if(dlg && !dlg->isInUse()) {
            dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
            dlg->show("Mbed demo", false);
            char szVer[10];
            tccore::copyTcMenuVersion(szVer, sizeof szVer);
            dlg->copyIntoBuffer(szVer);
        }
    });

    // We set the number of items in our runtime list
    menuCountingList.setNumberOfRows(10);
}

int main() {
    setup();
    while(isRunning) {
        taskManager.runLoop();

        // If there's a large enough gap between tasks, you can add a low power manager as follows below, just ensure
        // that it still wakes up on all the events.
        //auto microsToWait = taskManager.microsToNextTask();
        //myLowPowerLib.sleepWithInterruptCapability(microsToWait);
    }
}

void CALLBACK_FUNCTION onTenthsChaned(int /*id*/) {
    char sz[20];
    ltoaClrBuff(sz, menuTenths.getCurrentValue(), 5, '0', sizeof sz);
    menuEdit.setTextValue(sz);
}

void CALLBACK_FUNCTION onFoodChange(int /*id*/) {
    char sz[20];
    menuFoods.copyEnumStrToBuffer(sz, sizeof  sz, menuFoods.getCurrentValue());
    menuEdit.setTextValue(sz);
}

void CALLBACK_FUNCTION onFrequencyChanged(int /*id*/) {
    char sz[20];
    menuFrequency.copyValue(sz, sizeof sz);
    menuEdit.setTextValue(sz);
}

// see tcMenu list documentation on thecoderscorner.com
int CALLBACK_FUNCTION fnCountingListRtCall(RuntimeMenuItem * /*item*/, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        serdebugF2("Invoked list item", row);
        return true;
    case RENDERFN_NAME:
        strcpy(buffer, "Name");
        fastltoa(buffer, row, 3, '0', bufferSize);
        return true;
    case RENDERFN_VALUE:
        // TODO - return a value for the row (for list items row==LIST_PARENT_ITEMPOS is back)
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xFFFF; // lists are generally not saved to EEPROM
    default: return false;
    }
}

class NTPTimeMenuSetupEvent : public NTPTimeEvent {
public:

    NTPTimeMenuSetupEvent(NetworkInterface *interface, const char *timeServer, int timePort)
            : NTPTimeEvent(interface, timeServer, timePort) {
    }

    void exec() override {
        serdebugF("Time event has triggered");
        set_time(_presentValue);

        taskManager.scheduleFixedRate(1, [] {
            auto timeNow = time(nullptr);
            auto tm = gmtime(&timeNow);
            menuRTCDate.getUnderlyingData()->year = (tm->tm_year + 1900);
            menuRTCDate.getUnderlyingData()->month = tm->tm_mon + 1;
            menuRTCDate.getUnderlyingData()->day = tm->tm_mday;
            menuRTCTime.getUnderlyingData()->hours = tm->tm_hour;
            menuRTCTime.getUnderlyingData()->minutes = tm->tm_min;
            menuRTCTime.getUnderlyingData()->seconds = tm->tm_sec;
            menuRTCTime.setSendRemoteNeededAll();
            menuRTCDate.setSendRemoteNeededAll();
            menuRTCTime.setChanged(true);
            menuRTCDate.setChanged(true);

        }, TIME_SECONDS);
        setCompleted(true);
    }
};

#ifdef BUILD_FOR_MBED_6
void prepareRealtimeClock() {

    // make sure a connection was registered before trying to use it
    BaseRemoteServerConnection *pConnection = remoteServer.getRemoteServerConnection(0);

    // and then we
    auto& devInit = reinterpret_cast<const MbedEthernetInitialiser&>(pConnection->getDeviceInitialisation());
    if(pConnection != nullptr && devInit.isInitialised()) {
        SocketAddress addr;
        if(devInit.getInterface()->get_ip_address(&addr) == NSAPI_ERROR_OK) {
            menuIP.setIpAddress(addr.get_ip_address());
            taskManager.registerEvent(new NTPTimeMenuSetupEvent(
                    devInit.getInterface(),
                    "2.pool.ntp.org", 123),  true);
            return;
        }
    }
    taskManager.scheduleOnce(5, prepareRealtimeClock, TIME_SECONDS);
}
#else
void prepareRealtimeClock() {
    if(remoteServer.isBound()) {
        menuIP.setIpAddress(remoteServer.networkInterface()->get_ip_address());
        taskManager.registerEvent(new NTPTimeMenuSetupEvent(
                remoteServer.networkInterface(),
                "2.pool.ntp.org", 123),  true);
        return;
    }
    taskManager.scheduleOnce(5, prepareRealtimeClock, TIME_SECONDS);
}
#endif // BUILD_FOR_MBED_5

void CALLBACK_FUNCTION onSaveAll(int id) {
    menuMgr.save();
    reinterpret_cast<HalStm32EepromAbstraction*>(menuMgr.getEepromAbstraction())->commit();
    auto* dlg = renderer.getDialog();
    if(dlg && !dlg->isInUse()) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
        dlg->showRam("Saved to ROM", false);
        dlg->copyIntoBuffer("");
    }
}
