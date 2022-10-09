/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file GraphicsDeviceRenderer.h the interface that all graphics devices should implement to do the actual graphics rendering.
 */

#ifndef TCLIBRARYDEV_GRAPHICSDEVICERENDERER_H
#define TCLIBRARYDEV_GRAPHICSDEVICERENDERER_H

#include <PlatformDetermination.h>
#include "../tcMenu.h"
#include "BaseGraphicalRenderer.h"
#include "GfxMenuConfig.h"

#define MINIMUM_CURSOR_SIZE 6

namespace tcgfx {

    /**
     * This is the interface that all graphical rendering devices extend from when using GraphicsDeviceRenderer. Instances
     * of this map all the graphics primitives for each display type. You can use it yourself in code to present moderately
     * complex displays without committing to a particular device. IE it would be trivial to switch between STM32 BCP LCD,
     * Adafruit library, TFTeSPI and U8G2.
     *
     * Example usage (assuming you have a drawable object pointer):
     *
     * ```
     * drawable->startDraw();
     * drawable->setDrawColor(RGB(0,0,0));
     * drawable->drawBox(Coord(0,0), Coord(320, 20), true);
     * drawable->setColors(RGB(255, 128, 0), RGB(0,0,0));
     * drawable->drawText(Coord(0,0), nullptr, 0, "hello world");
     * drawable->setDrawColor(RGB(0,0,255));
     * drawable->drawCircle(Coord(50, 50), 10, true);
     * drawable->endDraw();
     * ```
     *
     * The virtual methods add a small overhead, but allow us to easily support a very wide range of screens, in future
     * supporting new graphical displays will be far easier, and less maintenance going forwards.
     */
    class DeviceDrawable {
    protected:
        color_t backgroundColor = 0, drawColor = 0;
    public:
        virtual ~DeviceDrawable() = default;
        /**
         * @return the dimensions of the screen corrected for the current rotation
         */
        virtual Coord getDisplayDimensions()=0;

        /**
         * Gets a sub device that can be rendered to that buffers a part of the screen up to a particular size, it is
         * for sprites or partial buffers when the display is not fully double buffered. This method returns nullptr when
         * such a sub-device is not available or not needed.
         * @param width the needed width in pixels
         * @param height the needed height in pixels
         * @param bitsPerPx the number of bits per pixel
         * @param palette the palette that will be used
         * @return the sub display, or null if one is not available.
         */
        virtual DeviceDrawable* getSubDeviceFor(const Coord& where, const Coord& size, const color_t *palette, int paletteSize)=0;

        /**
         * Draw text at the location requested using the font and color information provided.
         * @param where the position on the screen
         * @param font the font to use
         * @param mag the magnification of the font - if supported
         * @param text the string to print
         */
        virtual void drawText(const Coord& where, const void* font, int mag, const char* text)=0;

        /**
         * Draw an icon bitmap at the specified location using the provided color, you can choose either the regular or
         * selected icon. Note that not all icon formats are available on every display.
         * @param where the position of the screen
         * @param icon the icon to draw
         * @param bg the background color to draw behind the bitmap
         * @param selected if the regular or selected icon should be used.
         */
        virtual void drawBitmap(const Coord &where, const DrawableIcon *icon, bool selected)=0;

        /**
         * Draws an XBM at the provided location, of the given size, in the colours provided.
         * @param where the position on the screen
         * @param size the size of the image in pixels
         * @param data the raw bitmap data that should be presented
         * @param fgColor the color for the bitmap to use
         * @param bgColor the background color to fill first.
         */
        virtual void drawXBitmap(const Coord& where, const Coord& size, const uint8_t* data)=0;

        /**
         * Draw either a hollow or filled both with the provided dimensions, in the color provided.
         * @param where the position of the box
         * @param size the size of the box
         * @param color the color of the box
         * @param filled if true, the box is filled, otherwise it is an outline only.
         */
        virtual void drawBox(const Coord& where, const Coord& size, bool filled)=0;

        /**
         * Draw a circle that is either filled or not filled on the screen
         * @param where the position of the circle *centre*
         * @param radius the radius of the circle to draw
         * @param filled true if it is to be filled, otherwise false
         */
        virtual void drawCircle(const Coord& where, int radius, bool filled)=0;

        /**
         * Draw a triangle or polygon onto the screen as a series of points, if the driver supports it. All drivers support
         * lines and triangles, but nothing above that is guaranteed.
         *
         * It may be that a triangle can be filled but a polygon cant. If points is two it will try and use draw line and
         * if points is 3 it will try to use draw triangle. There is no guarantee that the underlying driver will support
         * anything above draw triangle, so check your driver before using more than 3 points. TcMenu graphics libraries
         * never use this for anything higher than triangle, as it may not be supported.
         */
        virtual void drawPolygon(const Coord points[], int numPoints, bool filled)=0;

        /**
         * the device must be able to map colors for situations where a palette is used. The default implementation
         * just returns the original color.
         * @param col the color to find the underlying color for
         * @return the actual color mapping for the display.
         */
        virtual color_t getUnderlyingColor(color_t col) { return col; }

        /**
         * Indicates the start or end of a transaction, for core devices, it indicates the end of a rendering loop, for
         * sub devices it means that we have finished with the
         * @param isStarting
         * @param redrawNeeded
         */
        virtual void transaction(bool isStarting, bool redrawNeeded)=0;

        /**
         * gets the extents of the text, and optionally the baseline for the given font and text. The X axis of the returned
         * Coord is the text width, the Y axis has the height, finally the base line is the descent.
         * @param font the font to get sizing for
         * @param mag the magnification of the font if supported
         * @param text the text to get the size of
         * @param baseline optionally, the base line will be populated if this is provided.
         * @return the X and Y dimensions
         */
        virtual Coord textExtents(const void* font, int mag, const char* text, int* baseline)=0;

