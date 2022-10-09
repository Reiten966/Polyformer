/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * RemoteConnector.cpp - contains the base functionality for communication between the menu library
 * and remote APIs.
 */

#include <PlatformDetermination.h>
#include "tcMenuVersion.h"
#ifdef IOA_USE_MBED
#include "tcUtil.h"
#endif

#include <stdlib.h>
#include "RemoteConnector.h"
#include "TaskManager.h"
#include <IoLogging.h>
#include "BaseDialog.h"
#include "EditableLargeNumberMenuItem.h"

const char PGM_TCM pmemBootStartText[] = "START";
const char PGM_TCM pmemBootEndText[] = "END";

#ifdef IO_LOGGING_DEBUG
// utility function to write out a debug line with the message type attached.
inline void logMessageHeader(const char* tx, int remoteNo, uint16_t msgType) {
    char sz[3];
    sz[0] = char(msgType>>8);
    sz[1] = char(msgType & 0xff);
    sz[2] = 0;
    serlog2(SER_NETWORK_DEBUG, remoteNo, sz);
}
#else
#define logMessageHeader(x, y, z)
#endif

void stopPairing();

TagValueRemoteConnector::TagValueRemoteConnector(uint8_t remoteNo) :
        bootPredicate(MENUTYPE_BACK_VALUE, TM_INVERTED_LOCAL_ONLY),
        remotePredicate(remoteNo), remoteName{}, remoteMajorVer(0), remoteMinorVer(0),
        remotePlatform(PLATFORM_ARDUINO_8BIT) {
	this->transport = nullptr;
	this->processor = nullptr;
	this->localInfoPgm = nullptr;
	this->remoteNo = remoteNo;
	this->ticksLastRead = this->ticksLastSend = 0xffff;
	this->flags = 0;
    this->commsCallback = nullptr;
    this->authManager = nullptr;
}

void TagValueRemoteConnector::initialise(TagValueTransport* transport_, CombinedMessageProcessor* processor_,
                                         const ConnectorLocalInfo* localInfoPgm_, uint8_t remoteNo_=0) {
    this->processor = processor_;
    this->transport = transport_;
    this->localInfoPgm = localInfoPgm_;
    this->remoteNo = remoteNo_;
    this->remotePredicate.setRemoteNo(remoteNo_);

    // we must always have a mode of authentication, if nothing has been set then get the one from menuMgr as a backup.
    if(this->authManager == nullptr) authManager = menuMgr.getAuthenticator();
}

void TagValueRemoteConnector::setRemoteName(const char* name) {
	strncpy(remoteName, name, sizeof remoteName);
	remoteName[sizeof(remoteName)-1]=0;
}

void TagValueRemoteConnector::setRemoteConnected(uint8_t major, uint8_t minor, ApiPlatform platform) {
    if(isAuthenticated()) {
        serlogF(SER_NETWORK_INFO, "Fully authenticated connection");
        remoteMajorVer = major;
        remoteMinorVer = minor;
        remotePlatform = platform;
		setFullyJoinedRx(true);
        initiateBootstrap();
    }
    else {
        serlogF(SER_NETWORK_INFO, "Not authenticated, dropping");
        close();
    }
}

void TagValueRemoteConnector::provideAuthentication(const char* auth) {
    if(auth == nullptr || !authManager->isAuthenticated(remoteName, auth)) {
        serlogF2(SER_NETWORK_INFO, "Authentication failed for ", remoteName);
        // wait before returning the state to prevent denial of service.
        encodeAcknowledgement(0, ACK_CREDENTIALS_INVALID);
        taskManager.yieldForMicros(15000); 
        setAuthenticated(false);
        close();
        commsNotify(COMMSERR_DISCONNECTED);
    }
    else {
        encodeAcknowledgement(0, ACK_SUCCESS);
        setAuthenticated(true);
        serlogF2(SER_NETWORK_INFO, "Authenticated device ", remoteName);
        commsNotify(COMMSERR_CONNECTED);
    }
}

const char headerPairingText[] PROGMEM = "Pairing waiting";
const char headerPairingDone[] PROGMEM = "Pairing complete";
const char* lastUuid;

