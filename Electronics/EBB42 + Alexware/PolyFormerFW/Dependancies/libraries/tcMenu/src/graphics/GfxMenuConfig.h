/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file GfxMenuConfig.h
 *
 * This file contains the base drawing configuration structures and helper methods for
 * drawing onto graphical screens, be it mono or colour. Also there's some additional
 * structures for describing colours, coordinates and padding.
 */

#ifndef _GFX_MENU_CONFIG_H_
#define _GFX_MENU_CONFIG_H_

#include <tcUtil.h>
#include <SimpleCollections.h>
#include "MenuItems.h"
#include "DrawingPrimitives.h"
#include <IoLogging.h>


namespace tcgfx {

#define SPECIAL_ID_EDIT_ICON 0xfffe
#define SPECIAL_ID_ACTIVE_ICON 0xfffd

    /**
     * Holds the graphical configuration of how to render a menu onto a both mono and colour displays. If you don't intend
     * to override this initially just call the factory method provided with your renderer.
     */
    template<typename FONTPTR> struct ColorGfxMenuConfig {
        color_t bgTitleColor;
        color_t fgTitleColor;
        MenuPadding titlePadding;
        FONTPTR titleFont;

        color_t bgItemColor;
        color_t fgItemColor;
        MenuPadding itemPadding;
        FONTPTR itemFont;

        color_t bgSelectColor;
        color_t fgSelectColor;
        color_t widgetColor;
        MenuPadding widgetPadding;

        const uint8_t* activeIcon;
        const uint8_t* editIcon;
        uint8_t editIconWidth;
        uint8_t editIconHeight;

        uint8_t titleBottomMargin;
        uint8_t titleFontMagnification;
        uint8_t itemFontMagnification;
    };

    /**
     * @deprecated do not use in new designs use drawing properties instead, may be removed in a future release
     * @param config the config to be filled in.
     */
    void prepareDefaultGfxConfig(ColorGfxMenuConfig<void*>* config);


    /**
    * Provides a platform independent means of identifying where on the screen a particular menu item resides using a
    * simple grid layout
    */
    class GridPosition {
    public:
        /**
         * Represents how the item in this position should be drawn.
         */
        enum GridDrawingMode : uint8_t {
            /** Drawn as text in the form of Item on the left, value on the right */
            DRAW_TEXTUAL_ITEM,
            /** Drawn two buttons with the value in the middle, to move through a range of values */
            DRAW_INTEGER_AS_UP_DOWN,
            /** Drawn as a scroll slider, to move through a series of values */
            DRAW_INTEGER_AS_SCROLL,
            /** Drawn as an icon with no text underneath */
            DRAW_AS_ICON_ONLY,
            /** Drawn as an icon with text underneath  */
            DRAW_AS_ICON_TEXT,
            /** Drawn as a title, usually reserved for the title at the top of the page */
            DRAW_TITLE_ITEM
        };

        /**
         * Represents the justification of both the item name and the value within the items drawing space. This controls exactly
         * what text is presented, and the position too.
         */
        enum GridJustification : uint8_t {
            /** Indicates left justification - usually use of the combined types below */
            CORE_JUSTIFY_LEFT = 1,
            /** Indicates right justification - usually use of the combined types below */
            CORE_JUSTIFY_RIGHT = 2,
            /** Indicates centre justification - usually use of the combined types below */
            CORE_JUSTIFY_CENTER = 3,

            /** usually use of the combined types below */
            CORE_JUSTIFY_VALUE_REQUIRED = 0b1000,
            /** usually use of the combined types below */
            CORE_JUSTIFY_NAME_REQUIRED  = 0b0100,

