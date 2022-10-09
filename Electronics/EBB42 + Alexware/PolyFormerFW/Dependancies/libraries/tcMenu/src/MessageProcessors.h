/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * MessageProcessors.h - standard message processors that decode tcMenu messages.
 */

#ifndef _TCMENU_MESSAGEPROCESSORS_H_
#define _TCMENU_MESSAGEPROCESSORS_H_

#include <PlatformDetermination.h>
#include <SimpleCollections.h>
#include "tcMenu.h"

/**
 * @file MessageProcessors.h
 * 
 * This file contains the default processors that can deal with incoming messages turning
 * them into events on tcMenu.
 */

class TagValueRemoteConnector; // forward reference
struct FieldAndValue; // forward reference

/**
 * Message processors need to store some state while they are working through the fields
 * of a message, this union keeps state between a message starting processing and ending
 * It can be added to with additional unions. It is essentially stored globally so size
 * is an issue. If you need to extend the messages that can be processed, you'll probably
 * also need to store some state. This is the ideal place to store such state in the union.
 * 
 * This union structure exists once per remote connection, and given that only one message
 * on a remote connection can be processed at once, it can be a union. It is cleared at
 * the start of each process.
 */
union MessageProcessorInfo {
	struct {
		MenuItem* item;
		int changeValue;
        uint32_t correlation;
		ChangeType changeType;
	} value;
	struct {
		uint8_t major, minor;
		ApiPlatform platform;
        bool authProvided;
	} join;
    struct {
        char name[16];
    } pairing;
    struct {
        char mode;
        uint8_t button;
        uint32_t correlation;
    } dialog;
    struct {
        HeartbeatMode hbMode;
    } hb;
    struct {
        uint8_t data[16];
    } custom;
};

typedef void (*FieldUpdateFunction)(TagValueRemoteConnector*, FieldAndValue*, MessageProcessorInfo*);

/**
 * Each incoming message needs to have a MsgHandler associated with it. It maps the message type
 * to a function that can process the message fields as they arrive. It will be called every time
 * there is a new field on a message, it should at a minimum be able to process the field updates 
 * end check for the end of the message.
 * @see FieldAndValue
 */
class MsgHandler {
private:
    /** A function that will process the message, a field at a time */
    FieldUpdateFunction fieldUpdateFn;

    /** the type of message the above function can process. */
    uint16_t msgType;

public:
    // as this class is stored in a btree list, it needs copy constructors and = operators implemented
    MsgHandler() : fieldUpdateFn(nullptr), msgType(0xffff) {}
    MsgHandler(uint16_t msgType, FieldUpdateFunction fn) : fieldUpdateFn(fn), msgType(msgType) {}
    MsgHandler(const MsgHandler& other) = default;
    MsgHandler& operator=(const MsgHandler& other) = default;
    uint16_t getKey() const { return msgType; }
    void invoke(TagValueRemoteConnector* rc, FieldAndValue* fv, MessageProcessorInfo* info) {
        if(fieldUpdateFn) fieldUpdateFn(rc, fv, info);
    }
};

/**
 * If you decide to write your own processor, this method can handle join messages
 */
void fieldUpdateJoinMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * If you decide to write your own processor, this method can handle value messages.
 */
void fieldUpdateValueMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * If you decide to write your own processor, this method can handle pairing messages.
 */
void fieldUpdatePairingMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * If you decide to write your own processor, this method can handle dialog updates
 */
void fieldUpdateDialogMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * If you decide to write your own processor, this method can handle heartbeat updates
 */
void fieldUpdateHeartbeatMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * This message processor is responsible for handling messages coming off the wire and processing them into
 * usable events by the rest of the system. Usually, the message processor actually handles the event in 
 * full.
 * 
 * When a new message arrives, this class attempts to find a suitable processor function (or ignore if we
 * can't process), then each field in the message is passed to the function to processed.
 *
 * You can customize a message process by adding your own additional message types that it can process using the
 * addCustomMsgHandler(..) function.
 */
class CombinedMessageProcessor {
private:
	MessageProcessorInfo val;
    BtreeList<uint16_t, MsgHandler> messageHandlers;
	MsgHandler* currHandler;
public:
	/**
	 * Constructor takes an array of processors and the number of processors in the array.
	 */
	CombinedMessageProcessor();
	/**
	 * Whenever there is a new message, this will be called, to re-initialise the internal state
	 */
	void newMsg(uint16_t msgType);
	/**
	 * Called whenever a field has been processed in the current message, after a call to newMsg
	 */
	void fieldUpdate(TagValueRemoteConnector* connector, FieldAndValue* field);
    /**
     * If you want to be able to process a custom incoming message, simply add it here as a MsgHandler, see above.
     * It can be a local, even inline object as it will be copy constructed to an internal list.
     * Step 1. You define a custom message type using something similar to
     *
     *      `#define MSG_CUSTOM msgFieldToWord('Z','Z')`
     *
     * Step 2. You create a function that is called back for each field as it arrives
     *
     * ```
     * void myFieldProcessor(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
     *      // The info is a pointer to 16 bytes of memory that you can use for any purpose.
     *      if(field->fieldType == FVAL_END_MSG) {
     *          // message is ending. All fields are processed
     *      } else {
     *         // process fields as they arrive.
     *         if(field->field == FIELD_HB_MODE) {
     *             // do something, or maybe store in the info area temporarily.
     *         }
     *      }
     * }
     * ```
     *
     * Step 3. Register the processor
     *
     * ```
     * myProcessor.addCustomMsgHandler(MSG_CUSTOM, myFieldProcessor);
     * ```
     *
     * @see RemoteConnector for how to create a custom message rather than receive it.
     * @see FieldAndValue for more information about how the field callback works.
     */
     void addCustomMsgHandler(uint16_t msgType, FieldUpdateFunction callback) {
         messageHandlers.add(MsgHandler(msgType, callback));
     }
};

#endif /* _TCMENU_MESSAGEPROCESSORS_H_ */
