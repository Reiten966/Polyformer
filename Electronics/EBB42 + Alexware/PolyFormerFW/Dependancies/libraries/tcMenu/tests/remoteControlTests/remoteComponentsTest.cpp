
#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseRemoteComponents.h>
#include <SimpleCollections.h>
#include "remote/TcMenuWebServer.h"
#include "SimpleTestFixtures.h"
#include "UnitTestDriver.h"

using namespace aunit;
using namespace tcremote;

const ConnectorLocalInfo localInfo PROGMEM = { "unitTest", "699cb00d-200f-47f5-8d2e-471acbe4adb3"};
TcMenuRemoteServer remoteServer(localInfo);

class TestTransportInitialisation : public DeviceInitialisation {
private:
    bool connectionMade = false;
    int initCount = 0;
public:
    bool attemptInitialisation() override {
        connectionMade = false;
        initCount++;
        return initialised;
    }

    void setInialisationDone(bool done) {
        initialised = done;
    }

    int getInitialisationAttempts() const {
        return initCount;
    }

    bool isConnectionMade() const {
        return connectionMade;
    }

    bool attemptNewConnection(BaseRemoteServerConnection *connection) override {
        if(initialised) {
            auto* tagValConnector = reinterpret_cast<TagValueRemoteServerConnection*>(connection);
            auto* testTransport = reinterpret_cast<TestTagValTransport*>(tagValConnector->transport());
            testTransport->reset();
            connectionMade = true;
            initCount = 0;
        }
        return false;
    }
};

bool tcremote::checkForMessageOfType(BtreeList<uint16_t, ReceivedMessage>& msgs, uint16_t msgType, const char* expected) {
    serdebugF2("Check msg ", expected);
    auto* rxMsg = msgs.getByKey(msgType);
    if(rxMsg == nullptr) {
        serdebugF2("message not found ", msgType);
    }

    char *actualData = rxMsg->getData();
    if(strcmp(actualData, expected) != 0) {
        serdebugF4("string mismatch: ", msgType, actualData, expected);
        return false;
    }
    serdebugF("Success!")
    return true;
}

TestTagValTransport tvTestTransport;
TestTransportInitialisation tvTestInitialisation;
TagValueRemoteServerConnection tvRsc(tvTestTransport, tvTestInitialisation);

test(testTcMenuRemoteServer) {
    serdebugF("Starting remote protocol test");

    remoteServer.clearRemotes();
    menuMgr.setRootMenu(&menuVolume);

    remoteServer.addConnection(&tvRsc);

    assertNotEqual(nullptr, remoteServer.getRemoteConnector(0));
    assertEqual(&tvTestTransport, remoteServer.getTransport(0));
    assertEqual(1, remoteServer.remoteCount());
    assertEqual(nullptr, remoteServer.getRemoteConnector(1));
    assertEqual(nullptr, remoteServer.getTransport(1));

    tvRsc.runLoop();
    assertEqual(1, tvTestInitialisation.getInitialisationAttempts());
    tvTestInitialisation.setInialisationDone(true);
    tvRsc.runLoop();
    assertTrue(tvTestInitialisation.isConnectionMade());
    tvTestTransport.simulateConnection();

    tvTestTransport.simulateIncomingMsg(MSG_HEARTBEAT, "HI=5000|");
    tvTestTransport.simulateIncomingMsg(MSG_JOIN, "NM=unitTest|VE=103|PF=0|UU=db598308-9e31-451c-8511-25027bcf15fb|");
    bool commsEnded = false;
    uint32_t iterations = 0;
    while(!commsEnded && iterations++ < 100000) {
        tvRsc.runLoop();
        auto* msg = tvTestTransport.getReceivedMessages().getByKey(MSG_BOOTSTRAP);
        commsEnded = msg != nullptr && strncmp(msg->getData(), "BT=END|", 7) == 0;
    }

    serdebugF("Start iteration");
    assertTrue(checkForMessageOfType(tvTestTransport.getReceivedMessages(), MSG_BOOT_ANALOG, "PI=0|ID=1|IE=2|RO=0|VI=1|NM=Volume|AU=dB|AM=255|AO=-190|AD=2|VC=0|\002"));
    assertTrue(checkForMessageOfType(tvTestTransport.getReceivedMessages(), MSG_BOOT_ENUM, "PI=0|ID=2|IE=4|RO=0|VI=1|NM=Channel|VC=0|NC=3|CA=CD Player|CB=Turntable|CC=Computer|\002"));
    assertTrue(checkForMessageOfType(tvTestTransport.getReceivedMessages(), MSG_BOOT_SUBMENU, "PI=0|ID=3|IE=65535|RO=0|VI=1|NM=Settings|\002"));
    assertTrue(checkForMessageOfType(tvTestTransport.getReceivedMessages(), MSG_BOOT_BOOL, "PI=3|ID=4|IE=65535|RO=0|VI=1|NM=12V Standby|VC=0|BN=2|\002"));
    assertTrue(commsEnded);
    serdebugF("Finished iteration");

    serdebugF("Closing remotes");
    remoteServer.clearRemotes();
    tvTestTransport.reset();
    serdebugF("Remote protocol test finished");
}
