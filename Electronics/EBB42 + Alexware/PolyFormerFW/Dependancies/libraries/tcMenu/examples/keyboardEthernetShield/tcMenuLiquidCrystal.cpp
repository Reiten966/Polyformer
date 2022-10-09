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
        serlogF2(SER_TCMENU_INFO, "Title widget present max=", wid->getMaxValue());
        for(int i = 0; i < wid->getMaxValue(); i++) {
            serlogF2(SER_TCMENU_DEBUG, "Creating char ", charNo);
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
    serlogF4(SER_TCMENU_DEBUG, "draw widget", where.x, where.y, (int)ch);
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
    auto len = strlen(source);
    for(size_t i=0; i<len; i++) {
        auto pos = offset+i;
        if(pos >= bufferLen) return;
        buffer[pos] = source[i];
    }
}

void LiquidCrystalRenderer::drawMenuItem(GridPositionRowCacheEntry* entry, Coord where, Coord areaSize, bool /*ignored*/) {
    auto* theItem = entry->getMenuItem();
    theItem->setChanged(false);
    char sz[21];

    if(entry->getPosition().getJustification() == GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT) {
        buffer[0] = theItem->isEditing() ? editChar : (theItem->isActive() ? forwardChar : ' ');
        lcd->setCursor(where.x, where.y);
        int offs = 1;
        uint8_t finalPos = theItem->copyNameToBuffer(buffer, offs, bufferSize);
        for(uint8_t i = finalPos; i < uint8_t(areaSize.x); ++i)  buffer[i] = 32;
        buffer[bufferSize] = 0;
        copyMenuItemValue(theItem, sz, sizeof sz);
        uint8_t count = strlen(sz);
        int cpy = bufferSize - count;
        strcpy(buffer + cpy, sz);
        if(theItem == menuMgr.getCurrentEditor() && menuMgr.getEditorHints().getEditorRenderingType() != CurrentEditorRenderingHints::EDITOR_REGULAR) {
            setupEditorPlacement(where.x + cpy + menuMgr.getEditorHints().getStartIndex(), where.y);
        }
    }
    else {
        for(size_t i = 1; i < (sizeof(sz) - 1); ++i)  buffer[i] = 32;
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
            serlogF2(SER_TCMENU_DEBUG, "Value ", sz);
        }
        int position = calculateOffset(entry->getPosition().getJustification(), int(areaSize.x) + 1, sz);
        copyIntoBuffer(&buffer[1], sz, position, bufferSize);
        buffer[0] = theItem->isEditing() ? editChar : (theItem->isActive() ? forwardChar : ' ');
        buffer[min(uint8_t(areaSize.x + 1), bufferSize)] = 0;
        lcd->setCursor(where.x, where.y);
        if(theItem == menuMgr.getCurrentEditor() && menuMgr.getEditorHints().getEditorRenderingType() != CurrentEditorRenderingHints::EDITOR_REGULAR) {
            setupEditorPlacement(where.x + valueStart + menuMgr.getEditorHints().getStartIndex(), where.y);
        }
    }
    serlogF4(SER_TCMENU_DEBUG, "Buffer: ", where.x,where.y, buffer);
    lcd->print(buffer);
}

void LiquidCrystalRenderer::setupEditorPlacement(int32_t x, int32_t y) {
    lcdEditorCursorX = min((width - 1), x);
    lcdEditorCursorY = y;
}

void LiquidCrystalRenderer::drawingCommand(RenderDrawingCommand command) {
    switch (command) {
        case DRAW_COMMAND_CLEAR:
            lcd->clear();
            break;
        case DRAW_COMMAND_START:
            if(lcdEditorCursorX != 0xFF) {
                if(menuMgr.getCurrentEditor() == nullptr) {
                    lcdEditorCursorX = 0xFF; // edit has ended, clear our status
                    lcdEditorCursorY = 0xFF;
                }
                lcd->noCursor(); // always turn off the cursor while we draw
            }
        case DRAW_COMMAND_ENDED:
            if(lcdEditorCursorX != 0xFF) {
                lcd->setCursor(lcdEditorCursorX, lcdEditorCursorY);
                serlogF3(SER_TCMENU_DEBUG, "Editor cursor: ", lcdEditorCursorX, lcdEditorCursorY);
                lcd->cursor(); // re-enable the cursor after drawing.
            }
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
