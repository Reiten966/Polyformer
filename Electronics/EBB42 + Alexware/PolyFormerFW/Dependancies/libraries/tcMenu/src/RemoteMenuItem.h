/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef REMOTE_MENU_ITEMS_H
#define REMOTE_MENU_ITEMS_H

/**
 * @file RemoteMenuItem.h
 *
 * This file contains the extra types needed for remote menu items, they are not in the main MenuItems.h header because
 * they require all the remote headers be included.
 */

#include <PlatformDetermination.h>
#include "MenuItems.h"
#include <RemoteConnector.h>
#include <RemoteAuthentication.h>
#include <remote/BaseRemoteComponents.h>

int remoteInfoRenderFn(RuntimeMenuItem *item, uint8_t row, RenderFnMode mode, char *buffer, int bufferSize);

/**
 * A menu item that holds the current connectivity state of all remote connections registered with it, it is a run time list
 * so needs no additional info structure. For each remote connection that you create, you register it with the addConnector(..)
 * call. This registers this as the callback. If you also need to receive updates, register yourself as a communication listener
 * and you'll receive updates as a pass thru. Note that this object is presently a singleton, one instance should manage all
 * connection state.
 */
class RemoteMenuItem : public ListRuntimeMenuItem {
private:
    tcremote::TcMenuRemoteServer *pRemoteServer;
    CommsCallbackFn passThru;
    const char* pgmName;
    static RemoteMenuItem* instance;
public:
    /**
     * Construct a remote menu item providing the ID, maximum remotes supported and the next item
     */
    RemoteMenuItem(const char* name, menuid_t id, MenuItem *next = nullptr);

    /**
     * Add all connections on a remote server to the list by their connector ID.
     * @param server
     */
    void setRemoteServer(tcremote::TcMenuRemoteServer &server);

    /**
     * Register a pass thru for other items that are also interested in comms updates
     * @param passThru the callback to be called after this item has processed it
     */
    void registerCommsNotification(CommsCallbackFn passThruHandler) {
        this->passThru = passThruHandler;
    }

    /**
     * call the pass thru if it's registered
     * @param info the comms info
     */
    void doPassThru(CommunicationInfo info) {
        if (passThru) passThru(info);
    }

    /**
     * @return the global instance of this object. One list manages all connections.
     */
    static RemoteMenuItem *getInstance() { return instance; }

    friend int remoteInfoRenderFn(RuntimeMenuItem *item, uint8_t row, RenderFnMode mode, char *buffer, int bufferSize);
};

int authenticationMenuItemRenderFn(RuntimeMenuItem *item, uint8_t row, RenderFnMode mode, char *buffer, int bufferSize);

class EepromAuthenticationInfoMenuItem : public ListRuntimeMenuItem {
private:
    const char *pgmName;
    MenuCallbackFn onAuthChanged;
public:
    EepromAuthenticationInfoMenuItem(const char *name, MenuCallbackFn onAuthChanged, menuid_t id,
                                     MenuItem *next = nullptr);
    void init();

    EepromAuthenticatorManager *getAuthManager();

    friend int
    authenticationMenuItemRenderFn(RuntimeMenuItem *item, uint8_t row, RenderFnMode mode, char *buffer, int bufferSize);

    void invokePossibleListener() {
        if (onAuthChanged) onAuthChanged(id);
    }
};

#endif
