/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * LiquidCrystalIO renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This renderer is designed for use with this library: https://github.com/davetcc/LiquidCrystalIO
 */

#include "tcMenuLiquidCrystal.h"
#include "tcUtil.h"

extern const ConnectorLocalInfo applicationInfo;

LiquidCrystalRenderer::LiquidCrystalRenderer(LiquidCrystal& lcd, int dimX, int dimY) : BaseGraphicalRenderer(dimX, dimX, dimY, true, applicationInfo.name) {
    this->lcd = &lcd;
    this->backChar = '<';
    this->forwardChar = '>';
    this->editChar = '=';
}

void LiquidCrystalRenderer::initialise() {
    // first we create the custom characters for any title widget.
    // we iterate over each widget then over each each icon.
    TitleWidget* wid = firstWidget;
    int charNo = 0;
    while(wid != nullptr) {
        serdebugF2("Title widget present max=", wid->getMaxValue());
        for(int i = 0; i < wid->getMaxValue(); i++) {
            serdebugF2("Creating char ", charNo);
            lcd->createCharPgm((uint8_t)charNo, wid->getIcon(i));
            charNo++;
        }
        wid = wid->getNext();
    }
    lcd->clear();

    BaseGraphicalRenderer::initialise();
}

LiquidCrystalRenderer::~LiquidCrystalRenderer() {
    delete this->buffer;
    delete dialog;
}

void LiquidCrystalRenderer::setEditorChars(char back, char forward, char edit) {
    backChar = back;
    forwardChar = forward;
    editChar = edit;
}

void LiquidCrystalRenderer::drawWidget(Coord where, TitleWidget *widget, color_t, color_t) {
    char ch = char(widget->getHeight() + widget->getCurrentState());
    serdebugF4("draw widget", where.x, where.y, (int)ch);
    lcd->setCursor(where.x, where.y);
    widget->setChanged(false);
    lcd->write(ch);
}

int calculateOffset(GridPosition::GridJustification just, int totalLen, const char* sz) {
    int len = strlen(sz);
    auto actualJust = coreJustification(just);
    if(len > totalLen || actualJust == GridPosition::CORE_JUSTIFY_LEFT) return 0;

    if(actualJust == tcgfx::GridPosition::CORE_JUSTIFY_RIGHT) {
        return (totalLen - len) - 1;
    }
    else {
        // must be centered in this case.
        return (totalLen - len) / 2;
    }
}

void copyIntoBuffer(char* buffer, const char* source, int offset, int bufferLen) {
    int len = strlen(source);
    for(int i=0; i<len; i++) {
        auto pos = offset+i;
        if(pos >= bufferLen) return;
        buffer[pos] = source[i];
    }
}

void LiquidCrystalRenderer::drawMenuItem(GridPositionRowCacheEntry* entry, Coord where, Coord areaSize, bool /*ignored*/) {
    auto* theItem = entry->getMenuItem();
    theItem->setChanged(false);


    if(entry->getPosition().getJustification() == GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT) {
        buffer[0] = theItem->isEditing() ? editChar : (theItem->isActive() ? forwardChar : ' ');
        lcd->setCursor(where.x, where.y);
        int offs = 1;
        uint8_t finalPos = theItem->copyNameToBuffer(buffer, offs, bufferSize);
        for(uint8_t i = finalPos; i < areaSize.x; ++i)  buffer[i] = 32;
        buffer[bufferSize] = 0;
        menuValueToText(theItem, JUSTIFY_TEXT_RIGHT);
    }
    else {
        char sz[21];
        for(uint8_t i = 1; i < (sizeof(sz) - 1); ++i)  buffer[i] = 32;
        buffer[sizeof(sz)-1] = 0;
        uint8_t valueStart = 0;
        if(itemNeedsName(entry->getPosition().getJustification())) {
            theItem->copyNameToBuffer(sz, sizeof sz);
            valueStart += strlen(sz);
        }
        if(itemNeedsValue(entry->getPosition().getJustification())) {
            sz[valueStart] = 32;
            valueStart++;
            copyMenuItemValue(entry->getMenuItem(), sz + valueStart, sizeof(sz) - valueStart);
            serdebugF2("Value ", sz);
        }
        int position = calculateOffset(entry->getPosition().getJustification(), areaSize.x + 1, sz);
        copyIntoBuffer(&buffer[1], sz, position, bufferSize);
        buffer[0] = theItem->isEditing() ? editChar : (theItem->isActive() ? forwardChar : ' ');
        buffer[min(uint8_t(areaSize.x + 1), bufferSize)] = 0;
        lcd->setCursor(where.x, where.y);
    }
    serdebugF4("Buffer: ", where.x,where.y, buffer);
    lcd->print(buffer);
}

void LiquidCrystalRenderer::drawingCommand(RenderDrawingCommand command) {
    switch (command) {
        case DRAW_COMMAND_CLEAR:
            lcd->clear();
            break;
        default:
            break;
    }
}

void LiquidCrystalRenderer::fillWithBackgroundTo(int endPoint) {
    for(uint16_t y=endPoint;y<height;++y) {
        lcd->setCursor(0, y);
        for(uint16_t x=0;x<width;x++) {
            lcd->print(' ');
        }
    }
}

BaseDialog* LiquidCrystalRenderer::getDialog() {
    if(dialog == nullptr) {
        dialog = new MenuBasedDialog();
    }
    return dialog;
}