void onPairingFinished(ButtonType ty, void* voidConnector) {
    auto* connector = reinterpret_cast<TagValueRemoteConnector*>(voidConnector);
    if(ty==BTNTYPE_ACCEPT) {
        serlogF(SER_NETWORK_INFO, "Adding key after user pressed accept");
        bool added = connector->getAuthManager()->addAdditionalUUIDKey(connector->getRemoteName(), lastUuid);
        connector->encodeAcknowledgement(0, added ? ACK_SUCCESS : ACK_CREDENTIALS_INVALID);
    }
    else {
        serlogF(SER_NETWORK_INFO, "Not adding key, close pressed");
        connector->encodeAcknowledgement(0, ACK_CREDENTIALS_INVALID);
    }
} 

void stopPairing() {
    BaseDialog* dialog = MenuRenderer::getInstance()->getDialog();
    if(dialog->isInUse()) dialog->hide();
}

void TagValueRemoteConnector::pairingRequest(const char* name, const char* uuid) {
    BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
    if(!dlg) {
        // TODO, special handling for where there's no display locally.
        return;
    }
    // store the fields
    lastUuid = uuid;
    setRemoteName(name);

    if(!isPairing()) {
        // mark this connection as paring only until disconnected
        // this prevents any other use of the connection basically.
        setPairing(true);

        // show the dialog.
        dlg->setButtons(BTNTYPE_ACCEPT, BTNTYPE_CANCEL, 1);
        dlg->setUserData(this);
        dlg->show(headerPairingText, false, onPairingFinished);
    }
    dlg->copyIntoBuffer(name);
}

void TagValueRemoteConnector::commsNotify(uint16_t err) {
    if(commsCallback != nullptr) {
        CommunicationInfo info;
        info.remoteNo = remoteNo;
        info.connected = isAuthenticated();
        info.errorMode = err;
        commsCallback(info);
    }
}

void TagValueRemoteConnector::close() {
	serlogF(SER_NETWORK_INFO, "Closing connection");
	if (transport->connected()) {
		encodeHeartbeat(HBMODE_ENDCONNECT);
	}
	transport->close();

	if (isPairing()) stopPairing();
	setConnected(false);
	setPairing(false);

}

void TagValueRemoteConnector::tick() {
    dealWithHeartbeating();

	if(isConnected() && transport->connected() && isAuthenticated()) {
		performAnyWrites();
	}

    if(isPairing()) return; // never read anything else when in pairing mode.

    // field if available is kind of like a state machine. Due to limited memory
    // on some AVR's and problems with the size of virtual tables, it cannot be
    // implemented as a series of classes that implement an interface.
    //
    // Every tick we call the method and it processes whatever data is available
    // on the socket turning it into a message a field at a time.
	FieldAndValue* field = transport->fieldIfAvailable();
	switch(field->fieldType) {
	case FVAL_NEW_MSG:
        logMessageHeader("Msg In S: ", remoteNo, field->msgType);
		processor->newMsg(field->msgType);
		break;
	case FVAL_FIELD:
        logMessageHeader("Fld: ", remoteNo, field->field);
		processor->fieldUpdate(this, field);
        break;
	case FVAL_END_MSG:
        logMessageHeader("Msg In E: ", remoteNo, field->msgType);
		processor->fieldUpdate(this, field);
		ticksLastRead = 0;
		break;
	case FVAL_ERROR_PROTO:
		commsNotify(COMMSERR_PROTOCOL_ERROR);
		break;
	default: // not ready for processing yet.
		break;
	}
}

void TagValueRemoteConnector::setConnected(bool conn) {
    if(!conn) {
        this->ticksLastRead = this->ticksLastSend = 0xffff;
        flags = 0; // clear all flags on disconnect.
    }
    else {
        bitWrite(flags, FLAG_CURRENTLY_CONNECTED, true);
    } 

    commsNotify(conn ? COMMSERR_CONNECTED : COMMSERR_DISCONNECTED);
}

