/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file MenuItems.h
 * 
 * In TcMenu, MenuItem storage is shared between program memory and RAM. Usually each MenuItem has associated Info block and
 * within the InfoBlock, the first fields must be in the same order as AnyMenuInfo. The most commonly used menu items are
 * defined within this file. Each menu item also has a menu item type that is used during rendering and remote communication
 * to determine what it actually is.
 * 
 * Most of the editable menu items can stored to EEPROM, including AnalogMenuItem, EnumMenuItem, BooleanMenuItem and TextMenuItem
 */

#ifndef _MENUITEMS_h
#define _MENUITEMS_h

#include "tcUtil.h"

#define INFO_LOCATION_PGM true
#define INFO_LOCATION_RAM false

/** The maxmimum ID that is allowed manually, only automated ID's for back items and similar will exceed this */
#define MAXIMUM_ID_ALLOWED 32000

typedef uint16_t menuid_t;

/** Defines a menu ID that is not valid and may be used to indicate no menu item */
#define INVALID_MENU_ID 0xffff

/** the size of each name in program memory */
#define NAME_SIZE_T 20

/** the value that represents no call back */
#define NO_CALLBACK NULL

/** The definition of a callback from a menu item */
typedef void (*MenuCallbackFn)(int id);

/**
 * Every single info structure must have these fields in this order. They are always stored in program memory.
 * It is always safe to use this structure in place of a specific one. This is indeed how the core MenuItem class works.
 */
struct AnyMenuInfo {
	/** the name given to this menu item */ 
	char name[NAME_SIZE_T];
	/** the identifier for this menu */
	menuid_t id;
	/** eeprom address for this item or -1 if not stored */
	uint16_t eepromAddr;
	/** maximum value that this type can store */
	uint16_t maxValue;
	/** the callback function */
	MenuCallbackFn callback;
};

/**
 * The information block stored in program memory for analog items. Analog items represent a 16 bit
 * unsigned editable value. It can be made negative by applying an offset, and accuracy in decimal 
 * places set by the divisor. 
 * 
 * Note: These items must remain in this order, as MenuItem relies upon it.
 */
struct AnalogMenuInfo {
	/** the name given to this menu item */ 
	char name[NAME_SIZE_T];
	/** the identifier for this menu */
	menuid_t id;
	/** eeprom address for this item or -1 if not stored */
	uint16_t eepromAddr;
	/** maximum value that this type can store */
	uint16_t maxValue;
	/** the callback function */
	MenuCallbackFn callback;

	/**
	 * A 16 bit offset that can be either positive or negative, this will be added to the current value 
	 * before display
	 */
	int16_t offset;
	/**
	 * A divisor that is used to reduce the currentValue by means of division:
	 * 0 or 1 : no division
	 * 2 thru 10 : single decimal place using optimised rendering
	 * above 10 : turned to decimal and displayed to correct number of DP.
	 */
	uint16_t divisor;
	/**
	 * An optional unit name to be presented after the the value. For example V for volts, dB for decibel.
	 */
	char unitName[5];
};

/**
 * The information block stored in program memory for enumeration items. Enumeration items are somewhat
 * like a comboBox or choice, they have a predetermined set of options, the current value represents the
 * index of the chosen one.
 * 
 * Note: These items must remain in this order, as MenuItem relies upon it.
 */
struct EnumMenuInfo {
	/** the name given to this menu item */ 
	char name[NAME_SIZE_T];
	/** the identifier for this menu */
	menuid_t id;
	/** eeprom address for this item or -1 if not stored */
	uint16_t eepromAddr;
	/** the number of items in the below array. */
	uint16_t maxValue;
	/** the callback function */
	MenuCallbackFn callback;

	/**
	 * An array of pointers to char arrys. Each one represents a possible choice, for example:
	 * 
	 *     const char enumText1[] = "Option 1";
     *     const char* const minfoTextStrs[] = { enumText1 };
     * 
     * Note that on AVR arch. they must be in progam memory
	 */
	const char * const *menuItems;
};

