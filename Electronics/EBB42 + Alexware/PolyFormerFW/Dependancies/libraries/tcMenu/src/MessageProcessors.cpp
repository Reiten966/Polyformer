/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * MessageProcessors.cpp - standard message processors that decode tcMenu messages.
 */
#include "RemoteConnector.h"
#include "MessageProcessors.h"
#include "MenuIterator.h"
#include "BaseDialog.h"
#include "EditableLargeNumberMenuItem.h"

CombinedMessageProcessor::CombinedMessageProcessor() {
    messageHandlers.add(MsgHandler(MSG_CHANGE_INT, fieldUpdateValueMsg));
    messageHandlers.add(MsgHandler(MSG_JOIN, fieldUpdateJoinMsg));
    messageHandlers.add(MsgHandler(MSG_PAIR, fieldUpdatePairingMsg));
    messageHandlers.add(MsgHandler(MSG_DIALOG, fieldUpdateDialogMsg));
    messageHandlers.add(MsgHandler(MSG_HEARTBEAT, fieldUpdateHeartbeatMsg));
    this->currHandler = nullptr;
}

void CombinedMessageProcessor::newMsg(uint16_t msgType) {
    currHandler = messageHandlers.getByKey(msgType);

    if(currHandler != nullptr) {
        memset(&val, 0, sizeof val);
    } else {
        char sz[3];
        sz[0] = char(msgType>>8);
        sz[1] = char(msgType & 0xff);
        sz[2] = 0;
        serlogF2(SER_WARNING, "No handler - ", sz);
    }
}

void CombinedMessageProcessor::fieldUpdate(TagValueRemoteConnector* connector, FieldAndValue* field) {
    uint16_t mt = field->msgType;
    if(currHandler != nullptr && (connector->isAuthenticated() || mt == MSG_JOIN || mt == MSG_PAIR || mt == MSG_HEARTBEAT)) {
        currHandler->invoke(connector, field, &val);
    }
    else if(mt != MSG_HEARTBEAT) {
        serlogF3(SER_WARNING, "Did not proccess(mt,auth)", field->msgType, connector->isAuthenticated());
    }
}


void fieldUpdateHeartbeatMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) {
		if (info->hb.hbMode == HBMODE_ENDCONNECT) {
			serlogF(SER_NETWORK_INFO, "HB close msg");
			connector->close();
		}
		else if (info->hb.hbMode == HBMODE_STARTCONNECT) {
            serlogF(SER_NETWORK_INFO, "HB start msg");
			connector->encodeJoin();
		}
    } else {
        if(field->field == FIELD_HB_MODE) {
            info->hb.hbMode = (HeartbeatMode)(atoi(field->value));
        }
    }
}

void fieldUpdateDialogMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG && info->dialog.mode == 'A') {
        BaseDialog* dialog = MenuRenderer::getInstance()->getDialog();
        if(dialog) {
            dialog->remoteAction((ButtonType)info->dialog.button);
            connector->encodeAcknowledgement(info->dialog.correlation, ACK_SUCCESS);
        }
        else {
            connector->encodeAcknowledgement(info->dialog.correlation, ACK_UNKNOWN);
        }
        return;
    }

    switch(field->field) {
    case FIELD_BUTTON1:
        info->dialog.button = field->value[0] - '0';
        break;
    case FIELD_MODE:
        info->dialog.mode = field->value[0];
        break;
    case FIELD_CORRELATION:
        info->dialog.correlation = strtoul(field->value, nullptr, 16);
        break;
    }

}

void fieldUpdatePairingMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) return;

    switch(field->field) {
    case FIELD_MSG_NAME:
        strncpy(info->pairing.name, field->value, sizeof(info->pairing.name));
        break;
    case FIELD_UUID:
        serlogF3(SER_NETWORK_INFO, "Pairing request: ", info->pairing.name, field->value);
        connector->pairingRequest(info->pairing.name, field->value);
        break;
    }
}

void fieldUpdateJoinMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) {
        serlogF2(SER_NETWORK_INFO, "Join from ", info->join.platform);
        serlogF3(SER_NETWORK_INFO, "Remote version was ", info->join.major, info->join.minor);

        // ensure that authentication has been done, if not basically fail the connection.
        if(!info->join.authProvided) {
            serlogF(SER_WARNING, "Connection without authentication - stopping");
            connector->provideAuthentication(nullptr);
        }
        else {
            connector->setRemoteConnected(info->join.major, info->join.minor, info->join.platform);
        }
		return;
	}

	switch(field->field) {
	case FIELD_MSG_NAME:
        serlogF2(SER_NETWORK_DEBUG, "Join name ", field->value);
		connector->setRemoteName(field->value);
		break;
	case FIELD_VERSION: {
		int val = atoi(field->value);
		info->join.major = val / 100;
		info->join.minor = val % 100;
		break;
	}
    case FIELD_UUID: {
        serlogF(SER_NETWORK_DEBUG, "Join UUID - start auth");
        connector->provideAuthentication(field->value);
        info->join.authProvided = true;
        break;
    }
	case FIELD_PLATFORM:
		info->join.platform = (ApiPlatform) atoi(field->value);
		break;
	}
}

bool isStringTrue(const char* val) {

    if(val == nullptr || val[0] == 0) return false;
    // digit 1 or Y is true
    if(val[0] == 'Y' || val[0] == '1') return true;
    // first 3 letters of true in any case.
    if(tolower(val[0]) == 't' && tolower(val[1]) == 'r' && tolower(val[2] == 'u')) return true;

    return false;
}