void TagValueRemoteConnector::dealWithHeartbeating() {
	++ticksLastRead;
	++ticksLastSend;

    // pairing will not send heartbeats, so we wait about 10 seconds before closing out
    unsigned int maximumWaitTime = isPairing() ? (PAIRING_TIMEOUT_TICKS) : (HEARTBEAT_INTERVAL_TICKS * 3);

	if(ticksLastRead > maximumWaitTime && isConnected()) {
        serlogF3(SER_NETWORK_INFO, "Remote disconnected (rNo, ticks): ", remoteNo, ticksLastSend);
        close();
        return;
	}

    if(!isConnected() && transport->connected()) {
        serlogF2(SER_NETWORK_INFO, "Remote connected: ", remoteNo);
        encodeHeartbeat(HBMODE_STARTCONNECT);
		setConnected(true);
	}

	if(ticksLastSend > HEARTBEAT_INTERVAL_TICKS) {
		if(isConnectionFullyEstablished() && transport->available()) {
            serlogF3(SER_NETWORK_INFO, "Sending HB (rNo, ticks) : ", remoteNo, ticksLastSend);
            encodeHeartbeat(HBMODE_NORMAL);
        }
	}
}

void TagValueRemoteConnector::performAnyWrites() {
	if(isBootstrapMode()) {
		nextBootstrap();
	}
	else if(isBootstrapComplete()) {
        MenuItem* item = iterator.nextItem();
        if(item && MENUTYPE_SUB_VALUE != item->getMenuType()) {
            item->setSendRemoteNeeded(remoteNo, false);
            encodeChangeValue(item);
        }

        BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
        if(dlg!=nullptr && dlg->isRemoteUpdateNeeded(remoteNo)) {
            dlg->encodeMessage(this);
            dlg->setRemoteUpdateNeeded(remoteNo, false);
        }
    }
}

void TagValueRemoteConnector::initiateBootstrap() {
    serlogF2(SER_NETWORK_INFO, "Starting bootstrap mode", remoteNo);
    iterator.reset();
    iterator.setPredicate(&bootPredicate);
	encodeBootstrap(false);
	setBootstrapMode(true);
    setBootstrapComplete(false);
}

void TagValueRemoteConnector::nextBootstrap() {
	if(!transport->available()) return; // skip a turn, no write available.

    MenuItem* bootItem = iterator.nextItem();
	MenuItem* parent = iterator.currentParent() ;
    int parentId = parent == nullptr ? 0 : parent->getId();
	if(!bootItem) {
        serlogF2(SER_NETWORK_INFO, "Finishing bootstrap mode", remoteNo);
		setBootstrapMode(false);
        setBootstrapComplete(true);
		encodeBootstrap(true);
        encodeHeartbeat(HBMODE_NORMAL);
        iterator.reset();
        iterator.setPredicate(&remotePredicate);
		return;
	}

	bootItem->setSendRemoteNeeded(remoteNo, false);
	switch(bootItem->getMenuType()) {
	case MENUTYPE_SUB_VALUE:
		encodeSubMenu(parentId, (SubMenuItem*)bootItem);
		break;
	case MENUTYPE_BOOLEAN_VALUE:
		encodeBooleanMenu(parentId, (BooleanMenuItem*)bootItem);
		break;
	case MENUTYPE_ENUM_VALUE:
		encodeEnumMenu(parentId, (EnumMenuItem*)bootItem);
		break;
	case MENUTYPE_INT_VALUE:
		encodeAnalogItem(parentId, (AnalogMenuItem*)bootItem);
		break;
	case MENUTYPE_IPADDRESS:
	case MENUTYPE_TEXT_VALUE:
    case MENUTYPE_TIME:
    case MENUTYPE_DATE:
		encodeMultiEditMenu(parentId, reinterpret_cast<RuntimeMenuItem*>(bootItem));
		break;
	case MENUTYPE_LARGENUM_VALUE:
		encodeLargeNumberMenuItem(parentId, reinterpret_cast<EditableLargeNumberMenuItem*>(bootItem));
		break;
	case MENUTYPE_RUNTIME_LIST:
	case MENUTYPE_RUNTIME_VALUE:
		encodeRuntimeMenuItem(parentId, reinterpret_cast<RuntimeMenuItem*>(bootItem));
		break;
	case MENUTYPE_FLOAT_VALUE:
		encodeFloatMenu(parentId, (FloatMenuItem*)bootItem);
		break;
	case MENUTYPE_ACTION_VALUE:
		encodeActionMenu(parentId, (ActionMenuItem*)bootItem);
		break;
	case MENUTYPE_COLOR_VALUE:
	    encodeColorMenuItem(parentId, reinterpret_cast<Rgb32MenuItem*>(bootItem));
	    break;
	case MENUTYPE_SCROLLER_VALUE:
	    encodeScrollMenuItem(parentId, reinterpret_cast<ScrollChoiceMenuItem*>(bootItem));
	    break;
	default:
		break;
	}
}