/**
 * This indicates the preferred terminology to use to describe a boolean value. Such as On/Yes/True etc.
 */
enum BooleanNaming : uint8_t {
    /** prefer the use of true / false */
	NAMING_TRUE_FALSE = 0,
	/** prefer the use of On / Off */
	NAMING_ON_OFF,
	/** prefer the user of Yes and No */
	NAMING_YES_NO
};

/**
* The information block stored in program memory for boolean items. Boolean items hold either true or
* false only. The text used to represent those value can be changed by chaning the BooleanNaming parameter.
* 
* Note: These items must remain in this order, as MenuItem relies upon it.
*/
struct BooleanMenuInfo {
	/** the name given to this menu item */ 
	char name[NAME_SIZE_T];
	/** the identifier for this menu */
	menuid_t id;
	/** eeprom address for this item or -1 if not stored */
	uint16_t eepromAddr;
	/** maximum value that this type can store - always 1 */
	uint16_t maxValue;
	/** the callback function */
	MenuCallbackFn callback;

	/**
	 * Defines the text that will be used to represent the boolean in this item
	 * @see BooleanNaming
	 */
	BooleanNaming naming;
};

/**
 * The information block for a submenu stored in program memory. Sub menus can contain other menu items
 * as children and render as another menu below this menu.
 * 
 * Note: These items must remain in this order, as MenuItem relies upon it.
 */
struct SubMenuInfo {
	/** the name given to this menu item */ 
	char name[NAME_SIZE_T];
	/** the identifier for this menu */
	menuid_t id;
	/** Not used for submenus. */
	uint16_t eepromAddr;
	/** maximum value that this type can store - always 0 */
	uint16_t maxValue;
	/** the callback function - not used for submenu's */
	MenuCallbackFn callback;
};

/**
 * The information block for a floating point menu component. Floating point items are not editable, they are
 * generally to relay status information.
 */
struct FloatMenuInfo {
	/** the name given to this menu item */ 
	char name[NAME_SIZE_T];
	/** the identifier for this menu */
	menuid_t id;
	/** eeprom address for this item or -1 if not stored */
	uint16_t eepromAddr;
	/** The number of decimal places to render to */
	uint16_t numDecimalPlaces;
	/** The callback function */
	MenuCallbackFn callback;
};

/** 
 * Each menu item can be in the following states.
 */
enum Flags : uint8_t {
    /** the menu is currently active but not editing */
	MENUITEM_ACTIVE = 0,
    /** the menu has changed and needs drawing by the renderer */
	MENUITEM_CHANGED = 1,
    /** the menu cannot be changed by the renderer or remote, it can be changed by calling the setter. */
	MENUITEM_READONLY = 2,
    /** the menu must not be sent remotely, and is only available via the local renderer */
	MENUITEM_LOCAL_ONLY = 3,
    /** the menu is currently being edited */
	MENUITEM_EDITING = 4,
    /** the menu item is secured, and must be accessed with a pin */
    MENUITEM_PIN_SECURED = 5,
    /** the menu item is visible on the display and remote */
    MENUITEM_PIN_VISIBLE = 6,
    /** the info structure is in PROGMEM */
    MENUITEM_INFO_STRUCT_PGM = 7,
    /** indicates that remote 0 needs to resend this item */
	MENUITEM_REMOTE_SEND0 = 10,
    /** indicates that remote 1 needs to resend this item */
	MENUITEM_REMOTE_SEND1 = 11,
    /** indicates that remote 2 needs to resend this item */
	MENUITEM_REMOTE_SEND2 = 12,
    /** indicates that remote 3 needs to resend this item */
	MENUITEM_REMOTE_SEND3 = 13,
    /** indicates that remote 4 needs to resend this item */
	MENUITEM_REMOTE_SEND4 = 14,
    /** indicates that remote 5 needs to resend this item */
	MENUITEM_REMOTE_SEND5 = 15
};

#define MENUITEM_ALL_REMOTES 0xFC00

