#include <Arduino.h>
#include <SCCircularBuffer.h>
#include <RemoteConnector.h>
#include "remote/TcMenuWebServer.h"
#include "UnitTestDriver.h"

namespace tcremote {
    const uint8_t serverMask[] = {0xaa, 0xee, 0xdd, 0xa0};

    bool unitLayerStarted = true;
    int acceptorSocket = TC_BAD_SOCKET_ID;
    int acceptPortChosen = 0;
    ServerAcceptedCallback serverAcceptCallback = nullptr;
    void *serverCallbackData = nullptr;

    UnitDriverSocket driverSocket;

    void resetUnitLayer() {
        unitLayerStarted = false;
        acceptorSocket = TC_BAD_SOCKET_ID;
        acceptPortChosen = 0;
        serverAcceptCallback = nullptr;
        serverCallbackData = nullptr;
        driverSocket.reset();
    }

    SocketErrCode startNetLayerDhcp() {
        unitLayerStarted = true;
        return SOCK_ERR_OK;
    }

    SocketErrCode startNetLayerManual(const uint8_t *ip, const uint8_t *mac, const uint8_t *mask) {
        return SOCK_ERR_UNSUPPORTED;
    }

    void copyIpAddress(socket_t theSocket, char *buffer, size_t bufferSize) {
        if (theSocket == TC_LOCALHOST_SOCKET_ID) {
            strncpy(buffer, "192.168.20.21", bufferSize);
        } else {
            strncpy(buffer, "192.168.20.22", bufferSize);
        }
    }

    bool isNetworkUp() {
        return unitLayerStarted;
    }

    SocketErrCode initialiseAccept(int port, ServerAcceptedCallback onServerAccepted, void *callbackData) {
        acceptorSocket = 99;
        acceptPortChosen = port;
        serverAcceptCallback = onServerAccepted;
        serverCallbackData = callbackData;
        return SOCK_ERR_OK;
    }

    bool rawReadAvailable(socket_t socketNum) {
        if (socketNum < 0) return -1;
        if (driverSocket.isIdle()) return false;
        return driverSocket.readAvailable();
    }

    int rawReadData(socket_t socketNum, void *data, size_t dataLen) {
        if (socketNum < 0) return -1;
        if (driverSocket.isIdle()) return false;
        return driverSocket.performRawRead((uint8_t *) data, dataLen);
    }

    bool rawWriteAvailable(socket_t socketNum) {
        return true;
    }

    SocketErrCode rawWriteData(socket_t socketNum, const void *data, size_t dataLen, MemoryLocationType memType, int timeoutMillis) {
        if(memType == IN_PROGRAM_MEM) return SOCK_ERR_NO_PROGMEM_SUPPORT;
        if (socketNum < 0) return SOCK_ERR_FAILED;
        if (driverSocket.isIdle()) return SOCK_ERR_FAILED;
        if (driverSocket.performRawWrite((uint8_t *) data, dataLen) == dataLen) return SOCK_ERR_OK;
        return SOCK_ERR_FAILED;
    }

    SocketErrCode rawFlushAll(socket_t socketNum) {
        if (socketNum < 0) return SOCK_ERR_FAILED;
        if (driverSocket.isIdle()) return SOCK_ERR_FAILED;
        driverSocket.flush();
        return SOCK_ERR_OK;
    }

    void closeSocket(socket_t sockFd) {
        if (sockFd >= 0) {
            driverSocket.markAsClosed();
        }
    }

    socket_t nextDriverSocket() {
        if (driverSocket.isIdle()) return 0;
        else return TC_BAD_SOCKET_ID;
    }

    void simulateAccept() {
        if (serverAcceptCallback) {
            auto sock = nextDriverSocket();
            if (sock < 0) return;
            driverSocket.reset(true);
            serverAcceptCallback(sock, serverCallbackData);
        }
    }


    void UnitDriverSocket::flush() {
        if(!shouldBeInWebSocketMode) return;
        int fl = writeScBuffer.get();
        if (fl != 0x81) return; // Final message, text
        int len = writeScBuffer.get();
        if (len > 125) {
            return; // don't handle extended case
        }
        char sz[128];
        int i;
        for (i = 0; i < len; i++) {
            sz[i] = writeScBuffer.get();
        }
        sz[i] = 0;
        int pos = 0;
        if (sz[pos++] != 0x01 || sz[pos++] != 0x01) return;
        uint16_t msgTy = sz[pos++] << 8U;
        msgTy |= sz[pos++];

        receivedMessages.add(ReceivedMessage(receivedMessages.count(), msgTy, &sz[pos]));
    }

    void UnitDriverSocket::simulateIncomingMsg(uint16_t msgType, const char *data, bool masked) {
        readScBuffer.put(0x81);
        auto dataLen = strlen(data) + 5;
        uint8_t maskData = masked ? 0x80 : 0;
        if (dataLen > 125) {
            readScBuffer.put(maskData | 126);
            readScBuffer.put(dataLen >> 8);
            readScBuffer.put(dataLen & 0xff);
        } else {
            readScBuffer.put(maskData | dataLen);
        }

        if (masked) {
            readScBuffer.put(serverMask[0]); // mask 4 bytes
            readScBuffer.put(serverMask[1]);
            readScBuffer.put(serverMask[2]);
            readScBuffer.put(serverMask[3]);
        } else {
            readScBuffer.put(dataLen);
        }
        readScBuffer.put(0x01 ^ serverMask[0]); // start
        readScBuffer.put(0x01 ^ serverMask[1]); // tag val
        readScBuffer.put((msgType >> 8) ^ serverMask[2]);
        readScBuffer.put((msgType & 0xff) ^ serverMask[3]);
        int maskPosition = 0;
        while (*data) { // data
            readScBuffer.put(*data ^ serverMask[maskPosition % 4]);
            maskPosition++;
            data++;
        }
        readScBuffer.put(0x02 ^ serverMask[maskPosition % 4]); // end
    }

    void UnitDriverSocket::simulateIncomingRaw(const char *rawData) {
        while(*rawData) {
            readScBuffer.put(*rawData);
            rawData++;
        }
    }

    bool UnitDriverSocket::checkResponseAgainst(const char *expected) {
        char sz[255];
        size_t pos = 0;
        while(writeScBuffer.available() && pos < (sizeof(sz)-1)) {
            sz[pos++] = (char)(writeScBuffer.get());
        }
        sz[pos]=0;

        bool matches = strncmp(sz, expected, sizeof(sz)) == 0;
        if(!matches) {
            serdebugF2("Mismatch exp=", expected);
            serdebugF2("Mismatch act=", sz);
            return false;
        } else return true;
    }
}