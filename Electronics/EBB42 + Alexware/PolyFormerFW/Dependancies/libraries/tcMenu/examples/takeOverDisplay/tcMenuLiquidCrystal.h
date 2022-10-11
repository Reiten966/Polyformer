/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file tcMenuLiquidCrystal.h
 * 
 * LiquidCrystalIO renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This renderer is designed for use with this library: https://github.com/davetcc/LiquidCrystalIO
 */

#ifndef _TCMENU_LIQUID_CRYSTAL_H
#define _TCMENU_LIQUID_CRYSTAL_H

#include "tcMenu.h"
#include <LiquidCrystalIO.h>
#include <BaseDialog.h>
#include <graphics/BaseGraphicalRenderer.h>

using namespace tcgfx;

/**
 * A renderer that can renderer onto a LiquidCrystal display and supports the concept of single level
 * sub menus, active items and editing.
 */
class LiquidCrystalRenderer : public BaseGraphicalRenderer {
private:
    LiquidCrystal* lcd;
    NullItemDisplayPropertiesFactory propertiesFactory;
    char backChar;
    char forwardChar;
    char editChar;
public:
    LiquidCrystalRenderer(LiquidCrystal& lcd, int dimX, int dimY);
    ~LiquidCrystalRenderer() override;
    void initialise() override;
    void setTitleRequired(bool titleRequired) { titleMode = (titleRequired) ? TITLE_FIRST_ROW : NO_TITLE; }
    void setEditorChars(char back, char forward, char edit);

    uint8_t getRows() {return height;}
    LiquidCrystal* getLCD() {return lcd;}
    BaseDialog* getDialog() override;

    void drawingCommand(RenderDrawingCommand command) override;
    void drawWidget(Coord where, TitleWidget* widget, color_t colorFg, color_t colorBg) override;
    void drawMenuItem(GridPositionRowCacheEntry* entry, Coord where, Coord areaSize, bool drawAll) override;
    void fillWithBackgroundTo(int endPoint) override;

    ItemDisplayPropertiesFactory &getDisplayPropertiesFactory() override { return propertiesFactory; }
    NullItemDisplayPropertiesFactory &getLcdDisplayPropertiesFactory() { return propertiesFactory; }
};

#endif // _TCMENU_LIQUID_CRYSTAL_H