/**
 * As we don't have RTTI we need a way of identifying each menu item. Any value below 100 is based
 * on ValueMenuItem and can therefore be edited, otherwise it cannot be edited on the device.
 * MenuItems less than 100 are based on ValueMenuItem and can be edited as an integer.
 * MenuItems greater than 200 are runtime menu items.
 * Items between 100 and 200 are non editable regular menu items.
 */
enum MenuType : uint8_t {
	/** item is of type AnalogMenuItem */
	MENUTYPE_INT_VALUE = 1, 
	/** item is of type EnumMenuItem */
	MENUTYPE_ENUM_VALUE = 2,
	/** item is of type BooleanMenuItem */
	MENUTYPE_BOOLEAN_VALUE = 3,
	/** item is of type FloatMenuItem */
	MENUTYPE_FLOAT_VALUE = 101,
	/** item is of type ActionMenuItem */
	MENUTYPE_ACTION_VALUE = 102,
	/** item is a single item of type RuntimeMenuItem */
	MENUTYPE_RUNTIME_VALUE = 150,
	/** item is a list, of type MultiRuntimeMenuItem */
	MENUTYPE_RUNTIME_LIST = 151,
	/** item is of type BackMenuItem */
	MENUTYPE_BACK_VALUE = 152,
	/** item is of type ActivateSubMenuItem */
	MENUTYPE_ACTIVATE_SUBMENU = 153,
	/** item is of type ScrollChoiceMenuItem */
	MENUTYPE_SCROLLER_VALUE = 154,
    /** item is of type SubMenuItem */
    MENUTYPE_SUB_VALUE = 155,
    /** represents the title item, never sent remotely, only for display */
    MENUTYPE_TITLE_ITEM = 156,
    /** A button that can only be part of a dialog */
    MENUTYPE_DIALOG_BUTTON = 157,

    /** item is of type TextMenuItem */
	MENUTYPE_TEXT_VALUE = 200,
	/** item is an IP address and is editable per segment */
	MENUTYPE_IPADDRESS = 201,
    /** An item that represents a time */
    MENUTYPE_TIME = 202,
    /** An item that represents a date */
    MENUTYPE_DATE = 203,
    /** An item that represent a large editable number */
    MENUTYPE_LARGENUM_VALUE = 210,
    /** Represents an RGBA color picker, where alpha is optional */
    MENUTYPE_COLOR_VALUE = 211
};

/**
 * this is used in the redering function to indicate what needs to be done in this call.
 */
enum RenderFnMode : uint8_t {
	/** render the current value for the item provided */
	RENDERFN_VALUE,
	/** render the name part into the buffer */
	RENDERFN_NAME,
	/** Get the eeprom position in the returned int, buffer not needed */
	RENDERFN_EEPROM_POS,
	/** the callback has been triggered, buffer not needed */
	RENDERFN_INVOKE,
	/** A new value for a position in the list, provided in buffer, it's length is in size. - used only in editable mode */
	RENDERFN_SET_VALUE,
	/** Gets the range zero based, for this part - used only in editable mode, buffer not needed */
	RENDERFN_GETRANGE,
	/** Gets the integer value of the current part that is being edited, buffer not needed */
	RENDERFN_GETPART
};

// forward reference
class RuntimeMenuItem;

/**
 * When implementing runtime menu items, they need to have a render function that is called for
 * to obtain the name and value for each row, and should populate the buffer with appropriate
 * text.
 *
 * @param item the menu item that is being drawn
 * @param row the row in the list of items that is being drawn
 * @param mode one of the RenderFnMode enumerations indicating what you need to do.
 * @param buffer the buffer to copy into
 * @param bufferSize the size of the buffer available.
 */
