/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseBufferedRemoteTransport.h"

using namespace tcremote;

BaseBufferedRemoteTransport::BaseBufferedRemoteTransport(BufferingMode bufferMode, uint8_t readBufferSize, uint8_t writeBufferSize)
        : TagValueTransport(TVAL_BUFFERED), writeBufferSize(writeBufferSize), writeBufferPos(0),
        readBufferSize(readBufferSize), readBufferPos(0), readBufferAvail(0), mode(bufferMode), ticksSinceWrite(0) {
    readBuffer = new uint8_t[readBufferSize];
    writeBuffer = new uint8_t[writeBufferSize];
}

BaseBufferedRemoteTransport::~BaseBufferedRemoteTransport() {
    delete[] readBuffer;
    delete[] writeBuffer;
}

void BaseBufferedRemoteTransport::endMsg() {
    TagValueTransport::endMsg();
    if(mode == BUFFER_ONE_MESSAGE) flush();
}

uint8_t BaseBufferedRemoteTransport::readByte() {
    if(!readAvailable()) return -1;
    auto ch = readBuffer[readBufferPos];
    readBufferPos += 1;
    // only uncomment the below for worst case debugging.
    //serlogF2(SER_DEBUG, "readByte ", ch);
    return ch;
}

bool BaseBufferedRemoteTransport::readAvailable() {
    if(readBufferAvail && readBufferPos < readBufferAvail) {
        return true;
    }

    readBufferAvail = (int8_t)fillReadBuffer(readBuffer, readBufferSize);
    readBufferPos = 0;
    return readBufferPos < readBufferAvail;
}

int BaseBufferedRemoteTransport::writeChar(char data) {
    if(writeBufferPos >= writeBufferSize) {
        // we've exceeded the buffer size so we must flush, and then ensure
        // that flush actually did something and there is now capacity.
        flush();
        if(writeBufferPos >= writeBufferSize) return 0;// we did not write so return an error condition.
    }
    writeBuffer[writeBufferPos++] = data;
    ticksSinceWrite = 0;
    return 1;
}

int BaseBufferedRemoteTransport::writeStr(const char *data) {
    // only uncomment below for worst case debugging..
    //	serlogF2(SER_NETWORK_DEBUG, "writing ", data);

    size_t len = strlen(data);
    for(size_t i = 0; i < len; ++i) {
        if(writeChar(data[i]) == 0) {
            return 0;
        }
    }
    return (int)len;
}

void BaseBufferedRemoteTransport::flushIfRequired() {
    if(!connected() || writeBufferPos == 0 || mode == BUFFER_ONE_MESSAGE) return;

    if(ticksSinceWrite < TICKS_TO_FLUSH_WRITE) ++ticksSinceWrite;
    if(ticksSinceWrite == TICKS_TO_FLUSH_WRITE) {
        ticksSinceWrite = 0xff;
        flush();
    }
}

void BaseBufferedRemoteTransport::close() {
    flush();
    writeBufferPos = 0;
    readBufferPos = 0;
    readBufferAvail = 0;
    currentField.msgType = UNKNOWN_MSG_TYPE;
    currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
}
