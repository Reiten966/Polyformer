/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "MenuItems.h"
#include "RuntimeMenuItem.h"

MenuItem::MenuItem(MenuType menuType, const AnyMenuInfo* menuInfo, MenuItem* next, bool infoProgmem) {
	this->flags = 0;
	this->menuType = menuType;
	bitWrite(flags, MENUITEM_INFO_STRUCT_PGM, infoProgmem);
	if(menuInfo != nullptr) this->info = menuInfo;
	this->next = next;
    this->setChanged(true); // items always start out needing redrawing.
    this->setVisible(true); // always start out visible.
}

bool MenuItem::isSendRemoteNeeded(uint8_t remoteNo) const {
    return bitRead(flags, (remoteNo + (int)MENUITEM_REMOTE_SEND0));
}

void MenuItem::setSendRemoteNeeded(uint8_t remoteNo, bool needed) {
	bitWrite(flags, (remoteNo + (int)MENUITEM_REMOTE_SEND0), (needed && !isLocalOnly()));
}

void MenuItem::setEditing(bool active) {
	bool isEditOnEntry = isEditing();
	bitWrite(flags, MENUITEM_EDITING, active);
	setChanged(true);
	if (isMenuRuntimeMultiEdit(this) && !active && isEditOnEntry) {
		auto* item = reinterpret_cast<EditableMultiPartMenuItem*>(this);
		item->stopMultiEdit();
	}
}

void MenuItem::setSendRemoteNeededAll() {
    // make sure local only fields are never marked for sending.
    if(isLocalOnly()) clearSendRemoteNeededAll();

	flags = flags | MENUITEM_ALL_REMOTES;
}

void MenuItem::clearSendRemoteNeededAll() {
	flags = flags & (~MENUITEM_ALL_REMOTES);
}

void MenuItem::triggerCallback() const {
	if (isMenuRuntime(this)) {
		return asRuntimeItem(this)->runCallback();
	}

	auto* cb = (isInfoProgMem()) ? get_info_callback(&info->callback) : info->callback;
	if(cb) cb(getId());
}

uint8_t MenuItem::copyNameToBuffer(char* buf, int offset, int size) const {
	if (isMenuRuntime(this)) {
		asRuntimeItem(this)->copyRuntimeName(buf + offset, size - offset);
		return strlen(buf + offset) + offset;
	}
	else if(isInfoProgMem()) {
		const char* name = info->name;
		uint8_t ret = safeProgCpy(buf + offset, name, size - offset);
		return ret + offset;
	}
	else {
	    strncpy(buf + offset, info->name, size - offset);
	    return strlen(buf + offset) + offset;
	}
}

uint16_t MenuItem::getId() const
{
	if (isMenuRuntime(this)) {
		return asRuntimeItem(this)->getRuntimeId();
	}

	return (isInfoProgMem()) ? get_info_uint(&info->id) : info->id;
}

uint16_t MenuItem::getMaximumValue() const
{
	if (isMenuRuntime(this)) {
		return asRuntimeItem(this)->getNumberOfParts();
	}

	return (isInfoProgMem()) ? get_info_uint(&info->maxValue) : info->maxValue;
}

void MenuItem::changeOccurred(bool silent) {
    setChanged(true);
    setSendRemoteNeededAll();
    if(!silent) triggerCallback();
}

uint16_t MenuItem::getEepromPosition() const
{
	if (isMenuRuntime(this)) return asRuntimeItem(this)->getRuntimeEeprom();
	
	return  isInfoProgMem() ? get_info_uint(&info->eepromAddr) : info->eepromAddr;
}

void MenuItem::setActive(bool active)  {
    bitWrite(flags, MENUITEM_ACTIVE, active);
    setChanged(true);
}

// on avr boards we store all info structures in progmem, so we need this code to
// pull the enum structures out of progmem. Otherwise we just read it out normally

#ifdef __AVR__

void EnumMenuItem::copyEnumStrToBuffer(char* buffer, int size, int idx) const {
    if(!isInfoProgMem()) {
        auto* enumInfo = reinterpret_cast<const EnumMenuInfo*>(info);
        strncpy(buffer, enumInfo->menuItems[idx], size);
    }

    char** itemPtr = ((char**)pgm_read_ptr_near(&((EnumMenuInfo*)info)->menuItems) + idx);
    char* itemLoc = (char *)pgm_read_ptr_near(itemPtr);
    strncpy_P(buffer, itemLoc, size);
	buffer[size - 1] = 0;
}

int EnumMenuItem::getLengthOfEnumStr(int idx) const {
    if(!isInfoProgMem()) {
        auto* enumInfo = reinterpret_cast<const EnumMenuInfo*>(info);
        return strlen(enumInfo->menuItems[idx]);
    }
    char** itemPtr = ((char**)pgm_read_ptr_near(&((EnumMenuInfo*)info)->menuItems) + idx);
    char* itemLoc = (char *)pgm_read_ptr_near(itemPtr);
    return strlen_P(itemLoc);
}

#else 

