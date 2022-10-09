/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file tcMenuU8g2.h
 * 
 * U8g2 renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This library requires the u8g2 library available for download from your IDE library manager.
 */

#ifndef _TCMENU_U8G2_H_
#define _TCMENU_U8G2_H_

#include <tcMenu.h>
#include <tcUtil.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <graphics/BaseGraphicalRenderer.h>
#include <graphics/GraphicsDeviceRenderer.h>
#include <BaseDialog.h>
#include <tcUtil.h>


// If you DONT want task manager yield code in I2C set to 0
#ifndef WANT_TASK_MANAGER_FRIENDLY_YIELD
#define WANT_TASK_MANAGER_FRIENDLY_YIELD 1
#endif // WANT_TASK_MANAGER_FRIENDLY_YIELD

using namespace tcgfx;

/**
 * A standard menu render configuration that describes how to renderer each item and the title.
 * Specialised for u8g2 fonts.
 */
typedef struct ColorGfxMenuConfig<const uint8_t*> U8g2GfxMenuConfig;

// some colour displays don't create this value
#ifndef BLACK
#define BLACK 0
#endif

// some colour displays don't create this value
#ifndef WHITE
#define WHITE 1
#endif

/**
 * This is used to draw to I2C including a task manager yield to improve performance on slower I2C devices.
 * Not really needed for SPI as we are talking in low order millis for a full refresh.
 */
uint8_t u8g2_byte_with_yield(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

/**
 * A basic renderer that can use the AdaFruit_GFX library to render information onto a suitable
 * display. It is your responsibility to fully initialise and prepare the display before passing
 * it to this renderer. The usual procedure is to create a display variable globally in your
 * sketch and then provide that as the parameter to setGraphicsDevice. If you are using the
 * designer you provide the display variable name in the code generation parameters.
 *
 * You can also override many elements of the display using AdaColorGfxMenuConfig, to use the defaults
 * just call prepareAdaColorDefaultGfxConfig(..) passing it a pointer to your config object. Again the
 * designer UI takes care of this.
 */
class U8g2Drawable : public DeviceDrawable {
private:
	U8G2* u8g2;
#if WANT_TASK_MANAGER_FRIENDLY_YIELD == 1
    static TwoWire* pWire;
    friend uint8_t u8g2_byte_with_yield(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
#endif // WANT_TASK_MANAGER_FRIENDLY_YIELD
public:
    explicit U8g2Drawable(U8G2* u8g2, TwoWire* wire = nullptr);
    ~U8g2Drawable() override = default;

    DeviceDrawable* getSubDeviceFor(const Coord &where, const Coord &size, const color_t *palette, int paletteSize) override {return nullptr; }

    void drawText(const Coord &where, const void *font, int mag, const char *text) override;
    void drawBitmap(const Coord &where, const DrawableIcon *icon, bool selected) override;
    void drawXBitmap(const Coord &where, const Coord &size, const uint8_t *data) override;
    void drawBox(const Coord &where, const Coord &size, bool filled) override;
    void drawCircle(const Coord &where, int radius, bool filled) override;
    void drawPolygon(const Coord *points, int numPoints, bool filled) override;

    Coord getDisplayDimensions() override {  return Coord(u8g2->getWidth(), u8g2->getHeight()); }
    void transaction(bool isStarting, bool redrawNeeded) override;
    Coord textExtents(const void *font, int mag, const char *text, int *baseline) override;
    color_t getUnderlyingColor(color_t col) { return (col<4) ? col : 1; }

};

#endif // _TCMENU_U8G2_H_