        /**
         * Gets the width and height of the text in the font provided, returned in a Coord.
         * @param font the font to measure
         * @param mag the magnification if supported
         * @param text the text to measure
         * @return the X and Y dimensions
         */
        Coord textExtents(const void* font, int mag, const char* text) {
            return textExtents(font, mag, text, nullptr);
        }

        /**
         * Should be called before starting any rendering onto a device, in case there's anything that needs to be
         * done before drawing. This is a helper method that calls transaction()..
         */
        void startDraw() {
            transaction(true, false);
        }

        /**
         * Should be called after you've finished drawing, to ensure that everything is completed and drawn. This is a
         * helper method that calls transaction()..
         * @param needsDrawing true if there were changes, (default is true)
         */
        void endDraw(bool needsDrawing = true) {
            transaction(false, needsDrawing);
        }

        /**
         * set the draw color and background color to be used in subsequent operations
         * @param fg the foreground / draw color
         * @param bg the background color
         */
        void setColors(color_t fg, color_t bg) {
            backgroundColor = getUnderlyingColor(bg);
            drawColor = getUnderlyingColor(fg);
        }

        /**
         * Sets only the drawing color, leaving the background as it was before.
         * @param fg foreground / drawing color.
         */
        void setDrawColor(color_t fg) { drawColor = getUnderlyingColor(fg); }
    };

    /**
     * This class contains all the drawing code that is used for most graphical displays, it relies on an instance of
     * device drawable to do the drawing. It can also use sub drawing if the drawing device supports it, and it is enabled.
     * This means that on supported devices it is possible to do flicker free rendering.
     *
     * To support a new display, do not touch this class unless something is amiss or there is a bug, instead just implement
     * the above DeviceDrawable for that display. Take a look at the other rendering classes we already have for an example
     * of how.
     */
    class GraphicsDeviceRenderer : public BaseGraphicalRenderer {
    private:
        DeviceDrawable* rootDrawable;
        DeviceDrawable* drawable;
        ConfigurableItemDisplayPropertiesFactory propertiesFactory;
        bool redrawNeeded = false;
    public:
        GraphicsDeviceRenderer(int bufferSize, const char *appTitle, DeviceDrawable *drawable);

        void drawWidget(Coord where, TitleWidget *widget, color_t colorFg, color_t colorBg) override;
        void drawMenuItem(GridPositionRowCacheEntry *entry, Coord where, Coord areaSize, bool drawAll) override;
        void drawingCommand(RenderDrawingCommand command) override;

        void fillWithBackgroundTo(int endPoint) override;

        /**
         * Get the height for the font and add the descent to the bottom padding.
         * @param font the font to measure
         * @param mag any magnification to apply - if supported
         * @param padding the padding for the item, bottom will be adjusted
         * @return the height of the item.
         */
        int heightForFontPadding(const void *font, int mag, MenuPadding &padding);

        /**
         * Set up the display based on the legacy graphics configuration. This is deprecated and you should move to
         * using prepareDisplay. New code should NOT use this as it will be removed in a future build.
         * @deprecated use prepareDisplay with displayProperties overrides instead as per all examples.
         * @param gfxConfig the legacy graphics configuration
         */
        void setGraphicsConfiguration(void* gfxConfig);

        /**
         * Set up the display using a basic configuration. Setting factories with default colours and sizes.
         * @param monoPalette true if the display is monochrome, otherwise false.
         * @param itemFont the font to use for items
         * @param titleFont the font to use for the title
         * @param needEditingIcons true if editing icons should be prepared, otherwise false.
         */
        void prepareDisplay(bool monoPalette, const void *itemFont, int magItem, const void *titleFont, int magTitle, bool needEditingIcons);

        /**
         * Gets the abstract display properties factory, used internally to get the factory regardless of what actual
         * type it is, in user code that is using graphics properties factory, use getGraphicsPropertiesFactory
         * @return the properties factory
         */
        ItemDisplayPropertiesFactory &getDisplayPropertiesFactory() override { return propertiesFactory; }

        /**
         * Gets the graphical display properties factory, so that you can add graphics configuration easily.
         * @return the properties factory
         */
        ConfigurableItemDisplayPropertiesFactory &getGraphicsPropertiesFactory() { return propertiesFactory; }

        /**
         * Gets the underlying device drawable so that you can render to the screen in a device independent way.
         * On most systems this is a very thin wrapper on the library and performs very well for all but the most
         * intensive of drawing operations.
         * @return the underlying device drawable.
         */
        DeviceDrawable* getDeviceDrawable() { return rootDrawable; }
    private:
        int calculateSpaceBetween(const void* font, const char* buffer, int start, int end);
        void internalDrawText(GridPositionRowCacheEntry* pEntry, const Coord& where, const Coord& size);
        void drawCoreLineItem(GridPositionRowCacheEntry* entry, DrawableIcon* icon, Coord &where, Coord &size, bool drawBg);
        void drawTextualItem(GridPositionRowCacheEntry* entry, Coord& where, Coord& size);
        void drawSlider(GridPositionRowCacheEntry* entry, AnalogMenuItem* pItem, Coord& where, Coord& size);
        void drawUpDownItem(GridPositionRowCacheEntry* entry, Coord& where, Coord& size);
        void drawIconItem(GridPositionRowCacheEntry *pEntry, Coord& where, Coord& size);
        void drawBorderAndAdjustSize(Coord &where, Coord &size, MenuBorder &border);
    };

} // namespace tcgfx

#endif //TCLIBRARYDEV_GRAPHICSDEVICERENDERER_H
