/**
 * This class implements CustomDrawing which means that it can be registered with the renderer
 * to be told when the screen is about to reset (become idle basically). This callback will
 * also be told when a takeOverDisplay call starts, and then the rendering function is called
 * frequently, so you can update the display.
 *
 * Associated docs:
 * https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/renderer-take-over-display/
 */

#ifndef TCMENU_EXAMPLE_COLORETHERENETCUSTOMDRAW_H
#define TCMENU_EXAMPLE_COLORETHERENETCUSTOMDRAW_H

#include <BaseRenderers.h>
#include "colorTftEthernet32_menu.h"

class ColorEthernetCustomDraw : public CustomDrawing {
public:
    void reset() override {
        // When the display becomes idle, we take over the display, the no-arg version is
        // used to indicate you want the custom draw class to do the rendering.
        renderer.takeOverDisplay();
    }

    void started(BaseMenuRenderer *currentRenderer) override {
        // you need to handle the clearing and preparation of the display when first called.
        switches.getEncoder()->changePrecision(1000, 500);
        gfx.setCursor(0, 0);
        gfx.fillRect(0, 0, gfx.width(), gfx.height(), BLACK);
        gfx.setFont(nullptr);
        gfx.setTextSize(2);
        gfx.print("Encoder ");
    }


    void renderLoop(unsigned int currentValue, RenderPressMode pressType) override {
        // if the encoder / select button is held, we go back to the menu.
        if(pressType == RPRESS_HELD) {
            renderer.giveBackDisplay();
            return;
        }

        GFXcanvas1 canvas(100, 20);
        canvas.fillScreen(BLACK);
        canvas.setCursor(0,0);
        canvas.print(currentValue);
        canvas.setTextSize(2);
        gfx.drawBitmap(0, 35, canvas.getBuffer(), 100, 20, WHITE, BLACK);

    }
};

#endif //TCMENU_EXAMPLE_COLORETHERENETCUSTOMDRAW_H
