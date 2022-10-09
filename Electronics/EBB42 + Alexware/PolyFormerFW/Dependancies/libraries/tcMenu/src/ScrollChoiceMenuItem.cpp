/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "ScrollChoiceMenuItem.h"
#include <IoLogging.h>

ScrollChoiceMenuItem::ScrollChoiceMenuItem(int id, RuntimeRenderingFn renderFn, uint8_t currentSel,
                                           const char *enumItemsInRam, int itemSize, int numberOfItems,
                                           MenuItem *next)
        : RuntimeMenuItem(MENUTYPE_SCROLLER_VALUE, id, renderFn, currentSel, numberOfItems, next) {
    memMode = MEMORY_ONLY;
    rangeValues = enumItemsInRam;
    this->itemSize = itemSize;
    eepromStart = 0;
    lastCacheSize = 0;
}

ScrollChoiceMenuItem::ScrollChoiceMenuItem(uint16_t id, RuntimeRenderingFn renderFn, uint8_t currentSel,
                                           EepromPosition eepromStart, int itemSize, int numberOfItems, MenuItem *next)
        : RuntimeMenuItem(MENUTYPE_SCROLLER_VALUE, id, renderFn, currentSel, numberOfItems, next) {
    memMode = EEPROM_BASED;
    rangeValues = nullptr;
    this->itemSize = itemSize;
    this->eepromStart = eepromStart;
    lastCacheSize = 0;
}

ScrollChoiceMenuItem::ScrollChoiceMenuItem(uint16_t id, RuntimeRenderingFn renderFn, uint8_t currentSel,
                                           int numberOfItems, MenuItem *next)
        : RuntimeMenuItem(MENUTYPE_SCROLLER_VALUE, id, renderFn, currentSel, numberOfItems, next) {
    memMode = CUSTOM;
    rangeValues = nullptr;
    this->itemSize = 0;
    this->eepromStart = 0;
    lastCacheSize = 0;
}

void ScrollChoiceMenuItem::cacheEepromValues() {
    if(memMode != EEPROM_BASED) return;

    if(noOfParts != lastCacheSize) {
        delete[] rangeValues;
    }

    rangeValues = new char[noOfParts * itemSize];
    menuMgr.getEepromAbstraction()->readIntoMemArray((uint8_t*)rangeValues, eepromStart, itemSize * noOfParts);
}

void ScrollChoiceMenuItem::setCurrentValue(int val, bool silent) {
    if(val < 0 || val >= noOfParts || val == itemPosition) return;
    itemPosition = val;
    setChanged(true);
    setSendRemoteNeededAll();
    if(!silent)	triggerCallback();
}

void ScrollChoiceMenuItem::valueAtPosition(char *buffer, size_t bufferSize, int idx) {
    if(idx >= getNumberOfRows()) {
        buffer[0] = 0;
        return;
    }

    auto safeSize = min((size_t)itemSize, bufferSize - 1);

    if (rangeValues != nullptr) {
        serlogF4(SER_TCMENU_DEBUG, "Start Cached ", idx, (int)rangeValues, itemSize);
        strncpy(buffer, &rangeValues[itemSize * idx], safeSize);
        buffer[safeSize] = 0;
        serlogF2(SER_TCMENU_DEBUG, "Cached ", idx);
    }
    else if(memMode == EEPROM_BASED) {
        if(!menuMgr.getEepromAbstraction()) {
            strncpy(buffer, "!ROM", bufferSize);
            serlogF(SER_TCMENU_DEBUG, "No Rom set");
        }
        else {
            EepromPosition position = eepromStart + (idx * itemSize);
            menuMgr.getEepromAbstraction()->readIntoMemArray((uint8_t *) buffer, position, safeSize);
            serlogF3(SER_TCMENU_DEBUG, "Rom ", idx, position);
        }
        buffer[safeSize] = 0;
    }
    else if(memMode == CUSTOM){
        renderFn(this, idx, RENDERFN_VALUE, buffer, bufferSize);
        serlogF2(SER_TCMENU_DEBUG, "Custom ", idx);
    }
}

void ScrollChoiceMenuItem::copyTransportText(char *buffer, size_t bufferSize) {
    ltoaClrBuff(buffer, getCurrentValue(), 3, NOT_PADDED, bufferSize);
    appendChar(buffer, '-', bufferSize);
    size_t len = strlen(buffer);
    copyValue(&buffer[len], (int)(bufferSize - len));
    buffer[bufferSize - 1] = 0;
}

void ScrollChoiceMenuItem::setFromRemote(const char *buffer) {
    int pos = 0;
    setCurrentValue((int)parseIntUntilSeparator(buffer, pos));
}