void EnumMenuItem::copyEnumStrToBuffer(char* buffer, int size, int idx) const {
    EnumMenuInfo* enumInfo = (EnumMenuInfo*)info;
    const char * const* choices = enumInfo->menuItems;
    const char * choice = choices[idx];
    strncpy(buffer, choice, size);
	buffer[size - 1] = 0;
}

int EnumMenuItem::getLengthOfEnumStr(int idx) const {
    EnumMenuInfo* enumInfo = (EnumMenuInfo*)info;
    const char * const* choices = enumInfo->menuItems;
    const char * choice = choices[idx];
    return strlen(choice);
}

#endif

bool isSame(float d1, float d2) {
	float result = tcFltAbs(d1 - d2);
	return result < 0.0000001;
}

void AnalogMenuItem::copyValue(char * buffer, uint8_t bufferSize) const {
	uint8_t dpNeeded = getDecimalPlacesForDivisor();
	WholeAndFraction wf = getWholeAndFraction();

	buffer[0]=0;
	if(wf.negative) appendChar(buffer, '-', bufferSize);
	fastltoa(buffer, wf.whole, 5, NOT_PADDED, bufferSize);

	if (dpNeeded != 0) {
		appendChar(buffer, '.', sizeof bufferSize);
		fastltoa(buffer, wf.fraction, dpNeeded, '0', bufferSize);
	}
	uint8_t numLen = strlen(buffer);
	if(numLen < bufferSize) copyUnitToBuffer(buffer + numLen, bufferSize - numLen);
}

float AnalogMenuItem::getAsFloatingPointValue() const {
	WholeAndFraction wf = getWholeAndFraction();
	serlogF4(SER_TCMENU_DEBUG, "getasF ", wf.whole, wf.fraction, wf.negative);
	float fract = (float(wf.fraction) / float(getActualDecimalDivisor()));
	float whole = (wf.negative) ? -float(wf.whole) : float(wf.whole);
    serlogF3(SER_TCMENU_DEBUG, "whole, fract ", whole, fract);
	return (wf.negative) ? (whole - fract) : (whole + fract);

}

uint16_t AnalogMenuItem::getActualDecimalDivisor() const {
	uint16_t divisor = getDivisor();
	if (divisor < 2) return 1;
	return (divisor > 1000) ? 10000 : (divisor > 100) ? 1000 : (divisor > 10) ? 100 : 10;
}

WholeAndFraction AnalogMenuItem::getWholeAndFraction() const {
	WholeAndFraction wf;
	int32_t calcVal = int32_t(getCurrentValue()) + int32_t(getOffset());
	int32_t divisor = getDivisor();

    wf.negative = (calcVal < 0);

	if (divisor < 2) {
		wf.whole = abs(calcVal);
		wf.fraction = 0;
	}
	else {
		wf.whole = abs(calcVal) / int16_t(divisor);
		int fractMax = getActualDecimalDivisor();
		wf.fraction = abs((calcVal % divisor)) * (fractMax / divisor);
	}
	return wf;
}

void AnalogMenuItem::setFromWholeAndFraction(WholeAndFraction wf) {
	int fractMax = getActualDecimalDivisor();
	uint16_t divisor = getDivisor();
	int correctedFraction = wf.fraction / (fractMax / divisor);
	auto val = (wf.whole * getDivisor());
	if(wf.negative) {
        val = -val;
        val = val - correctedFraction;
	}
	else {
	    val = val + correctedFraction;
	}
	setCurrentValue(val - getOffset());

    serlogF4(SER_TCMENU_DEBUG, "setWF ", wf.whole, wf.fraction, getCurrentValue());
}

void AnalogMenuItem::setFromFloatingPointValue(float value) {
	WholeAndFraction wf;
	if(value < 0.0F) {
	    wf.negative = true;
	    value = tcFltAbs(value);
	}
	wf.whole = static_cast<uint32_t>(value);
	wf.fraction = static_cast<uint32_t>((value - float(wf.whole)) * float(getActualDecimalDivisor()));

	setFromWholeAndFraction(wf);
}

uint8_t AnalogMenuItem::getDecimalPlacesForDivisor() const {
	uint16_t divisor = getDivisor();
	if (divisor < 2) return 0;

	return (divisor > 1000) ? 4 : (divisor > 100) ? 3 : (divisor > 10) ? 2 : 1;
}

int AnalogMenuItem::getOffset() const {
    auto* anInfo = reinterpret_cast<const AnalogMenuInfo*>(info);
    return isInfoProgMem() ? get_info_int(&(anInfo->offset)) : anInfo->offset;
}

uint16_t AnalogMenuItem::getDivisor() const {
    auto* anInfo = reinterpret_cast<const AnalogMenuInfo*>(info);
    return isInfoProgMem() ? get_info_uint(&(anInfo->divisor)) : anInfo->divisor;
}

int AnalogMenuItem::unitNameLength() const {
    auto* anInfo = reinterpret_cast<const AnalogMenuInfo*>(info);
    return isInfoProgMem() ? ((int) strlen_P(anInfo->unitName)) : strlen(anInfo->unitName);
}