void TagValueRemoteConnector::encodeDialogMsg(uint8_t mode, uint8_t btn1, uint8_t btn2, const char* header, const char* b1) {
	if(!prepareWriteMsg(MSG_DIALOG)) return;

	char buffer[2];
    buffer[0]=(char)mode;
    buffer[1]=0;
    transport->writeField(FIELD_MODE, buffer);

    transport->writeFieldInt(FIELD_BUTTON1, btn1);
    transport->writeFieldInt(FIELD_BUTTON2, btn2);
    
    if(mode == 'S') {
        transport->writeField(FIELD_HEADER, header);
        transport->writeField(FIELD_BUFFER, b1);
    }
    transport->endMsg();
}

void TagValueRemoteConnector::encodeCustomTagValMessage(uint16_t msgType, void (*msgWriter)(TagValueTransport*)) {
    if(!prepareWriteMsg(msgType)) return;
    msgWriter(transport);
    transport->endMsg();
    serlogF2(SER_NETWORK_INFO, "Custom message write complete", msgType);
}

void TagValueRemoteConnector::encodeJoin() {
	if(!prepareWriteMsg(MSG_JOIN)) return;
    char szName[40];
    safeProgCpy(szName, localInfoPgm->uuid, sizeof(szName));
    transport->writeField(FIELD_UUID, szName);
    safeProgCpy(szName, localInfoPgm->name, sizeof(szName));
    transport->writeField(FIELD_MSG_NAME, szName);
    transport->writeFieldInt(FIELD_VERSION, API_VERSION);
    transport->writeFieldInt(FIELD_PLATFORM, TCMENU_DEFINED_PLATFORM);
    transport->endMsg();
	setFullyJoinedTx(true);
    serlogF2(SER_NETWORK_INFO, "Join sent ", szName);
}

void TagValueRemoteConnector::encodeBootstrap(bool isComplete) {
	if(!prepareWriteMsg(MSG_BOOTSTRAP)) return;
    transport->writeField(FIELD_BOOT_TYPE, potentialProgramMemory(isComplete ?  pmemBootEndText : pmemBootStartText));
    transport->endMsg();
}

void TagValueRemoteConnector::encodeHeartbeat(HeartbeatMode hbMode) {
	if(!prepareWriteMsg(MSG_HEARTBEAT)) return;
    transport->writeFieldInt(FIELD_HB_INTERVAL, HEARTBEAT_INTERVAL);
    transport->writeFieldLong(FIELD_HB_MILLISEC, millis());
    transport->writeFieldInt(FIELD_HB_MODE, hbMode);
    transport->endMsg();
}

bool TagValueRemoteConnector::prepareWriteMsg(uint16_t msgType) {
    if(!transport->connected()) {
        logMessageHeader("Wr Err ", remoteNo, msgType);
        commsNotify(COMMSERR_WRITE_NOT_CONNECTED);
        setConnected(false); // we are immediately not connected in this case.
        return false;
    }
    transport->startMsg(msgType);
    ticksLastSend = 0;
    logMessageHeader("Msg Out ", remoteNo, msgType);
    return true;
}

void TagValueRemoteConnector::encodeBaseMenuFields(int parentId, MenuItem* item) {
    transport->writeFieldInt(FIELD_PARENT, parentId);
    transport->writeFieldInt(FIELD_ID, item->getId());
    transport->writeFieldInt(FIELD_EEPROM, item->getEepromPosition());
    transport->writeFieldInt(FIELD_READONLY, item->isReadOnly());
    transport->writeFieldInt(FIELD_VISIBLE, item->isVisible());
    char sz[20];
    item->copyNameToBuffer(sz, sizeof(sz));
    transport->writeField(FIELD_MSG_NAME, sz);
}

