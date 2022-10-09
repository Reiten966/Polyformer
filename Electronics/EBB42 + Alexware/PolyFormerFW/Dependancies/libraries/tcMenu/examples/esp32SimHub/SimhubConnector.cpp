/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * SimHub Connector  that connects to simhub using the serial port and changes menu items based on simhub update.
 * This class should not be directly edited, it will be replaced each time the project is built.
 * If you want to edit this file in place, make sure to rename it first.
 */

#include <MenuItems.h>
#include <MenuIterator.h>
#include <RuntimeMenuItem.h>
#include "SimhubConnector.h"

SimhubConnector::SimhubConnector(SerPortName* serialPort, uint16_t statusMenuId) : lineBuffer {} {
    this->serialPort = serialPort;
    statusMenuItem = (statusMenuId != INVALID_MENU_ID) ? reinterpret_cast<BooleanMenuItem*>(getMenuItemById(statusMenuId)) : nullptr;
    this->linePosition = 0;
    changeStatus(false);
}

void SimhubConnector::tick() {
    while(serialPort && serialPort->available()) {
        if(linePosition >= MAX_LINE_WIDTH) {
            lineBuffer[MAX_LINE_WIDTH - 1] = 0;
            serdebugF2("Error occurred during Rx ", lineBuffer);
            linePosition = 0;
        }

        lineBuffer[linePosition] = (char)serialPort->read();

        if(lineBuffer[linePosition] == '\n') {
            lineBuffer[linePosition] = 0;
            serdebugF2("RX ", lineBuffer);
            // we have received a command from simhub
            processCommandFromSimhub();
            linePosition=0;
        }
        else linePosition++;
    }

}

void SimhubConnector::processCommandFromSimhub() {
    if(isDigit(lineBuffer[0])) {
        processTcMenuCommand();
    }
    else if(strcmp(lineBuffer, "simhubStart") == 0) {
        changeStatus(true);

    }
    else if(strcmp(lineBuffer, "simhubEnd") == 0) {
        changeStatus(false);
    }
}

void SimhubConnector::changeStatus(bool connected) {
    this->connected = connected;
    if(statusMenuItem != nullptr) {
        statusMenuItem->setBoolean(connected);
    }
}

void SimhubConnector::processTcMenuCommand() {
    int i = 0;
    char sz[6];
    while(isDigit(lineBuffer[i]) && i < 5) {
        sz[i] = lineBuffer[i];
        i++;
    };
    sz[i] = 0;
    int menuId = atoi(sz);
    while(lineBuffer[i] != '=') i++;
    char* value = &lineBuffer[i + 1];

    serdebugF4("updch, item, value", sz, menuId, value);

    MenuItem* menuItem = getMenuItemById(menuId);
    if(menuItem->getMenuType() == MENUTYPE_INT_VALUE || menuItem->getMenuType() == MENUTYPE_ENUM_VALUE || menuItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
        auto* valItem = reinterpret_cast<ValueMenuItem*>(menuItem);
        valItem->setCurrentValue(atoi(value));
    }
    else if(menuItem->getMenuType() == MENUTYPE_TEXT_VALUE) {
        auto* txtItem = reinterpret_cast<TextMenuItem*>(menuItem);
        txtItem->setTextValue(value);
    }
}

void SimHubRemoteConnection::init(int remoteNumber, const ConnectorLocalInfo &info) {
    /* currently does nothing, simhub connector presently only receives information */
}

void SimHubRemoteConnection::tick() {
    connector.tick();
}

bool SimHubRemoteConnection::connected() {
    return connector.getStatus();
}

const char pgmSimHubConnectionText[] PROGMEM = "SimHub: ";

void SimHubRemoteConnection::copyConnectionStatus(char *buffer, int bufferSize) {
    if(bufferSize < 11) {
        buffer[0] = 'S'; buffer[1] = 'H';
        buffer[2] = ':'; buffer[3] = connector.getStatus() ? 'Y' : 'N';
        buffer[4] = 0;
    } else {
        safeProgCpy(buffer, pgmSimHubConnectionText, bufferSize);
        appendChar(buffer, connector.getStatus() ? 'Y' : 'N', bufferSize);
    }
}
