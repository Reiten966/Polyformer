/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file BaseRemoteComponents.h contains remote components that are shared across all remote devices.
 */

#ifndef TCMENU_BASEREMOTECOMPONENTS_H
#define TCMENU_BASEREMOTECOMPONENTS_H

#include <PlatformDetermination.h>
#include "../RemoteConnector.h"

// This defines the number of different connections that can be established, for example each websocket, or each
// tagval connection takes one of these, you can define this to a different number as a build flag. Web server static
// content servers don't count toward this.
#ifndef ALLOWED_CONNECTIONS
#define ALLOWED_CONNECTIONS 4
#endif

namespace tcremote {

    class BaseRemoteServerConnection;

    /**
     * The device initialiser is responsible for initially preparing the hardware communications device for use, and
     * then for creating new connections that can be used with a transport, you provide the transport to it, and the
     * transport provided MUST be of the expected type for this instance.
     */
    class DeviceInitialisation {
    protected:
        bool initialised = false;
    public:
        bool isInitialised() const { return initialised; }
        virtual bool attemptInitialisation()=0;
        virtual bool attemptNewConnection(BaseRemoteServerConnection* remoteConnection)=0;
    };

    /**
     * Some communication devices don't need any initialisation, in these cases this class can be used, it is a NOP
     * implementation that always returns success.
     */
    class NoInitialisationNeeded : public DeviceInitialisation {
    private:
        bool attemptConnectionReturn;
    public:
        explicit NoInitialisationNeeded(bool attemptConnectionReturn = true) : attemptConnectionReturn(attemptConnectionReturn) {}

        bool attemptInitialisation() override {
            initialised = true;
            return true;
        }

        bool attemptNewConnection(BaseRemoteServerConnection *transport) override { return true; }
    };

    enum RemoteServerType: uint8_t {
        TAG_VAL_REMOTE_SERVER, SIMHUB_CONNECTOR, TAG_VAL_WEB_SOCKET
    };

    class BaseRemoteServerConnection {
    protected:
        DeviceInitialisation &initialisation;
        RemoteServerType remoteServerType;
    public:
        BaseRemoteServerConnection(DeviceInitialisation &initialisation, RemoteServerType remoteServerType)
                : initialisation(initialisation), remoteServerType(remoteServerType) {}

        RemoteServerType getRemoteServerType() { return remoteServerType; }

        DeviceInitialisation& getDeviceInitialisation() const { return initialisation; }
        void runLoop();
        virtual void init(int remoteNumber, const ConnectorLocalInfo& info) = 0;
        virtual void tick() = 0;
        virtual bool connected() = 0;
        virtual void copyConnectionStatus(char *buffer, int bufferSize) = 0;
        virtual void notifyRemoteHasClosed() {}
    };

    /**
     * Contains the components of a single connection, which is essentially the remoteConnector and the transport, this
     * allows us to send and receive messages on that transport, connect to it, and determine if it is still connected.
     * It also allows us to establish new connections via the transport.
     */
    class TagValueRemoteServerConnection : public BaseRemoteServerConnection {
    private:
        TagValueRemoteConnector remoteConnector;
        TagValueTransport &remoteTransport;
        CombinedMessageProcessor messageProcessor;
    public:
        TagValueRemoteServerConnection(TagValueTransport &transport, DeviceInitialisation& initialisation);

        void init(int remoteNumber, const ConnectorLocalInfo& info) override;

        TagValueRemoteConnector *connector() { return &remoteConnector; }

        TagValueTransport *transport() { return &remoteTransport; }

        CombinedMessageProcessor *messageProcessors() { return &messageProcessor; }

        void tick() override;
        bool connected() override { return remoteTransport.connected(); }

        void copyConnectionStatus(char *buffer, int bufferSize) override;

        void notifyRemoteHasClosed() override;
    };

    /**
     * This is the component that allows us to manage as many connections as needed using a single instance, it
     * holds on to instances of RemoteServerConnection and services them all, it also provides the getter functions
     * for acquiring the transport or connector for a given item.
     */
    class TcMenuRemoteServer : public Executable {
        BaseRemoteServerConnection* connections[ALLOWED_CONNECTIONS];
        const ConnectorLocalInfo& appInfo;
        uint8_t remotesAdded;
    public:
        /**
         * Creates an instance of the remote server component that has no connections but is properly configured ready
         * for connections to be added. You must provide the application information. The server is started when the
         * first remote is added.
         *
         * @param appInfo the application information - uuid and name basically.
         */
        explicit TcMenuRemoteServer(const ConnectorLocalInfo& appInfo) : connections{}, appInfo(appInfo), remotesAdded(0) { }

        /**
         * Remove all current remotes
         */
        void clearRemotes() {
            remotesAdded = 0;
        }

        void exec() override;

        /**
         * Adds a connection to the managed connections, this class will ensure that it is properly initialised,
         * any authenticator on menuMgr added to it, and will call it's tick method frequently.
         * @param toAdd the connection to add
         * @return a remote number of 0xff if it fails.
         */
        uint8_t addConnection(BaseRemoteServerConnection *toAdd);

        /**
         * @return the number of remote connections added.
         */
        uint8_t remoteCount() const { return remotesAdded; }

        /**
         * Gets the `TagValueRemoteConnector` at the given remoteNo, or nullptr if not available.
         * @param num the remote number
         * @return either a valid object or nullptr
         */
        TagValueRemoteConnector *getRemoteConnector(int num) {
            if(num >= remotesAdded || connections[num]->getRemoteServerType() != TAG_VAL_REMOTE_SERVER) return nullptr;
            return reinterpret_cast<TagValueRemoteServerConnection*>(connections[num])->connector();
        }

        /**
         * Gets the `TagValueTransport` at the given remoteNo, or nullptr if not available.
         * @param num the remote number
         * @return either a valid object or nullptr
         */
        TagValueTransport *getTransport(int num) {
            if(num >= remotesAdded || connections[num]->getRemoteServerType() != TAG_VAL_REMOTE_SERVER) return nullptr;
            return reinterpret_cast<TagValueRemoteServerConnection*>(connections[num])->transport();
        }

        /**
         * Gets the underlying remote connection for external processing, this will be device specific and you should
         * check the type using `getRemoteServerType` before doing anything with the connection.
         * @param num the remote number
         * @return either a pointer to a remote server, or nullptr if there is no such connection.
         */
        BaseRemoteServerConnection* getRemoteServerConnection(int num) {
            if(num >= remotesAdded) return nullptr;
            return connections[num];
        }
    };

    /**
     * Turns a signal strength provided in S/N decibels as an integer into one of four strength icons from 1..4, as the
     * 0 icon is usually to indicate no connection. Where 1 represents a poor connection and 4 represents a good
     * connection.
     * @param strength an integer value in S/N decibels
     * @return the icon to use between 1..4
     */
    int fromWiFiRSSITo4StateIndicator(int strength);

}

#endif //TCMENU_BASEREMOTECOMPONENTS_H
