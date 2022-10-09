/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * Ethernet remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#ifndef TCMENU_MBEDETHERNETTRANSPORT_H
#define TCMENU_MBEDETHERNETTRANSPORT_H

#include <mbed.h>
#include <EthernetInterface.h>
#include <TCPSocket.h>

#include <remote/BaseBufferedRemoteTransport.h>
#include <remote/BaseRemoteComponents.h>
#include <TaskManager.h>
#include <SimpleSpinLock.h>

namespace tcremote {

    /**
     * An implementation of TagValueTransport that is able to read and write via a buffer to sockets.
     */
    class MBedEthernetTransport : public BaseBufferedRemoteTransport {
    private:
        InternetSocket* socket;
        bool isOpen;
    public:
        MBedEthernetTransport() : BaseBufferedRemoteTransport(BUFFER_MESSAGES_TILL_FULL, 96, 128) {
            this->socket = nullptr;
            isOpen = false;
        }

        ~MBedEthernetTransport() override;

        void setSocket(InternetSocket* sock) {
            close();

            socket = sock;
            socket->set_blocking(false);
            isOpen = true;
        }

        int fillReadBuffer(uint8_t* data, int maxSize) override;
        void flush() override;
        bool available() override;
        bool connected() override { return socket != nullptr && isOpen; }
        void close() override;

    };

    class MbedEthernetInitialiser : public DeviceInitialisation {
    public:
        enum MbedInitState { TRY_CONNECT, TRY_BIND, FULLY_CONNECTED };
    private:
        NetworkInterface* interface;
        TCPSocket server;
        int port;
        MbedInitState initState;
    public:
        explicit MbedEthernetInitialiser(int port, NetworkInterface* interface = nullptr);
        bool attemptInitialisation() override;
        bool attemptNewConnection(BaseRemoteServerConnection *remoteConnection) override;
        NetworkInterface* getInterface() const { return interface; }
    };

}

#ifndef TC_MANUAL_NAMESPACING
using namespace tcremote;
#endif // TC_MANUAL_NAMESPACING

#endif //TCMENU_MBEDETHERNETTRANSPORT_H