void TagValueRemoteConnector::encodeAnalogItem(int parentId, AnalogMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_ANALOG)) return;
    encodeBaseMenuFields(parentId, item);
    char sz[10];
    item->copyUnitToBuffer(sz);
    transport->writeField(FIELD_ANALOG_UNIT, sz);
    transport->writeFieldInt(FIELD_ANALOG_MAX, item->getMaximumValue());
    transport->writeFieldInt(FIELD_ANALOG_OFF, item->getOffset());
    transport->writeFieldInt(FIELD_ANALOG_DIV, item->getDivisor());
    transport->writeFieldInt(FIELD_ANALOG_STEP, item->getStep());
    transport->writeFieldInt(FIELD_CURRENT_VAL, item->getCurrentValue());
    transport->endMsg();
}

void TagValueRemoteConnector::encodeScrollMenuItem(int id, ScrollChoiceMenuItem *pItem) {
    if(!prepareWriteMsg(MSG_BOOT_SCROLL_CHOICE)) return;
    encodeBaseMenuFields(id, pItem);
    transport->writeFieldInt(FIELD_NO_CHOICES, pItem->getNumberOfRows());
    transport->writeFieldInt(FIELD_WIDTH, pItem->getItemWidth());
    transport->writeFieldInt(FIELD_EDIT_MODE, pItem->getMemMode());

    char sz[32];
    pItem->copyTransportText(sz, sizeof(sz));

    transport->writeField(FIELD_CURRENT_VAL, sz);

    transport->endMsg();
}

void TagValueRemoteConnector::encodeColorMenuItem(int id, Rgb32MenuItem *pItem) {
    if(!prepareWriteMsg(MSG_BOOT_RGB_COLOR)) return;
    encodeBaseMenuFields(id, pItem);
    transport->writeFieldInt(FIELD_ALPHA, pItem->isAlphaInUse());
    char sz[12];
    pItem->getUnderlying()->asHtmlString(sz, sizeof sz, true);
    transport->writeField(FIELD_CURRENT_VAL, sz);
    transport->endMsg();
}

void TagValueRemoteConnector::encodeLargeNumberMenuItem(int parentId, EditableLargeNumberMenuItem* item) {
	if (!prepareWriteMsg(MSG_BOOT_LARGENUM)) return;
	encodeBaseMenuFields(parentId, item);
	transport->writeFieldInt(FIELD_FLOAT_DP, item->getLargeNumber()->decimalPointIndex());
	transport->writeFieldInt(FIELD_MAX_LEN, item->getNumberOfParts());
	char sz[20];
	item->copyValue(sz, sizeof(sz));
	transport->writeField(FIELD_CURRENT_VAL, sz);
    transport->endMsg();
}

void TagValueRemoteConnector::encodeMultiEditMenu(int parentId, RuntimeMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_TEXT)) return;
    encodeBaseMenuFields(parentId, item);

	if (item->getMenuType() == MENUTYPE_TEXT_VALUE) {
		transport->writeFieldInt(FIELD_EDIT_MODE, EDITMODE_PLAIN_TEXT);
	}
	else if (item->getMenuType() == MENUTYPE_IPADDRESS) {
		transport->writeFieldInt(FIELD_EDIT_MODE, EDITMODE_IP_ADDRESS);
	}
    else if(item->getMenuType() == MENUTYPE_TIME) {
        auto* editable = reinterpret_cast<TimeFormattedMenuItem*>(item);
        transport->writeFieldInt(FIELD_EDIT_MODE, editable->getFormat());
    }
    else if(item->getMenuType() == MENUTYPE_DATE) {
        transport->writeFieldInt(FIELD_EDIT_MODE, EDITMODE_GREGORIAN_DATE);
    }

    auto* multipart = reinterpret_cast<EditableMultiPartMenuItem*>(item);
    char sz[20];
    multipart->copyValue(sz, sizeof(sz));
    transport->writeField(FIELD_CURRENT_VAL, sz);
    transport->writeFieldInt(FIELD_MAX_LEN, multipart->getNumberOfParts());

    transport->endMsg();
}

void writeFloatValueToTransport(TagValueTransport* transport, FloatMenuItem* item) {
	char sz[20];
	sz[0]=0;
	fastftoa(sz, item->getFloatValue(), item->getDecimalPlaces(), sizeof sz);
	transport->writeField(FIELD_CURRENT_VAL, sz);
}

