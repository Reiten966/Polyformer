/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file BaseGraphicalRenderer.h
 * Contains the base functionality for all graphical renderers.
 */

#ifndef TCMENU_BASEGRAPHICALRENDERER_H
#define TCMENU_BASEGRAPHICALRENDERER_H

#include <BaseRenderers.h>
#include "GfxMenuConfig.h"
#include "RuntimeTitleMenuItem.h"

namespace tcgfx {

    /**
     * An internal method used to calculate the row*col index that is used during rendering to locate items quickly
     * @param row the row
     * @param col the column
     * @return a key based on row and column.
     */
    inline uint16_t rowCol(int row, int col) {
        return (row * 100) + col;
    }

    /**
     * Represents a grid position along with the menuID and also the drawable icon, if one exists. It is stored in the list
     * of drawing instructions in the base graphical renderer, and then read when a new menu is displayed in order to reorder
     * and position the items before rendering, these instructions are quite display neutral. It is able to be stored within
     * the simple BtreeList because it implements getKey returning uint16_t
     */
    class GridPositionRowCacheEntry {
    private:
        MenuItem *menuItem;
        GridPosition thePosition;
        ItemDisplayProperties *properties;
    public:
        GridPositionRowCacheEntry() : menuItem(nullptr), thePosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0), properties(nullptr) {}

        GridPositionRowCacheEntry(const GridPositionRowCacheEntry &other) = default;
        GridPositionRowCacheEntry& operator=(const GridPositionRowCacheEntry &other) = default;

        GridPositionRowCacheEntry(MenuItem *item, const GridPosition &pos, ItemDisplayProperties *props) : menuItem(item), thePosition(pos), properties(props) {}

        const GridPosition &getPosition() { return thePosition; }

        ItemDisplayProperties *getDisplayProperties() { return properties; }

        uint16_t getKey() const { return rowCol(thePosition.getRow(), thePosition.getGridPosition()); }

        uint16_t getHeight() {
            return thePosition.getGridHeight() != 0 ? thePosition.getGridHeight() : properties->getRequiredHeight();
        }

