/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */
/**
 * @file RemoteTypes.h
 * 
 * contains the definitions of each message and field.
 */

#ifndef _TCMENU_REMOTETYPES_H_
#define _TCMENU_REMOTETYPES_H_

/**
 * This defines the maximum size of any field value that can be received by this library.
 * If you need longer fields, change this value to a higher one.
 */
#define MAX_VALUE_LEN 40

enum AckResponseStatus {
    // warnings
    ACK_VALUE_RANGE = -1 , 
    // success
    ACK_SUCCESS = 0, 
    //errors
    ACK_ID_NOT_FOUND = 1, ACK_CREDENTIALS_INVALID = 2,
    
    // unknown error, always last
    ACK_UNKNOWN = 10000 
    // unknown error, always last
};

/**
 * Converts a message field as two separate entities into a single word.
 */
#define msgFieldToWord(a,b)  ( (((uint16_t)(a))<<8U) | ((uint16_t)(b)) )

/*
 * Definitions for an unknown field key or part thereof.
 */
#define UNKNOWN_FIELD_PART 0x00

/**
 * Definition for an unknown message key
 */
#define UNKNOWN_MSG_TYPE 0x0000

/** Message type definition for paring message */
#define MSG_PAIR msgFieldToWord('P','R')
/** Message type definition for acknowledgement message */
#define MSG_ACKNOWLEDGEMENT msgFieldToWord('A','K')
/** Message type definition for Join message */
#define MSG_JOIN msgFieldToWord('N','J')
/** Message type definition for heartbeat message */
#define MSG_HEARTBEAT msgFieldToWord('H','B')
/** Message type definition for Bootstrap message */
#define MSG_BOOTSTRAP msgFieldToWord('B','S')
/** Message type definition for Analog item bootstrap message */
#define MSG_BOOT_ANALOG msgFieldToWord('B','A')
/** Message type definition for ACtion item bootstap message */
#define MSG_BOOT_ACTION msgFieldToWord('B', 'C')
/** Message type definition for sub menu bootstrap message */
#define MSG_BOOT_SUBMENU msgFieldToWord('B', 'M')
/** Message type definition for enum bootstrap message */
#define MSG_BOOT_ENUM msgFieldToWord('B', 'E')
/** Message type definition for boolean bootstrap message */
#define MSG_BOOT_BOOL msgFieldToWord('B', 'B')
/** Message type definition for text boostrap message */
#define MSG_BOOT_TEXT msgFieldToWord('B','T')
/** Message type definition for large number boot message */
#define MSG_BOOT_LARGENUM msgFieldToWord('B', 'N')
/** Message type definition for floating point bootstrap message */
#define MSG_BOOT_FLOAT msgFieldToWord('B','F')
/** Message type definition for RGB color bootstrap message */
#define MSG_BOOT_RGB_COLOR msgFieldToWord('B', 'K')
/** Message type definition for scroll bootstrap message */
#define MSG_BOOT_SCROLL_CHOICE msgFieldToWord('B', 'Z')
/** Message type definition for runtime list bootstrap message */
#define MSG_BOOT_LIST msgFieldToWord('B','L')
/** Message type definition for value change message */
#define MSG_CHANGE_INT msgFieldToWord('V', 'C')
/** Message type defintion for a dialog change msg */
#define MSG_DIALOG msgFieldToWord('D', 'M')

#define FIELD_MSG_NAME    msgFieldToWord('N', 'M')
#define FIELD_VERSION     msgFieldToWord('V', 'E')
#define FIELD_PLATFORM    msgFieldToWord('P', 'F')
#define FIELD_BOOT_TYPE   msgFieldToWord('B', 'T')
#define FIELD_HB_INTERVAL msgFieldToWord('H', 'I')
#define FIELD_HB_MILLISEC msgFieldToWord('H', 'M')
#define FIELD_HB_MODE     msgFieldToWord('H', 'R')
#define FIELD_ID          msgFieldToWord('I', 'D')
#define FIELD_EEPROM      msgFieldToWord('I', 'E')
#define FIELD_READONLY    msgFieldToWord('R', 'O')
#define FIELD_VISIBLE     msgFieldToWord('V', 'I')
#define FIELD_PARENT      msgFieldToWord('P', 'I')
#define FIELD_ANALOG_MAX  msgFieldToWord('A', 'M')
#define FIELD_ANALOG_OFF  msgFieldToWord('A', 'O')
#define FIELD_ANALOG_DIV  msgFieldToWord('A', 'D')
#define FIELD_ANALOG_STEP msgFieldToWord('A', 'S')
#define FIELD_ANALOG_UNIT msgFieldToWord('A', 'U')
#define FIELD_CURRENT_VAL msgFieldToWord('V', 'C')
#define FIELD_BOOL_NAMING msgFieldToWord('B', 'N')
#define FIELD_NO_CHOICES  msgFieldToWord('N', 'C')
#define FIELD_CHANGE_TYPE msgFieldToWord('T', 'C')
#define FIELD_MAX_LEN     msgFieldToWord('M', 'L')
#define FIELD_REMOTE_NO   msgFieldToWord('R', 'N')
#define FIELD_FLOAT_DP    msgFieldToWord('F', 'D')
#define FIELD_UUID        msgFieldToWord('U', 'U')
#define FIELD_CORRELATION msgFieldToWord('I', 'C')
#define FIELD_ACK_STATUS  msgFieldToWord('S', 'T')
#define FIELD_HEADER      msgFieldToWord('H', 'F')
#define FIELD_BUTTON1     msgFieldToWord('B', '1')
#define FIELD_BUTTON2     msgFieldToWord('B', '2')
#define FIELD_BUFFER      msgFieldToWord('B', 'U')
#define FIELD_MODE        msgFieldToWord('M', 'O')
#define FIELD_EDIT_MODE   msgFieldToWord('E', 'M')
#define FIELD_ALPHA       msgFieldToWord('R', 'A')
#define FIELD_WIDTH       msgFieldToWord('W', 'I')

#define FIELD_PREPEND_CHOICE 'C'
#define FIELD_PREPEND_NAMECHOICE 'c'

/**
 * Defines the types of change that can be received / sent in changes messages, either
 * delta or incremental (for example menuVolume + 3) or absolulte (channel is now 2)
 */
enum ChangeType: uint8_t {
	CHANGE_DELTA = 0, CHANGE_ABSOLUTE = 1, CHANGE_LIST = 2, CHANGE_LIST_RESPONSE = 3
};

/**
 * Defines the API platforms that are supported at the moment
 */
enum ApiPlatform : uint8_t {
	PLATFORM_ARDUINO_8BIT = 0,
	PLATFORM_JAVA_API = 1,
    PLATFORM_ARDUINO_32BIT = 2,
	PLATFORM_DOTNET = 3
};

/**
 * Defines the type of heartbeat we are dealing with
 */
enum HeartbeatMode : uint8_t {
	/** During normal operation we send this to continue the connection during idle times */
	HBMODE_NORMAL = 0,
	/** At connection start, we wait for this before proceeding */
	HBMODE_STARTCONNECT = 1,
	/** At the end of a connection we send this to stop the connection */
	HBMODE_ENDCONNECT = 2
};

#endif /* _TCMENU_REMOTETYPES_H_ */
