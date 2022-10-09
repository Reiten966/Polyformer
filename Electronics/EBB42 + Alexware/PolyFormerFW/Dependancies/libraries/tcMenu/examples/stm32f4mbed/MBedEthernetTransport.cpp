/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * Ethernet remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#include "MBedEthernetTransport.h"

MBedEthernetTransport::~MBedEthernetTransport() {
    if(socket) {
        socket->close();
    }
}

void MBedEthernetTransport::flush() {
    if(writeBufferPos == 0 || socket == nullptr) return;

    int attempts = 0;
    int pos = 0;
    int sizeToGo = writeBufferPos;
    while(++attempts < 50) {
        int written = socket->send(&writeBuffer[pos], sizeToGo);
        if(written == NSAPI_ERROR_WOULD_BLOCK) continue;
        if(written <=0) {
            serdebugF2("socket error ", written);
            close();
            return;
        }
        if(written < sizeToGo) {
            serdebugF2("Written chunk ", written);
            pos += written;
            sizeToGo -= written;
        }
        else {
            serdebugF2("Written all ", written);
            writeBufferPos = 0;
            return;
        }
        taskManager.yieldForMicros(250);
    }
}

int MBedEthernetTransport::fillReadBuffer(uint8_t* dataBuffer, int maxData) {
    if(!connected()) return 0;

    int attempts = 0;
    while(++attempts < 50) {
        auto amt = socket->recv(dataBuffer, maxData);
        if(amt == NSAPI_ERROR_WOULD_BLOCK) continue;
        if(amt <= 0) {
            serdebugF2("socket error ", amt);
            close();
            return 0;
        }

        serdebugF2("read to buffer ", amt);
        return amt;
    }
    return 0;
}

bool MBedEthernetTransport::available() {
    return (socket != nullptr && isOpen);
}

void MBedEthernetTransport::close() {
    if(socket == nullptr) return;
    serdebugF("closing socket");

    BaseBufferedRemoteTransport::close();

    isOpen = false;
    socket->close();
    // socket is now a dangling pointer and must be cleared
    socket = nullptr;
}

MbedEthernetInitialiser::MbedEthernetInitialiser(int port, NetworkInterface* networkInterface) : interface(networkInterface), port(port) {
    if(interface == nullptr) interface = NetworkInterface::get_default_instance();
    initState = TRY_CONNECT;
    interface->set_blocking(false);
    server.set_blocking(false);
}

bool MbedEthernetInitialiser::attemptInitialisation() {
    if(initState == TRY_CONNECT) {
        if(interface->connect() != NSAPI_ERROR_IS_CONNECTED) {
            return false;
        }
        initState = TRY_BIND;
    }

    if(initState == TRY_BIND) {
        serdebugF("Connected to network");
        if(server.open(interface) != 0) {
            serdebugF("Could not open socket");
            return false;
        }
        if(server.bind(port) != 0 || server.listen(1) != 0) {
            serdebugF2("Could not bind to ", port);
            return false;
        }
        initState = FULLY_CONNECTED;
        initialised = true;
        return true;
    }
    return false;
}

bool MbedEthernetInitialiser::attemptNewConnection(BaseRemoteServerConnection *remoteConnection) {
    auto* tvRemote = reinterpret_cast<TagValueRemoteServerConnection*>(remoteConnection);
    auto* mbedTransport = reinterpret_cast<MBedEthernetTransport*>(tvRemote->transport());
    nsapi_error_t acceptErr;
    auto tcpSock = server.accept(&acceptErr);
    if(acceptErr == NSAPI_ERROR_OK) {
        serdebugF("Client found");
        mbedTransport->setSocket(tcpSock);
        return true;
    }
    else if(acceptErr != NSAPI_ERROR_WOULD_BLOCK) {
        serdebugF2("Error code ", acceptErr);
    }
    return false;
}