bool processValueChangeField(FieldAndValue* field, MessageProcessorInfo* info) {
    if(info->value.item->getMenuType() == MENUTYPE_INT_VALUE || info->value.item->getMenuType() == MENUTYPE_ENUM_VALUE) {
        auto valItem = (ValueMenuItem*)info->value.item;
        if(info->value.changeType == CHANGE_ABSOLUTE) {
            uint16_t newValue = atol(field->value);
            valItem->setCurrentValue(newValue); // for absolutes, assume other system did checking.
        }
        else if(info->value.changeType == CHANGE_DELTA) {
            // get the delta and current values.
            int deltaVal = atoi(field->value);
            long existingVal = valItem->getCurrentValue();

            // prevent an underflow or overflow situation.
            if((deltaVal < 0 && existingVal < abs(deltaVal)) || (deltaVal > 0 && (existingVal + deltaVal) > valItem->getMaximumValue()) ) {
                return false; // valid update but outside of range.
            }

            // we must be good to go if we get here, write it..
            valItem->setCurrentValue(existingVal + deltaVal);
        }
        serlogF2(SER_NETWORK_DEBUG, "Int change: ", valItem->getCurrentValue());
    }
    else if(info->value.item->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
        // booleans are always absolute
        auto* boolItem = reinterpret_cast<BooleanMenuItem*>(info->value.item);
        boolItem->setBoolean(isStringTrue(field->value));
        serlogF2(SER_NETWORK_DEBUG, "Bool change: ", boolItem->getBoolean());
    }
    else if(info->value.item->getMenuType() == MENUTYPE_TEXT_VALUE) {
        // text is always absolute
        auto* textItem = reinterpret_cast<TextMenuItem*>(info->value.item);
        textItem->setTextValue(field->value);
        serlogF2(SER_NETWORK_DEBUG, "Text change: ", textItem->getTextValue());
    }
    else if (info->value.item->getMenuType() == MENUTYPE_IPADDRESS) {
		auto* ipItem = reinterpret_cast<IpAddressMenuItem*>(info->value.item);
		ipItem->setIpAddress(field->value);
        serlogF2(SER_NETWORK_DEBUG, "Ip Addr change: ", field->value);
	}
    else if(info->value.item->getMenuType() == MENUTYPE_TIME) {
        auto* timeItem = reinterpret_cast<TimeFormattedMenuItem*>(info->value.item);
        timeItem->setTimeFromString(field->value);
        serlogF2(SER_NETWORK_DEBUG, "Time item change: ", field->value);
    }
    else if(info->value.item->getMenuType() == MENUTYPE_DATE) {
        auto* dateItem = reinterpret_cast<DateFormattedMenuItem*>(info->value.item);
        dateItem->setDateFromString(field->value);
        serlogF2(SER_NETWORK_DEBUG, "Date change: ", field->value);
    }
    else if (info->value.item->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
		auto* numItem = reinterpret_cast<EditableLargeNumberMenuItem*>(info->value.item);
		numItem->setLargeNumberFromString(field->value);
        serlogF2(SER_NETWORK_DEBUG, "Large num change: ", field->value);
	}
    else if(info->value.item->getMenuType() == MENUTYPE_COLOR_VALUE) {
        auto rgb = reinterpret_cast<Rgb32MenuItem*>(info->value.item);
        rgb->setColorData(RgbColor32(field->value));
        serlogF4(SER_NETWORK_DEBUG, "RGB change ", rgb->getUnderlying()->red, rgb->getUnderlying()->green, rgb->getUnderlying()->blue);
    }
    else if(info->value.item->getMenuType() == MENUTYPE_SCROLLER_VALUE) {
        auto sc = reinterpret_cast<ScrollChoiceMenuItem*>(info->value.item);
        sc->setFromRemote(field->value);
        serlogF2(SER_NETWORK_DEBUG, "Scroller change ", sc->getCurrentValue());
    }
    else if(info->value.item->getMenuType() == MENUTYPE_RUNTIME_LIST && info->value.changeType == CHANGE_LIST_RESPONSE) {
        auto listItem = reinterpret_cast<ListRuntimeMenuItem*>(info->value.item);
        int offs = 0;
        long idx = parseIntUntilSeparator(field->value, offs);
        bool invoke = parseIntUntilSeparator(field->value, offs) == 1L;
        if(invoke) {
            listItem->getChildItem((int)idx)->triggerCallback();
            // reset to parent after doing the callback
            listItem->asParent();
        }
    }
    return true;
}

bool processIdChangeField(FieldAndValue* field, MessageProcessorInfo* info) {
    int id = atoi(field->value);

    MenuItem* foundItem = getMenuItemById(id);
    if(foundItem != nullptr && !foundItem->isReadOnly()) {
        info->value.item = foundItem;
        serlogF2(SER_NETWORK_DEBUG, "ValChange for ID ", foundItem->getId());
        return true;
    }
    else {
        serlogF(SER_WARNING, "Bad ID on valchange msg");
        info->value.item = nullptr;
        return false;
    }
}

void fieldUpdateValueMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) {
		// if this is an action item, we trigger the callback to occur just before ending.
		if(info->value.item != nullptr && info->value.item->getMenuType() == MENUTYPE_ACTION_VALUE) {
			info->value.item->triggerCallback();
		}
		return;
	}
	
    bool ret;

	switch(field->field) {
    case FIELD_CORRELATION:
        info->value.correlation = strtoul(field->value, nullptr, 16);
        break;
	case FIELD_ID:
        ret = processIdChangeField(field, info);
        if(!ret) connector->encodeAcknowledgement(info->value.correlation, ACK_ID_NOT_FOUND);
		break;
	case FIELD_CURRENT_VAL:
        if(info->value.item != nullptr) {
            ret = processValueChangeField(field, info);
            connector->encodeAcknowledgement(info->value.correlation, ret ? ACK_SUCCESS : ACK_VALUE_RANGE);
        }
		break;
	case FIELD_CHANGE_TYPE:
		info->value.changeType = (ChangeType) atoi(field->value);
		break;
	}
}
