/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * NOTE: Be aware that the underlying SSD1306ascii library is GPL and linking with it will essentially make your code GPL
 *
 */

/**
 * @file ssd1306ascii.h
 *
 *  ssd1306ascii renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 *
 * This plugin allows rendering to the Ssd1306Ascii library. It is a low memory ascii renderer that
 * provides text based functions.
 *
 * LIBRARY REQUIREMENT
 * This renderer is designed for use with this library: https://github.com/greiman/SSD1306Ascii
 */
#include "ssd1306asciiRenderer.h"

extern const ConnectorLocalInfo applicationInfo;

SSD1306AsciiRenderer::SSD1306AsciiRenderer(uint8_t dimX, const uint8_t* titleFont, const uint8_t* itemFont) : BaseMenuRenderer(dimX) {
	this->backChar = '<';
	this->forwardChar = '>';
	this->editChar = '=';
	this->ssd1306 = nullptr;
    this->fontTitle = titleFont;
    this->fontItem = itemFont;
}

SSD1306AsciiRenderer::~SSD1306AsciiRenderer() {
    delete this->buffer;
    delete dialog;
}

void SSD1306AsciiRenderer::setEditorChars(char back, char forward, char edit) {
	backChar = back;
	forwardChar = forward;
	editChar = edit;
}

void SSD1306AsciiRenderer::renderList(uint8_t titleRows) {
	auto runList = reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu());

	uint8_t maxY = min((ssd1306->displayRows() - titleRows), runList->getNumberOfParts());
	uint8_t currentActive = runList->getActiveIndex();

	uint8_t offset = 0;
	if (currentActive >= maxY) {
		offset = (currentActive+1) - maxY;
	}

	for (int i = 0; i < maxY; i++) {
		uint8_t current = offset + i;
		RuntimeMenuItem* toDraw = (current==0) ? runList->asBackMenu() : runList->getChildItem(current - 1);
		renderMenuItem(i + titleRows, toDraw);
	}

	// reset the list item to a normal list again.
	runList->asParent();
}

void SSD1306AsciiRenderer::renderTitle() {
    if(menuMgr.getCurrentMenu() == menuMgr.getRoot()) {
        safeProgCpy(buffer, applicationInfo.name, bufferSize);
    }
    else {
        menuMgr.getCurrentMenu()->copyNameToBuffer(buffer, bufferSize);
    }
    serdebugF2("print app name", buffer);
    size_t bufSz = bufferSize;
    size_t last = min(bufSz, strlen(buffer));
    for(uint8_t i = last; i < bufSz; i++) {
        buffer[i] = ' ';
    }
    buffer[bufSz] = 0;
    ssd1306->setFont(fontTitle);
    ssd1306->setInvertMode(true);
    ssd1306->setCursor(0,0);
    ssd1306->print(buffer);
    ssd1306->setInvertMode(false);
}

void SSD1306AsciiRenderer::render() {
	uint8_t locRedrawMode = redrawMode;
	redrawMode = MENUDRAW_NO_CHANGE;

	countdownToDefaulting();

    ssd1306->setFont(fontTitle);
    int titleRows = ssd1306->fontRows();
	if (locRedrawMode == MENUDRAW_COMPLETE_REDRAW) {
        ssd1306->clear();
		renderTitle();
        taskManager.yieldForMicros(0);
	}

	if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST ) {
		if (menuMgr.getCurrentMenu()->isChanged() || locRedrawMode != MENUDRAW_NO_CHANGE) {
			renderList(titleRows);
		}
	} else {
        int cnt = titleRows;
		MenuItem* item = menuMgr.getCurrentMenu();

		// first we find the first currently active item in our single linked list
        int activeOffs = offsetOfCurrentActive(item);

        int rowsAvailable = ssd1306->displayRows() - titleRows;
		if (activeOffs >= rowsAvailable) {
			uint8_t toOffsetBy = (activeOffs - rowsAvailable) + 1;

			if (lastOffset != toOffsetBy) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
			lastOffset = toOffsetBy;

			while (item != nullptr && toOffsetBy) {
                if(item->isVisible()) toOffsetBy = toOffsetBy - 1;
				item = item->getNext();
			}
		} else {
			if (lastOffset != 0xff) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
			lastOffset = 0xff;
		}

		// and then we start drawing items until we run out of screen or items
		while (item && cnt < ssd1306->displayRows()) {
            if(item->isVisible())
            {
                if (locRedrawMode != MENUDRAW_NO_CHANGE || item->isChanged()) {
                    renderMenuItem(cnt, item);
                }
                ++cnt;
            }
			item = item->getNext();
		}
	}
}