typedef int(*RuntimeRenderingFn)(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/**
 * This is the base class of all menu items, containing functionality to hold the current state of the menu
 * and determine which is the next menu in the chain. It also defines a few functions that all implementations
 * implement.
 * 
 * As there is limited memory on the device, most of the static information is stored in a paired AnyMenuInfo
 * structure, saving quite a lot of RAM, also each menu item is in a chain, where getNext() will returned the
 * next available item. NULL represents the end of the chain.
 */
class MenuItem {
protected:
	uint16_t flags;
	MenuItem* next;
	/** we only have either an info structure or a runtime menu callback function */
	union {
		const AnyMenuInfo *info;
		RuntimeRenderingFn renderFn;
	};
	MenuType menuType;
public:

    /**
     * Copies the name into the provided buffer starting at position 0.
     * @param sz the buffer space
     * @param size the size of sz, generally obtained using sizeof
     */
	uint8_t copyNameToBuffer(char* sz, int size) const { return copyNameToBuffer(sz, 0, size);}

	/** 
     * Copies the name info the provided buffer starting at the specified 
     * position.
     * @param sz the buffer space
     * @param offset the offset to start at relative to the buffer
     * @param size the size of sz, generally obtained using sizeof
     * */
	uint8_t copyNameToBuffer(char* sz, int offset, int size) const;
	/** Retrieves the ID from the info block */
	menuid_t getId() const;
	/** Retrieves the maximum value for this menu type */
	uint16_t getMaximumValue() const;
	/** Retrieves the eeprom storage position for this menu (or 0xffff if not applicable) */
	uint16_t getEepromPosition() const;
	/** returns the menu type as one of the above menu type enumeration */
	MenuType getMenuType() const { return menuType; }
	/** triggers the event callback associated with this item */
	void triggerCallback() const;

    bool isInfoProgMem() const { return bitRead(flags, MENUITEM_INFO_STRUCT_PGM); }

	/** set the item to be changed, this lets the renderer know it needs painting */
	void setChanged(bool changed) { bitWrite(flags, MENUITEM_CHANGED, changed); }
	/** returns the changed state of the item */
	bool isChanged() const { return bitRead(flags, MENUITEM_CHANGED); }
	/** returns if the menu item needs to be sent remotely */
	bool isSendRemoteNeeded(uint8_t remoteNo) const;
	/** Set all the flags indicating that a remote refresh is needed for all remotes */
	void setSendRemoteNeededAll();
    /** Clears all the flags indicating that a remote send is needed for all remotes. */
    void clearSendRemoteNeededAll();
	/** set the flag indicating that a remote refresh is needed for a specific remote */
	void setSendRemoteNeeded(uint8_t remoteNo, bool needed);

	/** sets this to be the active item, so that the renderer shows it highlighted */
	void setActive(bool active);
	/** returns the active status of the item */
	bool isActive() const { return bitRead(flags, MENUITEM_ACTIVE); }

	/** sets this item as the currently being edited, so that the renderer shows it as being edited */
	void setEditing(bool active);
	/** returns true if the status is currently being edited */
	bool isEditing() const { return bitRead(flags, MENUITEM_EDITING); }

	/** sets this item to be read only, so that the manager will not allow it to be edited */
	void setReadOnly(bool active) { bitWrite(flags, MENUITEM_READONLY, active); }
	/** returns true if this item is read only */
	bool isReadOnly() const { return bitRead(flags, MENUITEM_READONLY); }

	/** sets this item to be available only locally */
	void setLocalOnly(bool localOnly) { bitWrite(flags, MENUITEM_LOCAL_ONLY, localOnly); }
	/** returns true if this item is only available locally */
	bool isLocalOnly() const { return bitRead(flags, MENUITEM_LOCAL_ONLY); }

	/** sets this item to need pin security in order to display, currently only available locally */
	void setSecured(bool secured) { bitWrite(flags, MENUITEM_PIN_SECURED, secured); }
	/** returns true if this item requires a pin to display, , currently only available locally */
	bool isSecured() const { return bitRead(flags, MENUITEM_PIN_SECURED); }

	/** sets this item to need pin security in order to display, currently only available locally */
	void setVisible(bool visible) { bitWrite(flags, MENUITEM_PIN_VISIBLE, visible); }
	/** returns true if this item requires a pin to display, , currently only available locally */
	bool isVisible() const { return bitRead(flags, MENUITEM_PIN_VISIBLE); }

    /** gets the next menu (sibling) at this level */
	MenuItem* getNext() const { return next; }
	/** sets or changes the menu item that comes after this one, nullptr means this is the last item */
	void setNext(MenuItem* next) { this->next = next; }

    /**
     * Marks the menu item as having updated locally and remotely and also calls the callback if not silent
     */
    void changeOccurred(bool silent);
protected:
	/**
	 * Do not directly create menu items, always use the leaf classes, such as AnalogMenuItem etc.
	 */
	MenuItem(MenuType menuType, const AnyMenuInfo* menuInfo, MenuItem* next, bool infoProgMem);

};

/** 
 * Represents an item that has a 16 bit unsigned integer backing it, never directly used, this class is
 * essentially abstract.
 */
class ValueMenuItem : public MenuItem {
protected:
	uint16_t currentValue;

	/** Use the leaf types, dont construct directly. Initialise an instance of this type with the required data values */
	ValueMenuItem(MenuType menuType, const AnyMenuInfo* info, uint16_t defaultVal, MenuItem* next, bool infoPgm)
	        : MenuItem(menuType, info, next, infoPgm) {
		this->currentValue = defaultVal;
	}
public:
	/** Sets the integer current value to a new value, and marks the menu changed */
	void setCurrentValue(uint16_t val, bool silent = false);

	/** gets the current value */
	uint16_t getCurrentValue() const { return currentValue; }
};

/**
 * This makes working with the analog values in an AnalogMenuItem easier by splitting the raw value
 * into a whole part and a decimal fraction part based on the menu items divisor.
 */
struct WholeAndFraction {
public:
	WholeAndFraction() {
		this->whole = this->fraction = this->negative = 0;
	}

	WholeAndFraction(const WholeAndFraction& that) {
		this->whole = that.whole;
		this->fraction = that.fraction;
		this->negative = that.negative;
	}

	WholeAndFraction(uint16_t whole, uint16_t fract, bool negative) {
		this->whole = whole;
		this->fraction = fract;
		this->negative = negative;
	}
	uint32_t whole: 15;
	uint32_t fraction: 16;
    uint32_t negative: 1;
};

/**
 * An item that can represent a numeric value, integer, or decimal. On an 8bit Arduino this value is a
 * 16 bit unsigned integer value. We can make it appear negative by giving a negative offset. We make
 * it appear decimal by giving it a divisor. If the divisor were 2, we'd increment in halves. If the
 * offset point were -100, unit dB and divisor 2, the first value would be -50.0dB and the next would be
 * -49.5dB and so on. For convenience there are methods to convert between both floating point values and
 * also fixed point (WholeAndFraction) values.
 * @see AnalogMenuInfo
 */
class AnalogMenuItem : public ValueMenuItem {
private:
    int8_t step = 1;
public:
	/**
	 * Create an instance of the class
	 * 
	 * @param info an AnalogMenuInfo structure
	 * @param defaultVal the default starting value
	 * @param next the next menu in the chain if there is one, or NULL.
	 */
	AnalogMenuItem(const AnalogMenuInfo* info, uint16_t defaultVal, MenuItem* next = nullptr, bool infoInPgm = INFO_LOCATION_PGM)
	        : ValueMenuItem(MENUTYPE_INT_VALUE, (const AnyMenuInfo*)info, defaultVal, next, infoInPgm) {;}

	/** Returns the offset from the MenuInfo structure */
	int getOffset() const;

    /** Returns the step from the menu info structure */
    int getStep() const { return step; }

    /** Change the step for incremental updates, must be an exact multiple of max value */
    void setStep(int newStep) { step = newStep; }

	/** Returns the divisor from the menu info structure */
	uint16_t getDivisor() const;

	/** Returns the length of the unit name */
	int unitNameLength() const;

	/** copies the unit name into the provided buffer */
	void copyUnitToBuffer(char* unitBuff, uint8_t size = 5) const;

	/**
	 * copies the whole value including unit into the buffer provided.
	 * @param buffer the buffer to write the value into
	 * @param bufferSize the size of the buffer
	 */
	void copyValue(char* buffer, uint8_t bufferSize) const;


	/** 
	 * returns the closest floating point representation of the value, note that floating point values are
	 * not always able to exactly represent a given value and may therefore be slightly out.
	 * @return the nearest floating point value
	 */
	float getAsFloatingPointValue() const;

	/**
	 * Sets the menu item's current value to be the value provided in the float.
	 * @param value the new value.
	 */
	void setFromFloatingPointValue(float value);

    int getIntValueIncludingOffset() { return (int)currentValue + (int)getOffset(); }

	/**
	 * gets the whole and fraction part with the fractional part converted to decimal for ease of use. It
	 * based upon the divisor.
	 * @return a structure containing the whole and fraction in decimal form
	 */
	WholeAndFraction getWholeAndFraction() const;

	/**
	 * sets the menu based on the decimal whole and decimal fraction part. If decimal is true, the fraction
	 * is expected to be in decimal form (eg for halves it would be 0 or 5).
	 * @param wf the whole fraction part.
	 */
	void setFromWholeAndFraction(WholeAndFraction wf);

	/**
	 * @return the number of decimal places needed for the fraction part based on the divisor
	 */
	uint8_t getDecimalPlacesForDivisor() const;

	/**
	 * @return the nearest decimal divisor based on the divisor.
	 */
	uint16_t getActualDecimalDivisor() const;
};

/**
 * An item that can represent a known series of values, somewhat like a combo box. We provide a list
 * of choices and only one of those choices can be active at once. The choice is a zero based integer
 * with the first choice being 0 and so on.
 * @see EnumMenuInfo
 */
class EnumMenuItem : public ValueMenuItem {
public:
	/**
	 * Create an instance of the class
	 * 
	 * @param info an EnumMenuInfo structure
	 * @param defaultVal the default starting value
	 * @param next the next menu in the chain if there is one, or NULL.
	 */
	EnumMenuItem(const EnumMenuInfo *info, uint8_t defaultVal, MenuItem* next = nullptr, bool infoInPgm = INFO_LOCATION_PGM)
	        : ValueMenuItem(MENUTYPE_ENUM_VALUE, (const AnyMenuInfo*)info, defaultVal, next, infoInPgm) {;}

	/**
	 * Copies one of the enum strings into a buffer
	 * @param buffer the buffer to copy into
	 * @param idx the index of choice to copy
	 */ 
	void copyEnumStrToBuffer(char* buffer, int size, int idx) const;

	/**
	 * Returns the length of an enumeration string with given index
	 * @param idx the index to get the length for
	 */
	int getLengthOfEnumStr(int idx) const;
};

/**
 * An item that can represent only two states, true or false. Can be configured to show as ON/OFF, TRUE/FALSE or
 * YES/NO as required.
 * @see BooleanMenuInfo
 */
class BooleanMenuItem : public ValueMenuItem {
public:
	/**
	 * Create an instance of the class
	 * 
	 * @param info a BooleanMenuInfo structure
	 * @param defaultVal the default starting value
	 * @param next the next menu in the chain if there is one, or NULL.
	 */
	BooleanMenuItem(const BooleanMenuInfo* info, bool defaultVal, MenuItem* next = nullptr, bool infoInPgm = INFO_LOCATION_PGM)
	        : ValueMenuItem(MENUTYPE_BOOLEAN_VALUE, (const AnyMenuInfo*)info, defaultVal, next, infoInPgm) {;}

	/**
	 * returns the boolean naming for this item, EG: how the value should be rendered 
	 */
	BooleanNaming getBooleanNaming() const;

	/** return the boolean value currently stored */
	bool getBoolean() const {return currentValue != 0;}
	/** set the boolean value currently stored */
	void setBoolean(bool b, bool silent = false) {setCurrentValue(b, silent);}
};

/**
 * FloatMenuItem is for situations where absolute accuracy of the value is not important, for example showing
 * a calculated value from some sensors.
 * @see FloatMenuInfo
 */
class FloatMenuItem : public MenuItem {
private:
	float currValue;
public:
	/**
	 * Create an instance of the class
	 * 
	 * @param info a FloatMenuInfo structure
	 * @param next the next menu in the chain if there is one, or NULL.
	 */
	FloatMenuItem(const FloatMenuInfo* info, MenuItem* next, bool infoInPgm = INFO_LOCATION_PGM)
	        : MenuItem(MENUTYPE_FLOAT_VALUE, (const AnyMenuInfo*)info, next, infoInPgm) { currValue = 0; }

	/**
	 * return the number of decimal places to display for this value
	 */
	int getDecimalPlaces() const;

	/**
	 * Set the floating point value and mark as changed
	 */
	void setFloatValue(float newVal, bool silent = false);

	/**
	 * Get the current floating point value
	 */
	float getFloatValue() const { return currValue; }
};


/**
 * ActionMenuItem is for situations where you want an action to take place when the item is selected.
 * This will call the callback function when the OK button is pressed, or remotely triggered by
 * sending a suitable trigger command through the API.
 */
class ActionMenuItem : public MenuItem {
public:
	/**
	 * Create an instance of the class
	 *
	 * @param info a AnyMenuInfo structure
	 * @param next the next menu in the chain if there is one, or NULL.
	 */
	ActionMenuItem(const AnyMenuInfo* info, MenuItem* next, bool infoInProgmem = INFO_LOCATION_PGM)
	        : MenuItem(MENUTYPE_ACTION_VALUE, info, next, infoInProgmem) {;}
};

// forward reference
class RuntimeMenuItem;

/**
 * Copies the textual representation suitable for display on a device into the provided buffer safely.
 * @param item the item who's value is to be copied
 * @param buffer the buffer for the copy
 * @param bufferSize size of the buffer
 */
void copyMenuItemValue(const MenuItem* item, char* buffer, size_t bufferSize);

/**
 * Copies both the name and textual value representation suitable for display on a device into the buffer safely.
 * @param item the item to be copied
 * @param buffer the buffer to copy into
 * @param bufferSize the size of the buffer
 * @param additionalSep optional provide the separator character (use '\0' for no additional separator)
 */
void copyMenuItemNameAndValue(const MenuItem* item, char* buffer, size_t bufferSize, char additionalSep = ':');

/**
 * Copies the item value into the buffer provided using the above `copyMenuItemValue` method, if the resulting string
 * is empty, then the default is provided instead.
 */
void copyMenuItemValueDefault(const MenuItem* item, char* buffer, size_t bufferSize, const char* defValue);

/**
 * Any MenuType with an ID less than 100 is editable as an integer
 */
inline bool isMenuBasedOnValueItem(const MenuItem* item) {
    if(item == nullptr) return false;
	auto ty =  uint8_t(item->getMenuType());
	return ty == MENUTYPE_INT_VALUE || ty == MENUTYPE_ENUM_VALUE || ty == MENUTYPE_BOOLEAN_VALUE;
}

/**
 * returns true if the menu item is an editable runtime item
 */
inline bool isMenuRuntime(const MenuItem* t) {
	return t && (uint8_t(t->getMenuType()) >= uint8_t(MENUTYPE_RUNTIME_VALUE));
}

/**
 * Returns true if the menu item is a runtime item type. Otherwise returns false.
 */
inline bool isMenuRuntimeMultiEdit(const MenuItem* t) {
	return t && (uint8_t(t->getMenuType()) >= uint8_t(MENUTYPE_TEXT_VALUE));
}

/**
 * gets the menu item as a real time item, only call after determining the menu item is realtime using
 * the above isMenuRuntime() method.
 * @see isMenuRuntime
 */
inline const RuntimeMenuItem* asRuntimeItem(const MenuItem* i) {
	return reinterpret_cast<const RuntimeMenuItem*>(i);
}

#endif