            /** Justify the item name on the left and the value on the right */
            JUSTIFY_TITLE_LEFT_VALUE_RIGHT = 0,
            /** Justify the item name and value on the right */
            JUSTIFY_TITLE_LEFT_WITH_VALUE = CORE_JUSTIFY_LEFT + CORE_JUSTIFY_NAME_REQUIRED + CORE_JUSTIFY_VALUE_REQUIRED,
            /** Justify the item name and value centered */
            JUSTIFY_CENTER_WITH_VALUE = CORE_JUSTIFY_CENTER + CORE_JUSTIFY_NAME_REQUIRED + CORE_JUSTIFY_VALUE_REQUIRED,
            /** Justify the item name and value on the right */
            JUSTIFY_RIGHT_WITH_VALUE = CORE_JUSTIFY_RIGHT + CORE_JUSTIFY_NAME_REQUIRED + CORE_JUSTIFY_VALUE_REQUIRED,
            /** Justify just the item name to the left */
            JUSTIFY_LEFT_NO_VALUE = CORE_JUSTIFY_LEFT + CORE_JUSTIFY_NAME_REQUIRED,
            /** Justify just the item name in the center */
            JUSTIFY_CENTER_NO_VALUE = CORE_JUSTIFY_CENTER + CORE_JUSTIFY_NAME_REQUIRED,
            /** Justify just the item name in the center */
            JUSTIFY_RIGHT_NO_VALUE = CORE_JUSTIFY_RIGHT + CORE_JUSTIFY_NAME_REQUIRED,
            /** Justify just the items current value to the left */
            JUSTIFY_LEFT_VALUE_ONLY= CORE_JUSTIFY_LEFT + CORE_JUSTIFY_VALUE_REQUIRED,
            /** Justify just the items current value in the center */
            JUSTIFY_CENTER_VALUE_ONLY = CORE_JUSTIFY_CENTER + CORE_JUSTIFY_VALUE_REQUIRED,
            /** Justify just the items current value to the right */
            JUSTIFY_RIGHT_VALUE_ONLY = CORE_JUSTIFY_RIGHT + CORE_JUSTIFY_VALUE_REQUIRED
        };
    private:
        /** the number of columns in the grid */
        uint32_t gridSize: 3;
        /** the position in the columns */
        uint32_t gridPosition: 3;
        /** not yet implemented in any renderers, the number of rows this item takes up. */
        uint32_t gridRowSpan: 2;
        /** the height or 0 if not overridden. Between 0..255*/
        uint32_t gridHeight: 9;
        /** the row ordering for the item */
        uint32_t rowPosition: 7;
        /** the drawing mode that should be used */
        uint32_t drawingMode: 4;
        /** the text justification that should be used */
        uint32_t justification: 4;
    public:
        GridPosition() : gridSize(0), gridPosition(0), gridHeight(0), rowPosition(0), drawingMode(0),
                         justification(JUSTIFY_TITLE_LEFT_VALUE_RIGHT) {}

        GridPosition(const GridPosition &other) = default;
        GridPosition& operator=(const GridPosition &other) = default;

        /**
         * Create a simple grid position that represents a row with a single column with optional override of the row height
         * @param mode the mode in which to draw the item
         * @param justification the desired justification for this item
         * @param row the row on which to draw the item
         * @param height the height of the item or leave blank for default
         */
        GridPosition(GridDrawingMode mode, GridJustification justification, int row, int height = 0)
                : gridSize(1), gridPosition(1), gridHeight(height),rowPosition(row), drawingMode(mode), justification(justification) { }

                /**
                 * Create a more complex multi column grid with height, this represents a single row with one or more columns,
                 * a position in the columns, and if need be, a height override.
                 * @param mode the mode in which to draw the item
                 * @param justification the desired justification for this item
                 * @param size the number of columns in the row
                 * @param pos the column position in the row
                 * @param row the row on which to draw the item
                 * @param hei the height of the row, or 0 for the default height.
                 */
        GridPosition(GridDrawingMode mode, GridJustification just, int size, int pos, int row, int hei)
                : gridSize(size), gridPosition(pos), gridHeight(hei), rowPosition(row), drawingMode(mode), justification(just) { }