void SSD1306AsciiRenderer::renderMenuItem(uint8_t row, MenuItem* item) {
	if (item == nullptr || row > ssd1306->displayRows()) return;

	item->setChanged(false);
	ssd1306->setCursor(0, row);

	int offs;
	if (item->getMenuType() == MENUTYPE_BACK_VALUE) {
		buffer[0] = item->isActive() ? (char)backChar : ' ';
		buffer[1] = (char)backChar;
		offs = 2;
	}
	else {
		buffer[0] = item->isEditing() ? editChar : (item->isActive() ? forwardChar : ' ');
		offs = 1;
	}
    uint8_t finalPos = item->copyNameToBuffer(buffer, offs, bufferSize);
	for(uint8_t i = finalPos; i < bufferSize; ++i)  buffer[i] = 32;
	buffer[bufferSize] = 0;

    ssd1306->setFont(fontItem);

	if (isItemActionable(item)) {
		buffer[bufferSize - 2] = (char)forwardChar;
        buffer[bufferSize - 1] = 0;
        ssd1306->print(buffer);
    }
	else {
        char sz[20];
        copyMenuItemValue(item, sz, sizeof sz);
        uint8_t count = strlen(sz);
        if(count < 0 || count > bufferSize) {
            return;
        }
        int cpy = (bufferSize - count) - 1;

        auto hints = menuMgr.getEditorHints();
        if(menuMgr.getCurrentEditor() && hints.getEditorRenderingType() != CurrentEditorRenderingHints::EDITOR_REGULAR && item->isEditing()) {
            int startIndex = min(count, hints.getStartIndex());
            int endIndex = min(count, hints.getEndIndex());
            strncpy(buffer + cpy, sz, startIndex);
            buffer[cpy + startIndex] = 0;
            ssd1306->print(buffer);
            if(startIndex != endIndex) {
                ssd1306->setInvertMode(true);
                strncpy(buffer, &sz[startIndex], endIndex - startIndex);
                buffer[endIndex - startIndex] = 0;
                ssd1306->print(buffer);
                ssd1306->setInvertMode(false);
            }
            strncpy(buffer, &sz[endIndex], bufferSize);
            buffer[bufferSize-1]=0;
            ssd1306->print(buffer);
        } else {
            strcpy(buffer + cpy, sz);
            buffer[bufferSize - 1] = 0;
            serdebugF3("Buffer: ", row, buffer);
            ssd1306->print(buffer);
        }
    }
}

BaseDialog* SSD1306AsciiRenderer::getDialog() {
    if(dialog == nullptr) {
        dialog = new SSD1306AsciiDialog(this);
    }
    return dialog;
}

// dialog

void SSD1306AsciiDialog::internalRender(int currentValue) {
    SSD1306AsciiRenderer* lcdRender = ((SSD1306AsciiRenderer*)MenuRenderer::getInstance());
    SSD1306Ascii* display = lcdRender->getDisplay();

    if(needsDrawing == MENUDRAW_COMPLETE_REDRAW) {
        display->clear();
    }

    char data[20];
    strncpy_P(data, headerPgm, sizeof(data));
    data[sizeof(data)-1]=0;
    display->setFont(lcdRender->getTitleFont());
    display->setCursor(0,0);
    display->print(data);
    display->setCursor(0, display->fontRows());

    display->setFont(lcdRender->getItemFont());
        display->print(lcdRender->getBuffer());

    if(button1 != BTNTYPE_NONE) {
        copyButtonText(data, 0, currentValue);
        display->setCursor(0, display->displayRows() - display->fontRows());
        display->setInvertMode(currentValue == 0);
        display->print(data);
        display->setInvertMode(false);
    }
    if(button2 != BTNTYPE_NONE) {
        copyButtonText(data, 1, currentValue);
        display->setInvertMode(currentValue == 1);
        int startX = int(lcdRender->getBufferSize() - strlen(data)) * display->fontWidth();
        display->setCursor(startX, display->displayRows() - display->fontRows());
        display->print(data);
        display->setInvertMode(false);
    }
}