int enumItemRenderFn(RuntimeMenuItem *item, uint8_t row, RenderFnMode mode, char *buffer, int bufferSize) {
    if (item->getMenuType() != MENUTYPE_SCROLLER_VALUE) return 0;
    auto scrollItem = reinterpret_cast<ScrollChoiceMenuItem*>(item);

    switch (mode) {
        case RENDERFN_VALUE: {
            scrollItem->valueAtPosition(buffer, bufferSize, row);
            return true;
        }
        case RENDERFN_NAME: {
            if (buffer) buffer[0] = 0;
            return true;
        }
        case RENDERFN_EEPROM_POS: return -1;
        default: return false;
    }
}

Rgb32MenuItem::Rgb32MenuItem(uint16_t id, RuntimeRenderingFn renderFn, bool includeAlpha, MenuItem *next)
        : EditableMultiPartMenuItem(MENUTYPE_COLOR_VALUE, id, includeAlpha ? 4 : 3, renderFn, next) {
    alphaChannel = includeAlpha;
    if (!alphaChannel) data.alpha = 255;
}

uint8_t hexValueOf(char val) {
    if(val >= '0' && val <= '9') return val - '0';
    val = (char)toupper(val);
    if(val >= 'A' && val <= 'F') return val - ('A' - 10);
    return 0;
}

RgbColor32::RgbColor32(const char *htmlColor) {
    alpha = 255;
    auto len = strlen(htmlColor);
    if(htmlColor[0] == '#' && len == 4) { // #rgb
        red = hexValueOf(htmlColor[1]) << 4U;
        green = hexValueOf(htmlColor[2]) << 4U;
        blue = hexValueOf(htmlColor[3]) << 4U;
        return;
    }
    else if(htmlColor[0] == '#' && len >= 7) { // #rrggbb
        red = (hexValueOf(htmlColor[1]) << 4U) + hexValueOf(htmlColor[2]);
        green = (hexValueOf(htmlColor[3]) << 4U) + hexValueOf(htmlColor[4]);
        blue = (hexValueOf(htmlColor[5]) << 4U) + hexValueOf(htmlColor[6]);
        if(len == 9) {
            alpha = (hexValueOf(htmlColor[7]) << 4U) + hexValueOf(htmlColor[8]);
        }
        return;
    }

    red = green = blue = 0;
}

char hexChar(uint8_t val) {
    if(val < 9) return char(val + '0');
    return char(val - 10) + 'A';
}

void RgbColor32::asHtmlString(char *buffer, size_t bufferSize, bool includeAlpha) const {
    if(bufferSize < 8) {
        buffer[0] = 0;
        return;
    }

    buffer[0] = '#';
    buffer[1] = hexChar(red >> 4U);
    buffer[2] = hexChar(red & 0x0fU);
    buffer[3] = hexChar(green >> 4U);
    buffer[4] = hexChar(green & 0x0fU);
    buffer[5] = hexChar(blue >> 4U);
    buffer[6] = hexChar(blue & 0x0fU);
    if(includeAlpha && bufferSize > 9) {
        buffer[7] = hexChar(alpha >> 4U);
        buffer[8] = hexChar(alpha & 0xfU);
        buffer[9] = 0;
    }
    else {
        buffer[7] = 0;
    }

}
void wrapForEdit(int val, int idx, uint8_t row, char* buffer, int bufferSize, bool forTime = false);
int rgbAlphaItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    if (item->getMenuType() != MENUTYPE_COLOR_VALUE) return 0;
    auto rgbItem = reinterpret_cast<Rgb32MenuItem*>(item);
    auto idx = row - 1;
    auto data = rgbItem->getUnderlying();

    switch(mode) {
        case RENDERFN_NAME: {
            if (buffer) buffer[0] = 0;
            return true;
        }
        case RENDERFN_VALUE: {
            buffer[0] = 'R';
            buffer[1] = 0;
            wrapForEdit(data->red, 0, row, buffer, bufferSize, false);
            appendChar(buffer, ' ', bufferSize);
            appendChar(buffer, 'G', bufferSize);
            wrapForEdit(data->green, 1, row, buffer, bufferSize, false);
            appendChar(buffer, ' ', bufferSize);
            appendChar(buffer, 'B', bufferSize);
            wrapForEdit(data->blue, 2, row, buffer, bufferSize, false);
            if(rgbItem->isAlphaInUse()) {
                appendChar(buffer, ' ', bufferSize);
                appendChar(buffer, 'A', bufferSize);
                wrapForEdit(data->alpha, 3, row, buffer, bufferSize, false);
            }
            return true;
        }
        case RENDERFN_GETRANGE: {
            return 255;
        }
        case RENDERFN_GETPART: {
            switch(idx) {
                case 0:  return data->red;
                case 1:  return data->green;
                case 2:  return data->blue;
                default: return data->alpha;
            }
        }

        case RENDERFN_SET_VALUE: {
            switch(idx) {
                case 0:
                    data->red = *buffer;
                    break;
                case 1:
                    data->green = *buffer;
                    break;
                case 2:
                    data->blue = *buffer;
                    break;
                default:
                    data->alpha = *buffer;
                    break;
            }
            return true;
        }
        default: return false;
    }
}