        GridDrawingMode getDrawingMode() const { return static_cast<GridDrawingMode>(drawingMode); }

        GridJustification getJustification() const { return static_cast<GridJustification>(justification); }

        int getGridSize() const { return gridSize; }

        int getGridHeight() const { return gridHeight; }

        int getGridPosition() const { return gridPosition; }

        int getRow() const { return rowPosition; }
    };

    /**
     * A helper function that checks if a particular justification includes the value
     * @param justification the justification to check
     * @return true if the value is needed
     */
    inline bool itemNeedsValue(GridPosition::GridJustification justification) {
        return (justification & GridPosition::CORE_JUSTIFY_VALUE_REQUIRED) != 0;
    }

    /**
     * A helper function that checks if a particular justification includes the name
     * @param justification the justification to check
     * @return true if the name is needed
     */
    inline bool itemNeedsName(GridPosition::GridJustification justification) {
        return (justification & GridPosition::CORE_JUSTIFY_NAME_REQUIRED) != 0;
    }

    /**
     * Get the core justification part, eg left, right centre
     * @param j the justification to check
     * @return the core part, just left, right centre etc.
     */
    inline GridPosition::GridJustification coreJustification(GridPosition::GridJustification j) {
        return static_cast<GridPosition::GridJustification>(j & 0b11);
    }

    /**
     * Represents a grid position along with the menuID and also the drawable icon, if one exists. It is stored in the list
     * of drawing instructions in the base graphical renderer, and then read when a new menu is displayed in order to reorder
     * and position the items before rendering, these instructions are quite display neutral. It is able to be stored within
     * the simple BtreeList because it implements getKey returning uint16_t
     */
    class GridPositionWithId {
    private:
        uint16_t menuId;
        GridPosition thePosition;
    public:
        GridPositionWithId() : menuId(0xffff), thePosition(GridPosition::DRAW_TEXTUAL_ITEM,
                                                           GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0) {}

        GridPositionWithId(const GridPositionWithId &other) = default;
        GridPositionWithId& operator=(const GridPositionWithId &other) = default;

        GridPositionWithId(uint16_t itemId, const GridPosition &pos) : menuId(itemId), thePosition(pos) {}

        const GridPosition &getPosition() { return thePosition; }

        uint16_t getKey() const { return menuId; }

        void setNewPosition(GridPosition newPosition) {
            thePosition = newPosition;
        }
    };

    /**
     * Represents the display properties for a menu item, submenu or default, it stores this the properties key which
     * can key at each of those levels. It stores the color palette, padding, row spacing, font information and default
     * justification for rendering.
     */
    class ItemDisplayProperties {
    public:
        /**
         * The sub-component that is being drawn that we need the formatting rules for.
         */
        enum ColorType: uint8_t {
            TEXT,
            BACKGROUND,
            HIGHLIGHT1,
            HIGHLIGHT2,
            SIZEOF_COLOR_ARRAY
        };
        /**
         * The overall component type being rendered
         */
        enum ComponentType {
            COMPTYPE_TITLE,
            COMPTYPE_ITEM,
            COMPTYPE_ACTION,
        };
    private:
        uint32_t propsKey;
        color_t colors[SIZEOF_COLOR_ARRAY];
        MenuPadding padding;
        const void* fontData;
        MenuBorder borderWidths;
        uint8_t fontMagnification: 4;
        uint8_t defaultJustification: 4;
        uint8_t spaceAfter;
        uint8_t requiredHeight;
    public:
        ItemDisplayProperties() : propsKey(0), colors{}, padding(), fontData(nullptr), borderWidths(), fontMagnification(1), defaultJustification(0), spaceAfter(0), requiredHeight(0) {}
        ItemDisplayProperties(uint32_t key, const color_t* palette, const MenuPadding& pad, const void* font, uint8_t mag, uint8_t spacing,
                              uint8_t height, GridPosition::GridJustification defaultJustification, MenuBorder borderWidths)
                              : propsKey(key), padding{pad}, fontData(font), borderWidths(borderWidths), fontMagnification(mag), defaultJustification(defaultJustification),
                                spaceAfter(spacing), requiredHeight(height) {
            memcpy(colors, palette, sizeof colors);
        }
        ItemDisplayProperties(const ItemDisplayProperties& other) : propsKey(other.propsKey), padding{other.padding}, fontData(other.fontData),
                                borderWidths(other.borderWidths), fontMagnification(other.fontMagnification), defaultJustification(other.defaultJustification),
                                spaceAfter(other.spaceAfter), requiredHeight(other.requiredHeight) {
            memcpy(colors, other.colors, sizeof colors);
        }
        ItemDisplayProperties& operator=(const ItemDisplayProperties& other) {
            if(&other == this) return *this;
            propsKey = other.propsKey;
            padding = other.padding;
            fontData = other.fontData;
            borderWidths = other.borderWidths;
            fontMagnification = other.fontMagnification;
            defaultJustification = other.defaultJustification;
            spaceAfter = other.spaceAfter;
            requiredHeight = other.requiredHeight;
            memcpy(colors, other.colors, sizeof colors);
            return *this;
        }

