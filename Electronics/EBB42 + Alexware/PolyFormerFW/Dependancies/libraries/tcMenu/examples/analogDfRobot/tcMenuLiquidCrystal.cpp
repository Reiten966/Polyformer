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

LiquidCrystalRenderer::LiquidCrystalRenderer(LiquidCrystal& lcd, uint8_t dimX, uint8_t dimY) : BaseMenuRenderer(dimX) {
    this->dimY = dimY;
    this->lcd = &lcd;
    this->backChar = '<';
    this->forwardChar = '>';
    this->editChar = '=';
    this->drewTitleThisTime = false;
    this->titleRequired = true;
}

void LiquidCrystalRenderer::initialise() {
    // first we create the custom characters for any title widget.
    // we iterate over each widget then over each each icon.
    TitleWidget* wid = firstWidget;
    int charNo = 0;
    while(wid != NULL) {
        for(int i = 0; i < wid->getMaxValue(); i++) {
            lcd->createCharPgm((uint8_t)charNo, wid->getIcon(i));
            charNo++;
        }
        wid = wid->getNext();
    }
    lcd->clear();
    BaseMenuRenderer::initialise();
}

LiquidCrystalRenderer::~LiquidCrystalRenderer() {
    delete this->buffer;
    if(dialog) delete dialog;
}

void LiquidCrystalRenderer::setEditorChars(char back, char forward, char edit) {
    backChar = back;
    forwardChar = forward;
    editChar = edit;
}

void LiquidCrystalRenderer::renderList() {
    ListRuntimeMenuItem* runList = reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu());

    uint8_t maxY = min(dimY, runList->getNumberOfParts());
    uint8_t currentActive = runList->getActiveIndex();

    uint8_t offset = 0;
    if (currentActive >= maxY) {
        offset = (currentActive+1) - maxY;
    }

    for (int i = 0; i < maxY; i++) {
        uint8_t current = offset + i;
        RuntimeMenuItem* toDraw = (current==0) ? runList->asBackMenu() : runList->getChildItem(current - 1);
        renderMenuItem(i, toDraw);
    }

    // reset the list item to a normal list again.
    runList->asParent();
}

void LiquidCrystalRenderer::renderTitle(bool forceDraw) {
    if(!drewTitleThisTime || forceDraw) {
        strcpy_P(buffer, applicationInfo.name);
        uint8_t bufSz = bufferSize;
        uint8_t last = min(bufSz, (uint8_t)strlen(buffer));
        for(uint8_t i = last; i < bufSz; i++) {
            buffer[i] = ' ';
        }
        buffer[bufSz] = 0;
        lcd->setCursor(0,0);
        lcd->print(buffer);
    }

    uint8_t widCount = 0;
    uint8_t charOffset = 0;
    TitleWidget* widget = firstWidget;
    while(widget != NULL) {
        if(widget->isChanged() || forceDraw) {
            lcd->setCursor(bufferSize - (widCount + 1), 0);
            widget->setChanged(false);
            lcd->write(charOffset + widget->getCurrentState());
        }
        charOffset += widget->getMaxValue();
        widget = widget->getNext();
        widCount++;
    }

}

void LiquidCrystalRenderer::render() {
    uint8_t locRedrawMode = redrawMode;
    redrawMode = MENUDRAW_NO_CHANGE;
    if (locRedrawMode == MENUDRAW_COMPLETE_REDRAW) {
        lcd->clear();
    }

    countdownToDefaulting();

    if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST ) {
        if (menuMgr.getCurrentMenu()->isChanged() || locRedrawMode != MENUDRAW_NO_CHANGE) {
            renderList();
        }
    }
    else {
        MenuItem* item = menuMgr.getCurrentMenu();

        if(lcdEditorCursorX != 0xFF) {
            if(menuMgr.getCurrentEditor() == nullptr) {
                lcdEditorCursorX = 0xFF; // edit has ended, clear our status
                lcdEditorCursorY = 0xFF;
            }
            lcd->noCursor(); // always turn off the cursor while we draw
        }

        bool titleNeeded = titleRequired && menuMgr.getCurrentMenu() == menuMgr.getRoot();

        // first we find the first currently active item in our single linked list
        int activeOffs = offsetOfCurrentActive(item);

        uint8_t cnt = 0;
        uint8_t numLines = dimY;
        if(titleNeeded && activeOffs <= (dimY - 2)) {
            renderTitle(locRedrawMode != MENUDRAW_NO_CHANGE);
            cnt++;
            if(!drewTitleThisTime) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
            drewTitleThisTime = true;
            numLines--;
        }
        else {
            if(drewTitleThisTime) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
            drewTitleThisTime = false;
        }

        if (activeOffs >= numLines) {
            uint8_t toOffsetBy = (activeOffs - dimY) + 1;

            if (lastOffset != toOffsetBy) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
            lastOffset = toOffsetBy;

            while (item != NULL && toOffsetBy) {
                if(item->isVisible()) toOffsetBy = toOffsetBy - 1;
                item = item->getNext();
            }
        }
        else {
            if (lastOffset != 0xff) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
            lastOffset = 0xff;
        }

        // and then we start drawing items until we run out of screen or items
        while (item && cnt < dimY) {
            if(item->isVisible())
            {
                if (locRedrawMode != MENUDRAW_NO_CHANGE || item->isChanged()) {
                    renderMenuItem(cnt, item);
                }
                ++cnt;
            }
            item = item->getNext();
        }

        if(lcdEditorCursorX != 0xFF) {
            lcd->setCursor(lcdEditorCursorX, lcdEditorCursorY);
            lcd->cursor(); // re-enable the cursor after drawing.
        }
    }
}

