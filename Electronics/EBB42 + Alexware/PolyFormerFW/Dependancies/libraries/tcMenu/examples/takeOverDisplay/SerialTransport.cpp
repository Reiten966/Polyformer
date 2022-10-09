/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * Serial remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#include "SerialTransport.h"
#include <tcMenu.h>

SerialTagValueTransport::SerialTagValueTransport(Stream* thePort) : TagValueTransport(TVAL_UNBUFFERED) {
	this->serialPort = thePort;
}

void SerialTagValueTransport::close() {
	currentField.msgType = UNKNOWN_MSG_TYPE;
	currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
}

// DO NOT replace this with the standard char* write method on serial.
// It cannot handle large volumes of data going through at once and often
// overflows the buffer causing data errors.
int SerialTagValueTransport::writeChar(char ch) {
    if(available()) {
        serialPort->write(ch);
    }
    else {
        int tries = 30;
        while(tries && !available()) {
            --tries;
            serialPort->flush();
            taskManager.yieldForMicros(100);
        }

        // if it's not available now, it probably will timeout anyway.
        if(!available()) {
            return 0;
        }

        serialPort->write(ch);
    }
    return 1;
}

int SerialTagValueTransport::writeStr(const char* str) {
    int i=0;
    bool lastWriteOk = true;
    while(str[i] && lastWriteOk) {
        lastWriteOk = writeChar(str[i]);
        i++;
    }
    return i;
}

