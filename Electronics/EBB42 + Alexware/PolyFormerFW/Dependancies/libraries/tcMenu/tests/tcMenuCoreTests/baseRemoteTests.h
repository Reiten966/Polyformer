#ifndef _BASE_REMOTE_TESTS_H
#define _BASE_REMOTE_TESTS_H

#include <RemoteConnector.h>
#include <MessageProcessors.h>

extern const char pgmMyName[];
extern NoRenderer noRenderer; 

class UnitTestTransport : public TagValueTransport {
private:
    // indicates that no write has taken place since the last flush when true
    bool lineFlushed;
    // indicates that something is wrong internally and an assertion should be created
    bool errorOccurred;
    // normally set to true to allow writing, set to false to stop writing.
    bool writeAvailable;
    // indicates if connected or not.
    bool conn;
    // Writing takes place to this buffer, the linePosition is the position in the below write buffer for write ops.
    unsigned int linePosition;
    char lineBuffer[128];
    // the variables dealing with reading from the buffer. Position in the buffer, length of buffer and the buffer.
    int readPosition;
    int readLength;
    char readBuffer[128];

public:
    UnitTestTransport() {
        init();
    }

    void init() {
        lineFlushed = false;
        errorOccurred = false;
        writeAvailable = true;
        conn = false;
        linePosition = readPosition = readLength = 0;
        lineBuffer[0]=0;
        readBuffer[0]=0;
    }

	~UnitTestTransport() override { }

   	void flush() override {
        if(!conn) {
            errorOccurred = true;
        }
        lineFlushed = true;
    }

	int writeChar(char data) override {
        lineFlushed = false;
        if(!conn || linePosition >= sizeof(lineBuffer)) {
            errorOccurred = true;
            return 0;
        }
        lineBuffer[linePosition] = data;
        linePosition++;
        return 1;
    }

	int writeStr(const char* data) override {
        lineFlushed = false;
        int dataLen = strlen(data);
        if(!conn || (linePosition + dataLen) >= sizeof(lineBuffer)) {
            serdebugF3("error write P,L=", linePosition, dataLen);
            errorOccurred = true;
            return 0;
        }
        strcpy(&lineBuffer[linePosition], data);
        linePosition += dataLen;
        return dataLen;
    }

	uint8_t readByte() override {
        if(!readAvailable()) {
            return 0;
        }

        return readBuffer[readPosition++];
    }

	bool readAvailable() override {
        return conn && readPosition < readLength;
    }

	bool available() override {
        return conn && writeAvailable;
    }

	bool connected() override {
        return conn;
    }

	void close() override {
        conn = false;
    }

    void setConnected(bool connected) {
        this->conn = connected;
    }

    void setReadBuffer(const char* buf) {
        strcpy(readBuffer, buf);
        readLength = strlen(readBuffer);
        readPosition = 0;
    }

    const char* getWriteBuffer() {
        return lineBuffer;
    }

    void resetWriteBuffer() {
        lineBuffer[0] = 0;
        linePosition = 0;
    }

	bool isReadBufferEmpty() {
		return readPosition == (readLength - 1);
	}

    int getLastWriteBufferChar() {
        if(linePosition == 0) return 0;
        return lineBuffer[linePosition-1];
    }
};

using namespace aunit;

class RemoteFixture : public TestOnce {
protected:
    TagValueRemoteConnector remoteConnector;
    UnitTestTransport transport;
    CombinedMessageProcessor messageProcessor;
public:
    RemoteFixture() : remoteConnector(2), messageProcessor(msgHandlers, MSG_HANDLERS_SIZE) { }

    void setup() override {
        taskManager.reset();
        transport.init();
        remoteConnector.initialise(&transport, &messageProcessor, pgmMyName);
        menuMgr.initWithoutInput(&noRenderer, &textMenuItem1);
    }

	void waitForMessageOnTransport(const char* expected) {
        int counter = 0;
		while (transport.getLastWriteBufferChar() != 0x02 && counter < 1000) {
			remoteConnector.tick();
            counter++;
		}
		remoteConnector.tick();
		assertEqual(transport.getWriteBuffer(), expected);
	}

	void waitForEmptyReadBuffer() {
        int counter = 0;
		while (!transport.isReadBufferEmpty() && counter < 1000) {
			remoteConnector.tick();
            counter++;
		}
	}
};

testF(RemoteFixture, testConnectingAndJoining) {
    transport.setConnected(true);
    waitForMessageOnTransport("MT=NJ|NM=UnitTest|VE=100|PF=0|~");
    transport.setReadBuffer("MT=NJ|NM=remo|VE=105|PF=1|~");
    waitForEmptyReadBuffer();
    
    assertEqual("remo", remoteConnector.getRemoteName());
    assertEqual(1, remoteConnector.getRemoteMajorVer());
    assertEqual(5, remoteConnector.getRemoteMinorVer());
    assertEqual(PLATFORM_JAVA_API, remoteConnector.getRemotePlatform());
    assertEqual((uint8_t)2, remoteConnector.getRemoteNo());
}

#endif