void LiquidCrystalRenderer::renderMenuItem(uint8_t row, MenuItem* item) {
    if (item == NULL || row > dimY) return;

    item->setChanged(false);
    lcd->setCursor(0, row);

    int offs;
    if (item->getMenuType() == MENUTYPE_BACK_VALUE) {
        buffer[0] = item->isActive() ? backChar : ' ';
        buffer[1] = backChar;
        offs = 2;
    }
    else {
        buffer[0] = item->isEditing() ? editChar : (item->isActive() ? forwardChar : ' ');
        offs = 1;
    }
    uint8_t finalPos = item->copyNameToBuffer(buffer, offs, bufferSize);
    for(uint8_t i = finalPos; i < bufferSize; ++i)  buffer[i] = 32;
    buffer[bufferSize] = 0;

    if (isItemActionable(item)) {
        buffer[bufferSize - 1] = forwardChar;
    }
    else {
        char sz[20];
        copyMenuItemValue(item, sz, sizeof sz);
        uint8_t count = strlen(sz);
        int cpy = bufferSize - count;
        strcpy(buffer + cpy, sz);
        if(item == menuMgr.getCurrentEditor() && menuMgr.getEditorHints().getEditorRenderingType() != CurrentEditorRenderingHints::EDITOR_REGULAR) {
            setupEditorPlacement(cpy + menuMgr.getEditorHints().getStartIndex(), row);
        }
    }
    lcd->print(buffer);
}

BaseDialog* LiquidCrystalRenderer::getDialog() {
    if(dialog == NULL) {
        dialog = new LiquidCrystalDialog(this);
    }
    return dialog;
}

// dialog

void LiquidCrystalDialog::internalRender(int currentValue) {
    LiquidCrystalRenderer* lcdRender = ((LiquidCrystalRenderer*)MenuRenderer::getInstance());
    LiquidCrystal* lcd = lcdRender->getLCD();
    if(needsDrawing == MENUDRAW_COMPLETE_REDRAW) {
        lcd->clear();
    }

    char data[20];
    strncpy_P(data, headerPgm, sizeof(data));
    data[sizeof(data)-1]=0;
    lcd->setCursor(0,0);
    lcd->print(data);

    // we can only print the buffer on a newline when there's enough rows.
    // so on 16x2 we have to show the buffer over the header. It's all we
    // can do.
    int nextY = 3;
    if(isCompressedMode()) {
        int len = strlen(lcdRender->getBuffer());
        int startX = lcdRender->getBufferSize() - len;
        lcd->setCursor(startX,0);
        lcd->print(lcdRender->getBuffer());
        nextY = 1;
    }
    else {
        lcd->setCursor(0,1);
        lcd->print(lcdRender->getBuffer());
    }

    if(button1 != BTNTYPE_NONE) {
        copyButtonText(data, 0, currentValue);
        lcd->setCursor(0, nextY);
        lcd->print(data);
    }
    if(button2 != BTNTYPE_NONE) {
        copyButtonText(data, 1, currentValue);
        int startX = lcdRender->getBufferSize() - strlen(data);
        lcd->setCursor(startX, nextY);
        lcd->print(data);
    }
}

void LiquidCrystalRenderer::setupEditorPlacement(int32_t x, int32_t y) {
    lcdEditorCursorX = min((bufferSize - 1), x);
    lcdEditorCursorY = y;
}