void TagValueRemoteConnector::encodeFloatMenu(int parentId, FloatMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_FLOAT)) return;
    encodeBaseMenuFields(parentId, item);
    transport->writeFieldInt(FIELD_FLOAT_DP, item->getDecimalPlaces());
    writeFloatValueToTransport(transport, item);
    transport->endMsg();
}

void runtimeSendList(ListRuntimeMenuItem* item, TagValueTransport* transport) {
	char sz[25];
	for (int i = 0; i < item->getNumberOfParts(); i++) {
		item->getChildItem(i);
		item->copyValue(sz, sizeof(sz));
		transport->writeField(msgFieldToWord(FIELD_PREPEND_CHOICE, 'A' + i), sz);
		item->copyNameToBuffer(sz, sizeof(sz));
		transport->writeField(msgFieldToWord(FIELD_PREPEND_NAMECHOICE, 'A' + i), sz);
	}
	item->asParent();
}

void TagValueRemoteConnector::encodeRuntimeMenuItem(int parentId, RuntimeMenuItem * item) {
	if (!prepareWriteMsg(MSG_BOOT_LIST)) return;
	transport->writeFieldInt(FIELD_NO_CHOICES, item->getNumberOfParts());
	if (item->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		runtimeSendList(reinterpret_cast<ListRuntimeMenuItem*>(item), transport);
	}
	else {
		char sz[25];
		item->copyValue(sz, sizeof(sz));
		transport->writeField(FIELD_PREPEND_CHOICE | 'A', sz);
	}
	encodeBaseMenuFields(parentId, item);
	transport->endMsg();
}

void TagValueRemoteConnector::encodeEnumMenu(int parentId, EnumMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_ENUM)) return;
    encodeBaseMenuFields(parentId, item);
    transport->writeFieldInt(FIELD_CURRENT_VAL, item->getCurrentValue());
    uint8_t noChoices = item->getMaximumValue() + 1;
    transport->writeFieldInt(FIELD_NO_CHOICES, noChoices);
    for(uint8_t i=0;i<noChoices;++i) {
        uint16_t choiceKey = msgFieldToWord(FIELD_PREPEND_CHOICE, 'A' + i);
        char szChoice[20];
        item->copyEnumStrToBuffer(szChoice, sizeof(szChoice), i);
        transport->writeField(choiceKey, szChoice);
    }
    transport->endMsg();
}

void TagValueRemoteConnector::encodeAcknowledgement(uint32_t correlation, AckResponseStatus status) {
    if(!prepareWriteMsg(MSG_ACKNOWLEDGEMENT)) return;
    transport->writeFieldInt(FIELD_ACK_STATUS, status);
    char sz[10];
    ltoa(correlation, sz, 16);
    transport->writeField(FIELD_CORRELATION, sz);
    transport->endMsg();

    serlogF3(SER_NETWORK_INFO, "Ack send: ", correlation, status);
}

void TagValueRemoteConnector::encodeBooleanMenu(int parentId, BooleanMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_BOOL)) return;
    encodeBaseMenuFields(parentId, item);
    transport->writeFieldInt(FIELD_CURRENT_VAL, item->getCurrentValue());
    transport->writeFieldInt(FIELD_BOOL_NAMING, item->getBooleanNaming());
    transport->endMsg();
}

void TagValueRemoteConnector::encodeSubMenu(int parentId, SubMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_SUBMENU)) return;
    encodeBaseMenuFields(parentId, item);
    transport->endMsg();
}

void TagValueRemoteConnector::encodeActionMenu(int parentId, ActionMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_ACTION)) return;
    encodeBaseMenuFields(parentId, item);
    transport->endMsg();
}

