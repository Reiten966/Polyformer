/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include <remote/BaseRemoteComponents.h>
#include "RemoteMenuItem.h"
#include "RemoteConnector.h"
#include "tcUtil.h"
#include "BaseDialog.h"

const char REMOVE_CONN_HDR_PGM[] PROGMEM = "Close Connection";

// called when the close connection dialog completes.
void onRemoteInfoDialogComplete(ButtonType btn, void* data) {
	if (btn == BTNTYPE_OK) {
		TagValueRemoteConnector* connector = reinterpret_cast<TagValueRemoteConnector*>(data);
		connector->close();
	}
}

// called for each state of the list, base row and child rows.
int remoteInfoRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    auto* remoteItem = reinterpret_cast<RemoteMenuItem*>(item);
    if(!remoteItem || !remoteItem->pRemoteServer) {
        buffer[0]=0;
        return false;
    }

	switch (mode) {
	case RENDERFN_NAME: {
		if (row < 0xff) {
			buffer[0] = 'R'; buffer[1] = 0;
			fastltoa(buffer, row, 2, NOT_PADDED, bufferSize);
		}
		else {
			safeProgCpy(buffer, remoteItem->pgmName, bufferSize);
		}
		return true;
	}
	case RENDERFN_VALUE: {
		if (row == 0xff) {
			buffer[0] = 0;
			return true;
		}
		auto* baseRemote = remoteItem->pRemoteServer->getRemoteServerConnection(row);
		if (baseRemote == nullptr) {
			buffer[0]=0;
		}
		else {
		    baseRemote->copyConnectionStatus(buffer, bufferSize);
		}
		return true;
	}
	case RENDERFN_INVOKE: {
		TagValueRemoteConnector* connector = remoteItem->pRemoteServer->getRemoteConnector(row);
		BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
		if (connector && dlg && !dlg->isInUse()) {
			dlg->setUserData(connector);
			dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL, 1);
			dlg->show(REMOVE_CONN_HDR_PGM, false, onRemoteInfoDialogComplete);
			dlg->copyIntoBuffer(connector->getRemoteName());
		}
		return true;
	}
	default: return false;
	}
}

void onRemoteItemCommsNotify(CommunicationInfo info) {
    RemoteMenuItem::getInstance()->setSendRemoteNeededAll();
    RemoteMenuItem::getInstance()->setChanged(true);

	// check if the pass thru should be called.
	RemoteMenuItem::getInstance()->doPassThru(info);
}

RemoteMenuItem* RemoteMenuItem::instance = nullptr;

RemoteMenuItem::RemoteMenuItem(const char* pgmName, menuid_t id, MenuItem* next)
	: ListRuntimeMenuItem(id, 0, remoteInfoRenderFn, next), pgmName(pgmName) {
    instance = this;
}

void RemoteMenuItem::setRemoteServer(tcremote::TcMenuRemoteServer& server) {
    pRemoteServer = &server;
    setNumberOfRows(server.remoteCount());

    for(int i=0; i<server.remoteCount();i++) {
        auto* conn = server.getRemoteConnector(i);
        if(conn) conn->setCommsNotificationCallback(onRemoteItemCommsNotify);
    }
}

// EEPROM Authentication manager start

EepromAuthenticationInfoMenuItem::EepromAuthenticationInfoMenuItem(const char* pgmName, MenuCallbackFn onAuthChanged,
                                                                   menuid_t id, MenuItem * next)
	: ListRuntimeMenuItem(id, 0, authenticationMenuItemRenderFn, next), pgmName(pgmName), onAuthChanged(onAuthChanged) {
}

void EepromAuthenticationInfoMenuItem::init() {
    setNumberOfRows(getAuthManager()->getNumberOfEntries());
}

EepromAuthenticatorManager *EepromAuthenticationInfoMenuItem::getAuthManager() {
    auto* authMgr = menuMgr.getAuthenticator();
    if(authMgr->getAuthenticationManagerType() == AUTHENTICATION_IN_EEPROM) {
        return reinterpret_cast<EepromAuthenticatorManager*>(authMgr);
    }
    return nullptr;
}

const char AUTH_REMOVE_KEY[] PROGMEM = "Remove";
const char AUTH_REMOVE_ALL_KEYS[] PROGMEM = "Remove ALL keys?";
const char AUTH_EMPTY_KEY[] PROGMEM = "EmptyKey";

void onAuthenticateRemoveKeysDlgComplete(ButtonType btn, void* data) {
    if (btn == BTNTYPE_OK) {
        auto* mgr = reinterpret_cast<EepromAuthenticatorManager*>(menuMgr.getAuthenticator());
        reinterpret_cast<EepromAuthenticationInfoMenuItem*>(data)->invokePossibleListener();
        mgr->resetAllKeys();
    }
}

int authenticationMenuItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    if(menuMgr.getAuthenticator()->getAuthenticationManagerType() != AUTHENTICATION_IN_EEPROM) {
        buffer[0] = 0;
        return false;
    }

    auto* authItem = reinterpret_cast<EepromAuthenticationInfoMenuItem*>(item);

    switch (mode) {
        case RENDERFN_NAME:
            if (row == 0xff) {
                safeProgCpy(buffer, authItem->pgmName, bufferSize);
            }
            else if(row < authItem->getAuthManager()->getNumberOfEntries()) {
                authItem->getAuthManager()->copyKeyNameToBuffer(row, buffer, bufferSize);
                if (buffer[0] == 0) safeProgCpy(buffer, AUTH_EMPTY_KEY, bufferSize);
            }
            else {
                buffer[0] = 0;
            }
            return true;
        case RENDERFN_VALUE:
            if (row == 0xff) {
                buffer[0] = 0;
            }
            else {
                safeProgCpy(buffer, AUTH_REMOVE_KEY, bufferSize);
            }
            return true;
        case RENDERFN_INVOKE: {
            BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
            if (row < authItem->getAuthManager()->getNumberOfEntries() && dlg && !dlg->isInUse()) {
                dlg->setUserData(authItem);
                dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL, 1);
                dlg->show(AUTH_REMOVE_ALL_KEYS, false, onAuthenticateRemoveKeysDlgComplete);
                dlg->copyIntoBuffer("");
            }
            return true;
        }
        default: return false;
    }
}