        uint32_t getKey() const { return propsKey; }

        GridPosition::GridJustification getDefaultJustification() const { return (GridPosition::GridJustification)defaultJustification; }

        void setDefaultJustification(GridPosition::GridJustification justification) { defaultJustification = justification; }

        uint8_t getSpaceAfter() const {return spaceAfter; }

        void setSpaceAfter(uint8_t space) { spaceAfter = space; }

        uint8_t getRequiredHeight() const { return requiredHeight; }

        void setRequiredHeight(uint8_t newHeight) { requiredHeight = newHeight; }

        color_t getColor(ColorType color) const {
            return (color < SIZEOF_COLOR_ARRAY) ? colors[color] : RGB(0,0,0);
        }

        void setColor(ColorType color, color_t value) {
            if(color >= SIZEOF_COLOR_ARRAY) return;
            colors[color] = value;
        }

        void setColors(const color_t* palette) {
            memcpy(colors, palette, sizeof colors);
        }

        const MenuPadding& getPadding() const {
            return padding;
        }

        void setPadding(const MenuPadding& pad) {
            padding = pad;
        }

        MenuBorder getBorder() const {
            return borderWidths;
        }

        void setBorder(MenuBorder border) {
            borderWidths = border;
        }

        uint8_t getFontMagnification() const {
            return fontMagnification;
        }

        void setFontInfo(const void* font, uint8_t mag) {
            fontMagnification = mag;
            fontData = font;
        }

        const void* getFont() {
            return fontData;
        }

        color_t* getPalette() {
            return colors;
        }
    };

    /**
     * This factory is responsible for generating all the display configuration settings for the main display, it provides
     * all the grid setting overrides, icons, fonts, padding and colors. This class also provides sensible defaults for
     * when no overrides are present. For GridSettings, it returns null if there is no override, for colors, fonts and padding
     * it checks first by ID, then gets the default if no override exists.
     * be easy to extend by the end user, in order to add additional drawing rules easily. This class will slowly replace
     * the current GfxConfig objects which are quite inflexible.
     */
    class ItemDisplayPropertiesFactory {
    public:
        /**
         * Returns the configuration for the parameters below, it should never return nullptr.
         * @param pItem the item or null for default
         * @param compType the component type to get the rendering for
         * @return the properties for a given component.
         */
        virtual ItemDisplayProperties* configFor(MenuItem* pItem, ItemDisplayProperties::ComponentType compType) = 0;

        /**
         * Returns the icon associated with the menu item ID, there are two special IDs for the edit and active icons
         * @param id the menu item ID or the special ID for edit or active icon
         * @return the icon or nullptr if not available
         */
        virtual DrawableIcon* iconForMenuItem(uint16_t id) = 0;