void TagValueRemoteConnector::encodeChangeValue(MenuItem* theItem) {
    char sz[32];
    if(!prepareWriteMsg(MSG_CHANGE_INT)) return;
    transport->writeFieldInt(FIELD_ID, theItem->getId());
    switch(theItem->getMenuType()) {
    case MENUTYPE_ENUM_VALUE:
    case MENUTYPE_INT_VALUE:
    case MENUTYPE_BOOLEAN_VALUE:
        transport->writeFieldInt(FIELD_CHANGE_TYPE, CHANGE_ABSOLUTE); // menu host always sends absolute!
        transport->writeFieldInt(FIELD_CURRENT_VAL, ((ValueMenuItem*)theItem)->getCurrentValue());
        break;
	case MENUTYPE_COLOR_VALUE: {
	    auto rgb = reinterpret_cast<Rgb32MenuItem*>(theItem);
	    rgb->getUnderlying()->asHtmlString(sz, sizeof sz, true);
	    transport->writeField(FIELD_CURRENT_VAL, sz);
	    transport->writeFieldInt(FIELD_CHANGE_TYPE, CHANGE_ABSOLUTE);
	    break;
	}
	case MENUTYPE_SCROLLER_VALUE: {
	    auto sc = reinterpret_cast<ScrollChoiceMenuItem*>(theItem);
	    sc->copyTransportText(sz, sizeof sz);
	    transport->writeField(FIELD_CURRENT_VAL, sz);
	    transport->writeFieldInt(FIELD_CHANGE_TYPE, CHANGE_ABSOLUTE);
	    break;
	}
    case MENUTYPE_IPADDRESS:
    case MENUTYPE_TIME:
    case MENUTYPE_DATE:
    case MENUTYPE_LARGENUM_VALUE:
    case MENUTYPE_TEXT_VALUE: {
		((RuntimeMenuItem*)theItem)->copyValue(sz, sizeof(sz));
		transport->writeField(FIELD_CURRENT_VAL, sz);
        transport->writeFieldInt(FIELD_CHANGE_TYPE, CHANGE_ABSOLUTE); // menu host always sends absolute!
		break;
	}
	case MENUTYPE_RUNTIME_LIST:
        transport->writeFieldInt(FIELD_CHANGE_TYPE, CHANGE_LIST); // menu host always sends absolute!
		runtimeSendList(reinterpret_cast<ListRuntimeMenuItem*>(theItem), transport);
		break;
	case MENUTYPE_FLOAT_VALUE:
        transport->writeFieldInt(FIELD_CHANGE_TYPE, CHANGE_ABSOLUTE); // menu host always sends absolute!
        writeFloatValueToTransport(transport, (FloatMenuItem*)theItem);
        break;
    default:
        break;
    }
    transport->endMsg();
}

//
// Base transport capabilities
//

TagValueTransport::TagValueTransport(TagValueTransportType tvType) {
	this->currentField.field = UNKNOWN_FIELD_PART;
	this->currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
	this->currentField.msgType = UNKNOWN_MSG_TYPE;
	this->currentField.len = 0;
	this->transportType = tvType;
	this->protocolUsed = TAG_VAL_PROTOCOL;
}

void TagValueTransport::startMsg(uint16_t msgType) {
	// start of message
	writeChar(START_OF_MESSAGE);

	// protocol byte
	writeChar(protocolUsed);

    // message type high then low
	writeChar(char(msgType >> 8));
	writeChar(char(msgType & 0xff));
}

void TagValueTransport::writeField(uint16_t field, const char* value) {
	char sz[4];
	sz[0] = char(field >> 8);
	sz[1] = char(field & 0xff);
	sz[2] = '=';
	sz[3] = 0;
	writeStr(sz);
	writeStr(value);
	writeChar('|');
}

void TagValueTransport::writeFieldInt(uint16_t field, int value) {
	char sz[10];
	sz[0] = char(field >> 8);
	sz[1] = char(field & 0xff);
	sz[2] = '=';
	sz[3] = 0;
	writeStr(sz);
	itoa(value, sz, 10);
	writeStr(sz);
	writeChar('|');
}

void TagValueTransport::writeFieldLong(uint16_t field, long value) {
	char sz[12];
	sz[0] = char(field >> 8);
	sz[1] = char(field & 0xff);
	sz[2] = '=';
	sz[3] = 0;
	writeStr(sz);
	ltoa(value, sz, 10);
	writeStr(sz);
	writeChar('|');
}

void TagValueTransport::endMsg() {
	writeChar(0x02);
}

void TagValueTransport::clearFieldStatus(FieldValueType ty) {
	currentField.fieldType = ty;
	currentField.field = UNKNOWN_FIELD_PART;
	currentField.msgType = UNKNOWN_MSG_TYPE;
}

