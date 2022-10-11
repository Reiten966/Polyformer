/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file ScrollChoiceMenuItem.h contains the menu item definition for scrolling choice types, and also for RGB items
 */

#ifndef SCROLL_CHOICE_ENUM_MENU_ITEM_H
#define SCROLL_CHOICE_ENUM_MENU_ITEM_H

#include <PlatformDetermination.h>
#include "RuntimeMenuItem.h"
#include <EepromAbstraction.h>

/** the render function for enum items */
int enumItemRenderFn(RuntimeMenuItem *item, uint8_t row, RenderFnMode mode, char *buffer, int bufferSize);

/**
 * An item that can represent a series of values that is too large, changes at runtime, or otherwise cannot be described
 * by an EnumMenuItem. Imagine for example a DAB radio preset selector or the tracks on a CD, or a range of values that
 * are taken from EEPROM.
 *
 * There are three default ways in which you can use it, and these are described breifly below. In any of the cases,
 * this menu item is completely dynamic, and the number of items and values can change at runtime. The modes of operation
 * are as follows:
 *
 * * You provide an eeprom implementation, starting locating, item size, and count. No need for custom callback.
 * * You provide a memory address containing items, the size of each item and count. No need for custom callback
 * * You manually manage the items that are represented by providing a custom runtime item callback
 */
class ScrollChoiceMenuItem : public RuntimeMenuItem {
public:
    enum EnumMemMode : uint8_t {
        MEMORY_ONLY, EEPROM_BASED, CUSTOM
    };
private:
    EepromPosition eepromStart;
    const char *rangeValues;
    uint8_t itemSize;
    uint8_t lastCacheSize;
    EnumMemMode memMode;
public:
    /**
     * Create an enum menu item that has values directly in RAM. The items are stored in ram in an equally spaced array.
     * In this case you are responsible for the memory that makes up the array.
     * @param id the ID of the item
     * @param renderFn the function that will do the rendering, enumItemRenderFn is the default
     * @param currentSel the currently selected item
     * @param enumItemsInRam the array of items, where each item is of itemSize length
     * @param itemSize the size of each item
     * @param numberOfItems the number of items
     * @param next optional pointer to next item
     */
    ScrollChoiceMenuItem(int id, RuntimeRenderingFn renderFn, uint8_t currentSel, const char *enumItemsInRam,
                         int itemSize, int numberOfItems, MenuItem *next = nullptr);

    /**
     * Create a choice menu item that scrolls through available values. Use this constructor to store the values in
     * EEPROM stoage. This uses a flat array where each item is itemSize. So item 0 would be at eemproStart and item
     * 1 would be at eepromStart + itemSize.. Further, it can be cached into memory using cacheEepromValues(), calling
     * this again, will refresh the cache. Before using this you must call menuMgr.setEepromRef(..) to initialise the
     * EEPROM object that should be used.
     *
     * @param id the ID of the item
     * @param renderFn the function that will do the rendering, enumItemRenderFn is the default
     * @param currentSel
     * @param eepromStart the start location in eeprom of the array
     * @param itemSize the size of each item
     * @param numberOfItems the number of items to start with
     * @param next optional pointer to next item
     */
    ScrollChoiceMenuItem(uint16_t id, RuntimeRenderingFn renderFn, uint8_t currentSel, EepromPosition eepromStart,
                         int itemSize, int numberOfItems, MenuItem *next = nullptr);

    /**
     * Create a choice menu item that scrolls through available values. Use this constructor to provide the values
     * manually using a runtime menu callback, this is the most flexible method but requires extra coding.
     *
     * @param id the ID of the item
     * @param renderFn the function that will do the rendering, enumItemRenderFn is the default
     * @param currentSel the currently selected choice
     * @param numberOfItems the number of choices
     * @param next optional pointer to next item
     */
    ScrollChoiceMenuItem(uint16_t id, RuntimeRenderingFn renderFn, uint8_t currentSel, int numberOfItems, MenuItem *next = nullptr);

    /**
     * For EEPROM based items you can choose to cache the values in RAM, the total size should not exceed 256 bytes.
     */
    void cacheEepromValues();

    /**
     * Get the string value at a given index
     * @param buffer the buffer area
     * @param bufferSize size of the buffer
     * @param idx the index to copy
     */
    void valueAtPosition(char *buffer, size_t bufferSize, int idx);

    /**
     * Set the current choice to the new value
     * @param val the new value
     * @param silent if you do not want the callbacks to trigger.
     */
    void setCurrentValue(int val, bool silent = false);

    int getEepromStart() const { return eepromStart; }

    int getItemWidth() const { return itemSize; }

    EnumMemMode getMemMode() const { return memMode; }

    /**
     * @return integer of the current choice
     */
    int getCurrentValue() { return itemPosition; }

    void copyTransportText(char *string, size_t i);

    void setFromRemote(const char* buffer);
};

/**
 * Helper function to get the hex character for a digit.
 * @param val the input between 0..15
 * @return the hex digit for the value
 */
uint8_t hexValueOf(char val);

/**
 * This structure represents an RGBA value, using 8 bits per element. It uses the RGB color space and is capable
 * of constructing from the raw integer values or a HTML color code. It can also output it's current state to a
 * HTML color code.
 */
struct RgbColor32 {
public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

    RgbColor32() {
        red = green = blue = alpha = 0;
    }

    RgbColor32(int r, int g, int b, int a = 255) {
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }

    RgbColor32(const RgbColor32& other) {
        red = other.red;
        green = other.green;
        blue = other.blue;
        alpha = other.alpha;
    }

    RgbColor32& operator = (const RgbColor32& other) = default;

    explicit RgbColor32(const char* htmlColor);

    void asHtmlString(char *buffer, size_t bufferSize, bool withAlpha) const;
};

/** The rendering callback function for RGB values */
int rgbAlphaItemRenderFn(RuntimeMenuItem *item, uint8_t row, RenderFnMode mode, char *buffer, int bufferSize);

/**
 * A Menu item that can display and edit RGB values that are 32 bits wide, that is 8 bit per element and
 * an optional alpha channel. This is based on editable runtime menu item.
 */
class Rgb32MenuItem : public EditableMultiPartMenuItem {
private:
    RgbColor32 data;
    bool alphaChannel;
public:

    /**
     * Creates a color data menu item that can be edited, optionally including an alpha channel
     * @param id the id of the item
     * @param renderFn the rendering function to use - see default in the class definition
     * @param includeAlpha true to include alpha channel, otherwise false.
     * @param next optional pointer to the next menu item
     */
    Rgb32MenuItem(uint16_t id, RuntimeRenderingFn renderFn, bool includeAlpha, MenuItem* next = nullptr);

    /**
     * @return the underlying color data that can be directly modified.
     */
    RgbColor32* getUnderlying() { return &data; }

    /**
     * @return a copy of the underlying color data
     */
    RgbColor32 getColorData() { return data; }
    /**
     * copy the color data provided as the latest
     * @param other the new color data
     */
    void setColorData(const RgbColor32& other) {
        data = other;
        changeOccurred(false);
    }

    /**
     * @return true if the alpha channel is in use for this menuitem, otherwise false.
     */
    bool isAlphaInUse() const { return alphaChannel; }
};

#endif //SCROLL_CHOICE_ENUM_MENU_ITEM_H
