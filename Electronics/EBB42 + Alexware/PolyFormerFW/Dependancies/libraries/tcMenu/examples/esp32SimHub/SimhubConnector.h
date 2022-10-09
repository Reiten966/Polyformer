/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * SimHub Connector  that connects to simhub using the serial port and changes menu items based on simhub update.
 * This class should not be directly edited, it will be replaced each time the project is built.
 * If you want to edit this file in place, make sure to rename it first.
 */

#ifndef TCLIBRARYDEV_SIMHUBCONNECTOR_H
#define TCLIBRARYDEV_SIMHUBCONNECTOR_H

/**
 * this is the maximum number of chars on a line
 */
#define MAX_LINE_WIDTH 32

#include <Arduino.h>
#include <TaskManager.h>
#include <HardwareSerial.h>
#include <IoLogging.h>
#include <remote/BaseRemoteComponents.h>

#if defined(ESP8266) || defined(ESP32)
# define SerPortName HardwareSerial
#else
# define SerPortName Stream
#endif

namespace tcremote {

    class SimhubConnector {
    private:
        SerPortName* serialPort;
        BooleanMenuItem* statusMenuItem;
        char lineBuffer[MAX_LINE_WIDTH];
        int linePosition;
        bool connected;
    public:
        SimhubConnector(SerPortName* serialPort, menuid_t statusMenuId);
        void tick();
        bool getStatus() { return connected; }
    private:
        void processCommandFromSimhub();
        void processTcMenuCommand();
        void changeStatus(bool b);
    };


    class SimHubRemoteConnection : public BaseRemoteServerConnection {
    private:
        SimhubConnector connector;
        NoInitialisationNeeded noInitialisation;
    public:
        SimHubRemoteConnection(SerPortName* serialPort, menuid_t statusMenuId)
        : BaseRemoteServerConnection(noInitialisation, SIMHUB_CONNECTOR), connector(serialPort, statusMenuId) {
        }

        void init(int remoteNumber, const ConnectorLocalInfo &info) override;

        void tick() override;

        bool connected() override;

        void copyConnectionStatus(char *buffer, int bufferSize) override;
    };
}

#ifndef TC_MANUAL_NAMESPACING
using namespace tcremote;
#endif // TC_MANUAL_NAMESPACING

#endif //TCLIBRARYDEV_SIMHUBCONNECTOR_H