bool TagValueTransport::findNextMessageStart() {
	char read = 0;
	while(readAvailable() && read != START_OF_MESSAGE) {
		read = readByte();
	}
	return (read == START_OF_MESSAGE);
}

bool TagValueTransport::processMsgKey() {
	if(highByte(currentField.field) == UNKNOWN_FIELD_PART && readAvailable()) {
		char r = readByte();
		if(r == 0x02) {
			currentField.fieldType = FVAL_END_MSG;
			return false;
		}
		else {
			currentField.field = ((uint16_t)r) << 8;
		}
	}

	// if we are PROCESSING the key and we've already filled in the top half, then now we need the lower part.
	if(highByte(currentField.field) != UNKNOWN_FIELD_PART && lowByte(currentField.field) == UNKNOWN_FIELD_PART && readAvailable()) {
		currentField.field |= ((readByte()) & 0xff);
		currentField.fieldType = FVAL_PROCESSING_WAITEQ;
	}

	return true;
}

bool TagValueTransport::processValuePart() {
	char current = 0;
	while(readAvailable() && current != '|') {
		current = readByte();
		if(current != '|') {
			currentField.value[currentField.len] = current;
			// safety check for too much data!
			if(++currentField.len > (sizeof(currentField.value)-1)) {
				return false;
			}
		}
	}

	// reached end of field?
	if(current == '|') {
		currentField.value[currentField.len] = 0;
         currentField.fieldType = FVAL_FIELD;
	}
	return true;
}

FieldAndValue* TagValueTransport::fieldIfAvailable() {
    // don't start processing below when not connected or available
    if(!connected()) {
        clearFieldStatus(FVAL_PROCESSING_AWAITINGMSG);
        return &currentField;
    } 

	bool contProcessing = true;
	while(contProcessing) {
		switch(currentField.fieldType) {
		case FVAL_END_MSG:
			clearFieldStatus(FVAL_PROCESSING_AWAITINGMSG);
			break;
		case FVAL_ERROR_PROTO:
		case FVAL_PROCESSING_AWAITINGMSG: // in these states we need to find the next message
			if(findNextMessageStart()) {
				clearFieldStatus(FVAL_PROCESSING_PROTOCOL);
			}
			else {
				currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
				return &currentField;
			}
			break;

		case FVAL_PROCESSING_PROTOCOL: // we need to make sure the protocol is valid
			if(readAvailable()) {
                if(readByte() == protocolUsed) {
                    currentField.fieldType = FVAL_PROCESSING_MSGTYPE_HI;
                } else {
                    currentField.fieldType = FVAL_ERROR_PROTO;
                    contProcessing = false;
                }
            }
			break;

        case FVAL_PROCESSING_MSGTYPE_HI:
            if(readAvailable()) {
                currentField.msgType = readByte() << 8;
                currentField.fieldType = FVAL_PROCESSING_MSGTYPE_LO;
            } 
            break;

        case FVAL_PROCESSING_MSGTYPE_LO:
            if(readAvailable()) {
                currentField.msgType |= readByte() & 0xff;
                currentField.fieldType = FVAL_NEW_MSG;
            }
            return &currentField;

        case FVAL_NEW_MSG:
		case FVAL_FIELD: // the field finished last time around, now reset it.
			currentField.fieldType = FVAL_PROCESSING;
			currentField.field = UNKNOWN_FIELD_PART;
			break;

		case FVAL_PROCESSING: // we are looking for the field key
			contProcessing = processMsgKey();
			break;

		case FVAL_PROCESSING_WAITEQ: // we expect an = following the key
			if(!readAvailable()) break;
			if(readByte() != '=') {
				clearFieldStatus(FVAL_ERROR_PROTO);
				return &currentField;
			}
			currentField.len = 0;
			currentField.fieldType = FVAL_PROCESSING_VALUE;
			break;

		case FVAL_PROCESSING_VALUE: // and lastly a value followed by pipe.
			if(!processValuePart()) {
				clearFieldStatus(FVAL_ERROR_PROTO);
                contProcessing = false;
			}
			if(currentField.fieldType != FVAL_PROCESSING_VALUE) return &currentField;
			break;
		}
		contProcessing = contProcessing && readAvailable();
	}
	return &currentField;
}