        /**
         * Get the grid item for a given position if it is available
         * @param pItem the item to get the grid position for
         * @return the grid position if available
         */
        virtual GridPositionWithId* gridPositionForItem(MenuItem* pItem) = 0;

        /**
         * Get the selected color for a given palette entry
         * @param colorType
         * @return
         */
        virtual color_t getSelectedColor(ItemDisplayProperties::ColorType colorType, bool isUnderCursor = false)  = 0;

        /**
         * add a new grid position for a given menu item
         * @param item the menu item the position is for
         * @param position the position to record
         */
        virtual void addGridPosition(MenuItem* item, const GridPosition& position)  = 0;
    };

    /**
     * Used by the BaseGraphicalRenderer class to work out how to draw an item, which grid position it is in etc. This
     * version does not implement icons, nor does it implement overriding of display properties, it is intended for LCDs.
     *
     * This is used by non graphical displays to allow grid configurations while not providing the complex support for
     * display properties at different levels. This makes the class much simpler and should use far less memory on devices.
     */
    class NullItemDisplayPropertiesFactory : public ItemDisplayPropertiesFactory {
    private:
        color_t anEmptyPalette[4] = {0};
        ItemDisplayProperties props = ItemDisplayProperties(0, anEmptyPalette, MenuPadding(0), nullptr, 1, 0, 1, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder());
        BtreeList<uint16_t, GridPositionWithId> gridByItem;
    public:
        NullItemDisplayPropertiesFactory() : gridByItem(4) {}

        ItemDisplayProperties* configFor(MenuItem* pItem, ItemDisplayProperties::ComponentType compType) override {
            return &props;
        }

        DrawableIcon* iconForMenuItem(uint16_t id) override{
            return nullptr;
        }

        color_t getSelectedColor(ItemDisplayProperties::ColorType colorType, bool isUnderCursor = false) override {
            return 0;
        }

        GridPositionWithId* gridPositionForItem(MenuItem* pItem) override {
            if(!pItem) return nullptr;
            return gridByItem.getByKey(pItem->getId());
        }

        void addGridPosition(MenuItem* pItem, const GridPosition& position) override {
            if(!pItem) return;
            auto* grid = gridByItem.getByKey(pItem->getId());
            if(grid) {
                grid->setNewPosition(position);
            }
            else {
                serlogF2(SER_TCMENU_DEBUG, "Adding grid ", pItem->getId());
                gridByItem.add(GridPositionWithId(pItem->getId(), position));
            }
        }
    };

#define MENUID_NOTSET 0xffff

    inline uint32_t MakePropsKey(uint16_t menuId, bool parentKey, ItemDisplayProperties::ComponentType ty) {
        return (uint32_t)menuId | (parentKey ? 0x10000UL : 0UL) | ((uint32_t)ty << 18UL);
    }

    /**
     * How the editing cursor should be represented on the display
     */
    enum EditCursorMode {
        /** Show an underline underneath the area being edited, default */
        CURSOR_MODE_UNDERLINE,
        /** Fill the area being edited in a different color */
        CURSOR_MODE_BACKGROUND_BOX
    };

