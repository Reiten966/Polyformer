
#ifndef TCMENUEXAMPLE_SCREENSAVERCUSTOMDRAWING_H
#define TCMENUEXAMPLE_SCREENSAVERCUSTOMDRAWING_H

#include "stm32f4mbed_menu.h"
#include <Fonts/FreeSans9pt7b.h>
/**
 * Here we implement the custom drawing class so that we can register for drawing and reset events with it.
 * When the display times out the reset method is called, where in this case we take over the display. Then
 * when the take over of the display starts started() is called, followed by renderLoop() being called frequently
 * in a game loop until you give the display back.
 */
class ScreenSaverCustomDrawing : public CustomDrawing {
private:
    bool exitDisplayProc = false;
    int offsetX = 0, offsetY = 0;
    int renderTickCount = 0;
public:
    /**
     * Called when the display resets after a timeout, IE a period of inactivity. Here we use this to show
     * the screensaver
     */
    void reset() override {
        renderer.takeOverDisplay();
    }

    /**
     * Called when take over display starts, reset the display how you want it at this point and prepare any
     * variables etc.
     * @param currentRenderer the renderer that is being taken over.
     */
    void started(BaseMenuRenderer *currentRenderer) override {
        exitDisplayProc = false;
        offsetX = 16;
        offsetY = 16;

        gfx.clearDisplay();
        gfx.setFont(&FreeSans9pt7b);
        gfx.setTextSize(1);
    }

    /**
     * Called in a game loop so that you can render the screen if there are changes that need repainting.
     * @param currentValue the current value of the rotary encoder
     * @param userClick if the user has clicked on the button, one of: RPRESS_NONE, PRESS_PRESSED, RPRESS_HELD
     */
    void renderLoop(unsigned int currentValue, RenderPressMode userClick) override {
        if(userClick != RPRESS_NONE || exitDisplayProc || renderer.getDialog()->isInUse()) {
            renderer.giveBackDisplay();
            return;
        }

        if((renderTickCount % 100) == 0) {
            offsetX = rand() % 64;
            offsetY = rand() % 25;
        }

        gfx.clearDisplay();

        gfx.setCursor(offsetX, offsetY + 12);
        gfx.print("Mbed demo");

        gfx.setFont(nullptr);
        gfx.setCursor(5 + offsetX, 20 + offsetY);
        char sz[32];
        menuRTCDate.copyValue(sz, sizeof sz);
        gfx.print(sz);

        gfx.setCursor(5 + offsetX, 32 + offsetY);
        menuRTCTime.copyValue(sz, sizeof sz);
        gfx.print(sz);

        gfx.display();
        renderTickCount++;
    }

    /**
     * Close out the screen saver if presently active.
     */
    void removeScreenSaver() {
        exitDisplayProc = true;
    }
};

#endif //TCMENUEXAMPLE_SCREENSAVERCUSTOMDRAWING_H
