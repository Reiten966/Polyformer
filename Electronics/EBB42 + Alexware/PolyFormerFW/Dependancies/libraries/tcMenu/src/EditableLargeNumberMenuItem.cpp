/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include "EditableLargeNumberMenuItem.h"
#include "tcMenu.h"

void LargeFixedNumber::clear() {
	for (uint8_t i = 0; i < sizeof(bcdRepresentation); i++) bcdRepresentation[i] = 0;
	negative = false;
}

void LargeFixedNumber::setValue(uint32_t whole, uint32_t fraction, bool neg) {
	clear();
	convertToBcdPacked(fraction, 0, fractionDp);
	convertToBcdPacked(whole, fractionDp, totalSize);
	this->negative = neg;
}

uint32_t LargeFixedNumber::fromBcdPacked(int start, int end) {
	int32_t modulo = dpToDivisor((end - start) - 1);
	int32_t res = 0;
	for (int i = start; i < end; i++) {
		res = res + (getDigit(i) * modulo);
		modulo /= 10L;
	}
	return res;
}

void LargeFixedNumber::convertToBcdPacked(uint32_t value, int start, int end) {
	uint32_t modulo = dpToDivisor((end - start) - 1);
	for (int i = start; i < end; i++) {
		setDigit(i, min(9, int(value / modulo)));
		value = value % modulo;
		modulo /= 10L;
	}
}

int LargeFixedNumber::getDigit(int digit) {
	if (digit > 11) return false;
	uint8_t r = bcdRepresentation[digit / 2];
	if ((digit % 2) == 0) {
		return r & 0x0fU;
	}
	else {
		return (int)(r >> 4U);
	}
}

void LargeFixedNumber::setDigit(int digit, int v) {
    auto val = uint8_t(v);
    // prevent exceeding array
	if (digit > 11) return;
	// prevent writing invalid digits - use 0 instead.
	if(v < 0 || v > 9) v = 0;
	if ((digit % 2) == 0) {
		bcdRepresentation[digit / 2] = (bcdRepresentation[digit / 2] & 0xf0) | val;
	}
	else {
		bcdRepresentation[digit / 2] = (bcdRepresentation[digit / 2] & 0x0f) | (val << 4U);
	}
}

float LargeFixedNumber::getAsFloat() {
    if(fractionDp == 0) return (float)getWhole();

    float fraction = ((float)getFraction() / (float)dpToDivisor(fractionDp));
    float asFlt = (float)getWhole() + fraction;
    serlogF3(SER_TCMENU_DEBUG, "fract, asFlt ", fraction, asFlt);
    if (negative) asFlt = -asFlt;
    return asFlt;
}

void LargeFixedNumber::setFromFloat(float value) {
    bool neg = value < 0.0f;
    value = tcFltAbs(value);
    uint32_t val;
    uint32_t frc;
    if(fractionDp == 0) {
        val = (uint32_t)value;
        frc = 0;
    }
    else {
        val = (uint32_t)value;
        frc = (uint32_t)((value - (float) val) * ((float)dpToDivisor(fractionDp) + 0.5F));
    }
    setValue(val, frc, neg);
}

void EditableLargeNumberMenuItem::setLargeNumberFromString(const char* val) {
	int offset = 0;
	bool negative = false;
	if (val && val[0] == '-') {
		offset++;
		negative = true;
	}
	int32_t whole = parseIntUntilSeparator(val, offset);
	int32_t fract = parseIntUntilSeparator(val, offset, getLargeNumber()->decimalPointIndex());
	data.setValue(whole, fract, negative);
	setSendRemoteNeededAll();
	setChanged(true);
}

void wrapEditor(bool editRow, uint8_t row, char val, char* buffer, int bufferSize) {
	if (editRow) {
        menuMgr.setEditorHints(CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT, row, row+1);
	}
    appendChar(buffer, val, bufferSize);
}

int largeNumItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	if (item->getMenuType() != MENUTYPE_LARGENUM_VALUE) return 0;
	auto numItem = reinterpret_cast<EditableLargeNumberMenuItem*>(item);
	auto negativeAllowed = numItem->isNegativeAllowed();
	LargeFixedNumber *num = numItem->getLargeNumber();
    int numParts = numItem->getNumberOfParts() - (negativeAllowed ? 1 : 0);

	switch (mode) {
	case RENDERFN_VALUE: {
		buffer[0] = 0;
		bool editingMode = row > 0 && row < 0xf;
		bool hadNonZero = false;
		uint8_t editPosition = 0;

		if (negativeAllowed && (editingMode ||  num->isNegative())) {
            row = row - 1;
            wrapEditor(row == 0, row, num->isNegative() ? '-' : '+', buffer, bufferSize);
		}

		row = row - 1;

		for (int i = num->decimalPointIndex(); i < numParts; i++) {
			char txtVal = num->getDigit(i) + '0';
			hadNonZero |= txtVal != '0';
            bool lastDigit = i == (numParts - 1);
			if (hadNonZero || editingMode || lastDigit) {
				wrapEditor(row == editPosition, row + 1, txtVal, buffer, bufferSize);
			}
			editPosition++;
		}
		if(num->decimalPointIndex() != 0) appendChar(buffer, '.', bufferSize);

		for (int i = 0; i < num->decimalPointIndex(); i++) {
			char txtVal = num->getDigit(i) + '0';
			wrapEditor(row == editPosition, row + 2, txtVal, buffer, bufferSize);
			editPosition++;
		}
		return true;
	}
	case RENDERFN_GETRANGE: {
		if(negativeAllowed) {
		    return row == 1 ? 1 : 9;
		}
		else {
		    return 9;
		}
	}
	case RENDERFN_SET_VALUE: {
		int idx = row - 1;
		if(negativeAllowed) {
            if (idx == 0) {
                num->setNegative(buffer[0]);
                return true;
            }
            idx--;
		}
		int dpIndex = (numParts) - num->decimalPointIndex();
		int pos = idx >= dpIndex ? idx - dpIndex : idx + num->decimalPointIndex();
		num->setDigit(pos, buffer[0]);
		return true;
	}
	case RENDERFN_NAME: {
		if (buffer) buffer[0] = 0;
		return true;
	}
	case RENDERFN_GETPART: {
		int idx = row - 1;
		if(negativeAllowed) {
            if (idx == 0) {
                return num->isNegative();
            }
            idx--;
        }
		int dpIndex = (numParts) - num->decimalPointIndex();
		int pos =  idx >= dpIndex ? idx - dpIndex : idx + num->decimalPointIndex();
		return num->getDigit(pos);
	}
	default: return false;
	}
}