        MenuItem *getMenuItem() { return menuItem; }
    };

    /**
     * This is the base class for all simpler renderer classes where the height of a row is equal for all entries,
     * and there is always exactly one item on a row. This takes away much of the work to row allocation for simple
     * renderers. Examples of this are the LiquidCrystal renderer
     */
    class BaseGraphicalRenderer : public BaseMenuRenderer {
    public:
        /**
         * This provides the mode in which the renderer will draw the title, either not at all, always, or as the first row.
         */
        enum TitleMode : uint8_t {
            /** never draw the title */
            NO_TITLE,
            /** the title will only appear when row 0 is selected */
            TITLE_FIRST_ROW,
            /** the title will always be shown regardless of index position */
            TITLE_ALWAYS
        };
        /**
         * Represents the possible drawing commands that the base renderer sends to the leaf class
         */
        enum RenderDrawingCommand {
            /** this indicates the screen should be cleared */
            DRAW_COMMAND_CLEAR,
            /** this indicates that the drawing is about to start */
            DRAW_COMMAND_START,
            /** this indicates that the drawing is ending */
            DRAW_COMMAND_ENDED
        };
    private:
        BtreeList<uint16_t, GridPositionRowCacheEntry> itemOrderByRow;
        MenuItem *currentRootMenu;
        const char *pgmTitle;
        bool lastRowExactFit;
        bool useSliderForAnalog;
        GridPositionRowCacheEntry cachedEntryItem;
    protected:
        TitleMode titleMode = TITLE_FIRST_ROW;
        bool titleOnDisplay = false;
        bool hasTouchScreen;
        uint16_t width, height;
    public:
        BaseGraphicalRenderer(int bufferSize, int wid, int hei, bool lastRowExact, const char *appTitle)
                : BaseMenuRenderer(bufferSize, RENDER_TYPE_CONFIGURABLE) {
            width = wid;
            height = hei;
            titleOnDisplay = true;
            currentRootMenu = nullptr;
            lastRowExactFit = lastRowExact;
            pgmTitle = appTitle;
            useSliderForAnalog = true;
            hasTouchScreen = false;
        }

        void setTitleMode(TitleMode mode) {
            titleMode = mode;
            displayPropertiesHaveChanged();
        }

        void setUseSliderForAnalog(bool useSlider) {
            useSliderForAnalog = useSlider;
        }

        void setHasTouchInterface(bool hasTouch) { hasTouchScreen = hasTouch; }

        void render() override;

        /**
         * Usually called during the initialisation of the display internally to set the width and height.
         * @param w display width in current rotation
         * @param h display height in current rotation
         */
        void setDisplayDimensions(int w, int h) {
            serlogF3(SER_TCMENU_INFO, "Set dimensions: ", w, h);
            width = w;
            height = h;
        }

        /**
         * Draw a widget into the title area at the position indicated, the background will need to be cleared before
         * performing the operation.
         * @param where the position on the screen to draw the widget
         * @param widget the widget to be drawn.
         * @param colorFg the foreground color
         * @param colorBg the background color
         */
        virtual void drawWidget(Coord where, TitleWidget *widget, color_t colorFg, color_t colorBg) = 0;

        /**
         * Draw a menu item onto the display using the instructions
         * @param theItem the item to be rendered
         * @param mode the suggested mode in which to draw
         * @param where the position on the display to render at
         * @param areaSize the size of the area where it should be rendered
         */
        virtual void drawMenuItem(GridPositionRowCacheEntry *entry, Coord where, Coord areaSize, bool drawAll) = 0;

        /**
         * This sends general purpose commands that can be implemened by the leaf class as needed.
         */
        virtual void drawingCommand(RenderDrawingCommand command) = 0;

        /**
         * This indicates to the renderer leaf class that the background color should be filled from
         * Y end point to the end of the screen.
         * @param endPoint the last drawing point in the Y location
         */
        virtual void fillWithBackgroundTo(int endPoint) = 0;

        /**
         * Gets the item display factory that provides the formatting information for this renderer, it holds the font,
         * color, padding and grid information. For all bitmapped renderers (EG: AdaFruit_GFX, U8G2) you can safely up
         * cast to the ConfigurableItemDisplayPropertiesFactory
         * @return the display properties factory.
         */
        virtual ItemDisplayPropertiesFactory &getDisplayPropertiesFactory() = 0;


        /**
         * Find the active item in the current list that is being presented, defaults to item 0.
         * @return the active item
         */
        int findActiveItem(MenuItem* root) override;

        /**
         * @return the total number of items in the current menu
         */
        uint8_t itemCount(MenuItem* root, bool) override;

        /**
         * Provides the menu item grid position and dimensions of it, given a screen position. Usually used by touch screen
         * implementations. Note that this will return nullptr if no entry is found.
         * @param screenPos the raw screen position
         * @param localStart the local menu item start position
         * @param localSize the local menu item size
         * @return the grid cache entry or nullptr if no item was found.
         */
        GridPositionRowCacheEntry* findMenuEntryAndDimensions(const Coord &screenPos, Coord &localStart, Coord &localSize);

        /**
         * Gets the menu item at a given index, which may be different to the order in the tree.
         */
        MenuItem *getMenuItemAtIndex(MenuItem* currentRoot, uint8_t idx) override;

        /**
         * All base graphical renderers use a dialog based on menu item, this provides the greatest flexibility and
         * also reduces the amount of dialog code. This means nearly every display we support uses menu based dialogs
         * @return a shared menu based dialog instance
         */
        BaseDialog *getDialog() override;

        /**
         * @return width of the display in current rotation
         */
        int getWidth() const { return width; }

        /**
         * @return height of the display in current rotation
         */
        int getHeight() const { return height; }

        /**
         * Force the renderer to completely recalculate the display parameters next time it's drawn.
         */
        void displayPropertiesHaveChanged() { currentRootMenu = nullptr; }

    private:
        void checkIfRootHasChanged();

        bool drawTheMenuItems(int startRow, int startY, bool drawEveryLine);

        void renderList();

        void recalculateDisplayOrder(MenuItem *pItem, bool safeMode);

        void redrawAllWidgets(bool forceRedraw);

        int heightOfRow(int row, bool includeSpace=false);

        bool areRowsOutOfOrder();

        int calculateHeightTo(int index, MenuItem *pItem);

        ItemDisplayProperties::ComponentType toComponentType(GridPosition::GridDrawingMode mode, MenuItem *pMenuItem);
    };

    /**
     * This is a helper function for analog items that converts the range of an analog item over the width available
     * in a menu item. It's mainly used by scrolling components.
     * @param item the item to determine the current and max value from
     * @param screenWidth the width available for the full range
     * @return the point on the screen which represents current value
     */
    inline int analogRangeToScreen(AnalogMenuItem *item, int screenWidth) {
        float ratio = (float) screenWidth / (float) item->getMaximumValue();
        return int((float) item->getCurrentValue() * ratio);
    }

    /**
     * This method takes an existing graphics configuration and converts it into the new display properties format, its
     * designer as a bridge between the old config object method and the new more supportable properties definitions.
     * @param factory the properties factory that we wish to populate
     * @param gfxConfig the graphics configuration to convert
     * @param titleHeight the height of the title
     * @param itemHeight the height of a standard item
     */
    void preparePropertiesFromConfig(ConfigurableItemDisplayPropertiesFactory &factory,
                                     const ColorGfxMenuConfig<const void *> *gfxConfig, int titleHeight,
                                     int itemHeight);
} // namespace tcgfx

#endif //TCMENU_BASEGRAPHICALRENDERER_H
