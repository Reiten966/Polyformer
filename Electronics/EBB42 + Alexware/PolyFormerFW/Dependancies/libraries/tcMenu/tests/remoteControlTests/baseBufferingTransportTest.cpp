#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseBufferedRemoteTransport.h>
#include <SimpleCollections.h>

using namespace aunit;
using namespace tcremote;

class UnitTestBufferedTransport : public BaseBufferedRemoteTransport {
private:
    const char* expectedData;
    bool bufferComplete = false;
    int sequence = 1;
    bool bufferValid = false;
    bool shouldFillBuffer = true;
public:
    UnitTestBufferedTransport(BufferingMode bufferMode, const char *expected)
                : BaseBufferedRemoteTransport(bufferMode, 24, 32), expectedData(expected) {}

    int fillReadBuffer(uint8_t *dataBuffer, int maxSize) override {
        if(shouldFillBuffer) {
            int toFill = min(maxSize, 11);
            for (int i = 0; i < toFill; i++) {
                dataBuffer[i] = sequence++;
            }
            return toFill;
        } else return 0;
    }

    void flush() override {
        bufferValid = strncmp((const char*)writeBuffer, (const char*)expectedData, writeBufferPos) == 0;
        bufferComplete = true;
    }

    bool available() override {
        return true;
    }

    bool connected() override {
        return true;
    }

    void setBufferFill(bool f) { shouldFillBuffer = f; }

    bool didCompleteSuccessfully() const {
        return bufferValid && bufferComplete;
    }
};

test(testBufferedTransportReadB) {
    UnitTestBufferedTransport transport(BUFFER_ONE_MESSAGE, "");
    for(int i=1; i<100; i++) {
        assertTrue(transport.readAvailable());
        int by = transport.readByte();
        assertEqual(by, i);
    }

    transport.setBufferFill(false);

    int availableCount = 0;
    for(int i=1; i<100; i++) {
        if(transport.readAvailable()) {
            availableCount++;
            transport.readByte();
        }
    }
    assertTrue(availableCount < 25);
}

test(testBufferTransportWriteBufferOneMsg) {
    UnitTestBufferedTransport transport(tcremote::BUFFER_ONE_MESSAGE, "\001\001NJB1=1234|B2=4321|\002");

    transport.startMsg(MSG_JOIN);
    transport.writeField(FIELD_BUTTON1, "1234");
    transport.writeField(FIELD_BUTTON2, "4321");
    transport.endMsg();

    assertTrue(transport.didCompleteSuccessfully());
}

test(testBufferTransportWriteBufferMulti) {
    UnitTestBufferedTransport transport(BUFFER_MESSAGES_TILL_FULL, "\001\001NJB1=12|\002\001\001HBB1=2|\002");

    transport.startMsg(MSG_JOIN);
    transport.writeFieldInt(FIELD_BUTTON1, 12);
    transport.endMsg();

    transport.startMsg(MSG_HEARTBEAT);
    transport.writeFieldInt(FIELD_BUTTON1, 2);
    transport.endMsg();

    transport.flush();
    assertTrue(transport.didCompleteSuccessfully());
}