    /**
     * Provides full support for configurability of menu items, in terms of the their grid position and also any associated
     * icons and drawing color / font / padding overrides. Each time the renderer sets up a new menu, it calls into here
     * for each item to find out what settings to use for drawing. It is therefore possible to adjust settings either globally,
     * by sub menu, or for a given menu item. This class also stores icon definitions by menu item, so any items that are
     * set to draw as icons will look in that cache. After updating any drawing values, you should call refreshCache() to
     * ensure it is picked up.
     */
    class ConfigurableItemDisplayPropertiesFactory : public ItemDisplayPropertiesFactory {
    private:
        BtreeList<uint32_t, ItemDisplayProperties> displayProperties;
        BtreeList<uint16_t, DrawableIcon> iconsByItem;
        BtreeList<uint16_t, GridPositionWithId> gridByItem;
        color_t selectedTextColor = RGB(0,0,0);
        color_t selectedBackgroundColor = RGB(0, 100, 255);
        color_t editCursorColor = RGB(255, 255, 255);
        color_t editCursorTextColor = RGB(0, 0, 0);
        EditCursorMode editCursorMode = CURSOR_MODE_UNDERLINE;
    public:
        ConfigurableItemDisplayPropertiesFactory()
                : displayProperties(5, GROW_BY_5),
                  iconsByItem(6, GROW_BY_5) { }

        /**
         * Get the icon associated with a menu ID or nullptr if not available.
         * @param id the menu id
         * @return an icon or nullptr
         */
        DrawableIcon* iconForMenuItem(uint16_t id) override {
            return iconsByItem.getByKey(id);
        }

        /**
         * Gets the grid position override for a given menu item if one is available, or nullptr if not.
         * @param pItem the menu item to get the override for
         * @return the override position or nullptr
         */
        GridPositionWithId* gridPositionForItem(MenuItem* pItem) override {
            if(!pItem) return nullptr;
            return gridByItem.getByKey(pItem->getId());
        }

        /**
         * Get the display configuration for a given item given a component type. This will search for an item at
         * the menu item level first, then at the sub menu item level, and finally it will use the default. It is
         * always safe to call this function, it never returns nullptr.
         * @param pItem the item for which we want properties
         * @param compType the component type we are drawing
         * @return a display properties instance.
         */
        ItemDisplayProperties* configFor(MenuItem* pItem, ItemDisplayProperties::ComponentType compType)  override;

        /**
         * Gets the selected color for a particular color type (such as background, text etc)
         * @param colorType the color type we need
         * @return the actual color
         */
        color_t getSelectedColor(ItemDisplayProperties::ColorType colorType, bool isUnderCursor = false) override {
            if(isUnderCursor) {
                return colorType == ItemDisplayProperties::BACKGROUND ? editCursorColor : editCursorTextColor;
            }
            return colorType == ItemDisplayProperties::BACKGROUND ? selectedBackgroundColor : selectedTextColor;
        }

        /**
         * Adds a new grid position for a particular menu item to the cache of grid positions. Grid positions are
         * stored by menu item and override how and where an item is drawn in the grid.
         * @param pItem the item who's position is being overridden
         * @param position the position override
         */
        void addGridPosition(MenuItem* pItem, const GridPosition& position) override {
            if(!pItem) return;
            auto* grid = gridByItem.getByKey(pItem->getId());
            if(grid) {
                grid->setNewPosition(position);
            }
            else {
                gridByItem.add(GridPositionWithId(pItem->getId(), position));
            }
        }

        /**
         * Adds an icon image to the cache by menu ID. When there is a grid position override that has icon drawing as
         * its rendering type, then the image will be looked up using this cache.
         * @param toAdd the icon image to add.
         */
        void addImageToCache(const DrawableIcon& toAdd) {
            auto* current = iconsByItem.getByKey(toAdd.getKey());
            if(current) {
                current->setFromValues(toAdd.getDimensions(), toAdd.getIconType(), toAdd.getIcon(false), toAdd.getIcon(true));
            }
            else {
                iconsByItem.add(toAdd);
            }
        }