void AnalogMenuItem::copyUnitToBuffer(char *unitBuff, uint8_t size) const {
    auto* anInfo = reinterpret_cast<const AnalogMenuInfo*>(info);
    if(isInfoProgMem()) {
        safeProgCpy(unitBuff, anInfo->unitName, size);
    }
    else {
        strncpy(unitBuff, anInfo->unitName, size);
        unitBuff[size - 1] = 0;
    }
}

BooleanNaming BooleanMenuItem::getBooleanNaming() const {
    auto* enumInfo = reinterpret_cast<const BooleanMenuInfo*>(info);
    return isInfoProgMem() ? static_cast<BooleanNaming>(get_info_char(&(enumInfo->naming))) : enumInfo->naming;
}

void FloatMenuItem::setFloatValue(float newVal, bool silent) {
	if(isSame(newVal, currValue)) return;
	
	this->currValue = newVal;
	changeOccurred(silent);
}

int FloatMenuItem::getDecimalPlaces() const {
    auto *fltInfo = reinterpret_cast<const FloatMenuInfo*>(info);
    return isInfoProgMem() ? get_info_int(&fltInfo->numDecimalPlaces) : fltInfo->numDecimalPlaces;
}

void ValueMenuItem::setCurrentValue(uint16_t val, bool silent) {
	if (val == currentValue || val > getMaximumValue()) {
		return;
	}
	currentValue = val;
	changeOccurred(silent);
}

const char ON_STR[] PGM_TCM   = "ON";
const char OFF_STR[] PGM_TCM  = "OFF";
const char YES_STR[] PGM_TCM  = "YES";
const char NO_STR[] PGM_TCM   = "NO";
const char TRUE_STR[] PGM_TCM = "TRUE";
const char FALSE_STR[] PGM_TCM= "FALSE";

void copyMenuItemNameAndValue(const MenuItem* item, char* buffer, size_t bufferSize, char additionalSep) {
    item->copyNameToBuffer(buffer, bufferSize);
    if(additionalSep != 0) appendChar(buffer, additionalSep, bufferSize);
    appendChar(buffer, ' ', bufferSize);

    int pos = strlen(buffer);
    copyMenuItemValue(item, buffer + pos, bufferSize - pos);
}

void copyMenuItemValue(const MenuItem* item, char* buffer, size_t bufferSize) {
    buffer[0] = 0;
    if(item->getMenuType() == MENUTYPE_ENUM_VALUE) {
        auto* enItem = reinterpret_cast<const EnumMenuItem*>(item);
        enItem->copyEnumStrToBuffer(buffer, bufferSize, enItem->getCurrentValue());
    }
    else if(item->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
        auto* boolItem = reinterpret_cast<const BooleanMenuItem*>(item);
        BooleanNaming naming = boolItem->getBooleanNaming();
        const char* val;
        switch(naming) {
            case NAMING_ON_OFF:
                val = boolItem->getBoolean() ? ON_STR : OFF_STR;
                break;
            case NAMING_YES_NO:
                val = boolItem->getBoolean() ? YES_STR : NO_STR;
                break;
            default:
                val = boolItem->getBoolean() ? TRUE_STR : FALSE_STR;
                break;
        }
        safeProgCpy(buffer, val, bufferSize);
    }
    else if(item->getMenuType() == MENUTYPE_FLOAT_VALUE) {
        auto* flItem = reinterpret_cast<const FloatMenuItem*>(item);
        fastftoa(buffer, flItem->getFloatValue(), flItem->getDecimalPlaces(), bufferSize);
    }
    else if(item->getMenuType() == MENUTYPE_INT_VALUE) {
        auto* anItem = reinterpret_cast<const AnalogMenuItem*>(item);
        anItem->copyValue(buffer, bufferSize);
    }
    else if(item->getMenuType() == MENUTYPE_ACTION_VALUE || item->getMenuType() == MENUTYPE_SUB_VALUE) {
        appendChar(buffer, '>', bufferSize);
        appendChar(buffer, '>', bufferSize);
    }
    else if(item->getMenuType() == MENUTYPE_BACK_VALUE) {
        buffer[0]=0;
        if(item->isActive()) {
            strncpy(buffer, "[..]", bufferSize);
        }
    }
    else if(item->getMenuType() == MENUTYPE_TITLE_ITEM) {
        buffer[0] = 0;
    }
    else if(isMenuRuntime(item)) {
        auto* rtItem = reinterpret_cast<const RuntimeMenuItem*>(item);
        rtItem->copyValue(buffer, bufferSize);
    }
}

void copyMenuItemValueDefault(const MenuItem* item, char* buffer, size_t bufferSize, const char* defValue) {
    buffer[0]=0;
    copyMenuItemValue(item, buffer, bufferSize);
    if(strlen(buffer)==0) {
        strncpy(buffer, defValue, bufferSize);
        buffer[bufferSize - 1] = 0;
    }
}