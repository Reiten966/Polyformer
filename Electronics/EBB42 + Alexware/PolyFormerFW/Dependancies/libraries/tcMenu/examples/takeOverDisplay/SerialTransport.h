/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file SerialTransport.h
 * 
 * Serial remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#ifndef TCMENU_SERIALTRANSPORT_H_
#define TCMENU_SERIALTRANSPORT_H_

#include <Arduino.h>
#include <RemoteConnector.h>
#include <MessageProcessors.h>
#include <tcUtil.h>
#include <RemoteAuthentication.h>
#include <remote/BaseRemoteComponents.h>

namespace tcremote {

    /**
     * Serial transport is an implementation of TagValueTransport that works over a serial port
     */
    class SerialTagValueTransport : public TagValueTransport {
    private:
        Stream* serialPort;
    public:
        explicit SerialTagValueTransport(Stream* thePort);
        ~SerialTagValueTransport() override = default;

        void flush() override {serialPort->flush();}
        int writeChar(char data) override;
        int writeStr(const char* data) override;

        uint8_t readByte() override { return serialPort->read(); }
        bool readAvailable() override { return serialPort->available(); }
        bool available() override { return serialPort->availableForWrite() != 0;}
        bool connected() override { return true;}

        void close() override;
    };
}

#ifndef TC_MANUAL_NAMESPACING
using namespace tcremote;
#endif // TC_MANUAL_NAMESPACING

#endif /* TCMENU_SERIALTRANSPORT_H_ */