        /**
         * Sets the drawing properties that should be used when no overrides exist at any other level. these are basically the defaults for the component type
         * @param drawing the component type for which these are the defaults
         * @param palette the palette that should be used
         * @param pad the padding that should be used
         * @param font the font that will be used for text rendering
         * @param mag the font number or magnification depending on the drawable used
         * @param spacing the spacing between items
         * @param requiredHeight the required default height for items (can be overridden in position)
         * @param defaultJustification the default justification (can be overridden in position)
         * @param border the border to draw around the edges of the item
         */
        void setDrawingPropertiesDefault(ItemDisplayProperties::ComponentType drawing, const color_t* palette, MenuPadding pad, const void *font, uint8_t mag,
                                         uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification, MenuBorder border) {
            setDrawingProperties(MakePropsKey(MENUID_NOTSET, false, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification, border);
        }

        /**
         * Sets the drawing properties that should be used for a specific item, it must have the right component type.
         * @param drawing the component type for which these are the defaults
         * @param id the menu id for which this override is valid
         * @param palette the palette that should be used
         * @param pad the padding that should be used
         * @param font the font that will be used for text rendering
         * @param mag the font number or magnification depending on the drawable used
         * @param spacing the spacing between items
         * @param requiredHeight the required default height for items (can be overridden in position)
         * @param defaultJustification the default justification (can be overridden in position)
         * @param border the border to draw around the edges of the item
         */
        void setDrawingPropertiesForItem(ItemDisplayProperties::ComponentType drawing, uint16_t id, const color_t* palette, MenuPadding pad, const void *font, uint8_t mag,
                                         uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification, MenuBorder border) {
            setDrawingProperties(MakePropsKey(id, false, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification, border);
        }

        /**
         * Sets the drawing properties that should be used for a specific item, it must have the right component type.
         * @param drawing the component type for which these are the defaults
         * @param id the submenu id for which this override will apply to all children
         * @param palette the palette that should be used
         * @param pad the padding that should be used
         * @param font the font that will be used for text rendering
         * @param mag the font number or magnification depending on the drawable used
         * @param spacing the spacing between items
         * @param requiredHeight the required default height for items (can be overridden in position)
         * @param defaultJustification the default justification (can be overridden in position)
         * @param border the border to draw around the edges of the item
         */
        void setDrawingPropertiesAllInSub(ItemDisplayProperties::ComponentType drawing, uint16_t id, const color_t* palette, MenuPadding pad, const void *font, uint8_t mag,
                                          uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification, MenuBorder border) {
            setDrawingProperties(MakePropsKey(id, true, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification, border);
        }

        /**
         * Set the backround and text color for selected items
         * @param background the color of the background
         * @param text the color of the text
         */
        void setSelectedColors(color_t background, color_t text) {
            selectedBackgroundColor = background;
            selectedTextColor = text;
        }

        /**
         * Sets the edit cursor mode and cursor color for different display types
         * @param mode the type of cursor to present
         * @param cursorColor the color of the cursor itself
         * @param textColor for filled cursors, this is the text color on the top of the cursor
         */
        void setEditCursorMode(EditCursorMode mode, color_t cursorCol, color_t textCol) {
            editCursorMode = mode;
            editCursorColor = cursorCol;
            editCursorTextColor = textCol;
        }

        /**
         * Whenever you've called any method that adjusts the cache by adding new drawing options, you must then call refreshCache to
         * ensure the drawing functions are aware of the change
         */
        static void refreshCache();

    private:
        void setDrawingProperties(uint32_t key, const color_t* palette, MenuPadding pad, const void* font, uint8_t mag, uint8_t spacing,
                                  uint8_t requiredHeight, GridPosition::GridJustification defaultJustification, MenuBorder border);

    };

} // namespace tcgfx

/**
 * The default editing icon for approx 100-150 dpi resolution displays
 */
extern const uint8_t PROGMEM loResEditingIcon[];

/**
 * The default active icon for approx 100-150 dpi resolution displays
 */
extern const uint8_t PROGMEM loResActiveIcon[];

/**
 * The low resolution icon for indicating active status
 */
extern const uint8_t PROGMEM defActiveIcon[];

/**
 * The low resolution icon for editing status
 */
extern const uint8_t PROGMEM defEditingIcon[];

#endif // _GFX_MENU_CONFIG_H_
