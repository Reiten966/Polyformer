/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file TcMenuVersion.h contains the version information and accessor functions.
 */

#ifndef TCMENU_VERSION_H
#define TCMENU_VERSION_H

#include "tcUtil.h"

namespace tccore {

// here we define the version as both a string and separate field
#define TCMENU_MAJOR 2
#define TCMENU_MINOR 4
#define TCMENU_PATCH 0

/**
 * A helper to generate the major minor version numbers used in the protocol
 */
#define majorminor(maj, min) ((maj * 100) + min)

/**
 * Definition of the current API version
 */
#define API_VERSION majorminor(TCMENU_MAJOR, TCMENU_MINOR)

    inline void copyTcMenuVersion(char* buffer, size_t bufferSize) {
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, TCMENU_MAJOR, 3, NOT_PADDED, bufferSize);
        appendChar(buffer, '.', bufferSize);
        fastltoa(buffer, TCMENU_MINOR, 3, NOT_PADDED, bufferSize);
        appendChar(buffer, '.', bufferSize);
        fastltoa(buffer, TCMENU_PATCH, 3, NOT_PADDED, bufferSize);
    }

}

#endif //TCMENU_VERSION_H
