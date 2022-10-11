/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "DrawableTouchCalibrator.h"
#include <BaseDialog.h>

using namespace tcextras;
using namespace tcgfx;

#define TOUCH_BLACK RGB(0,0,0)
#define TOUCH_ORANGE RGB(255, 69, 0)
#define TOUCH_YELLOW RGB(255, 255, 0)

void TouchScreenCalibrator::started(BaseMenuRenderer *currentRenderer) {
    if (currentRenderer->getRendererType() != RENDER_TYPE_CONFIGURABLE) {
        giveItBack();
    }
    drawable = renderer->getDeviceDrawable();
    drawable->startDraw();
    drawable->setDrawColor(TOUCH_BLACK);
    drawable->drawBox(Coord(0, 0), drawable->getDisplayDimensions(), true);
    drawable->endDraw();
    takeOverCount = 0;
}

void TouchScreenCalibrator::renderLoop(unsigned int currentValue, RenderPressMode userClick) {

    // If the dialog is in use then we leave this loop immediately to give it priority.
    // As of 2.1 onwards this behaviour is up to you, you can choose to have higher priority than dialogs.
    if (renderer->getDialog() != nullptr && renderer->getDialog()->isInUse()) {
        giveItBack();
        return;
    }

    drawable->startDraw();
    drawable->setDrawColor(TOUCH_BLACK);
    drawable->drawCircle(Coord(oldX, oldY), 10, true);

    drawable->drawBox(Coord(0, 0), Coord(drawable->getDisplayDimensions().x, 45), true);
    drawable->setColors(TOUCH_ORANGE, TOUCH_BLACK);
    drawable->drawText(Coord(0, 2), nullptr, 0, "Calibrate screen");

    char sz[40];
    strcpy(sz, "x: ");
    fastftoa(sz, touchScreen->getLastX(), 3, sizeof sz);
    strcat(sz, ", y: ");
    fastftoa(sz, touchScreen->getLastY(), 3, sizeof sz);
    strcat(sz, ", z: ");
    fastltoa(sz, touchScreen->getLastTouchState(), 1, NOT_PADDED, sizeof sz);

    drawable->setColors(TOUCH_YELLOW, TOUCH_BLACK);
    drawable->drawText(Coord(0, 22), nullptr, 0, sz);

    oldX = int(touchScreen->getLastX() * (float) drawable->getDisplayDimensions().x);
    oldY = int(touchScreen->getLastY() * (float) drawable->getDisplayDimensions().y);
    drawable->setDrawColor(TOUCH_YELLOW);
    drawable->drawCircle(Coord(oldX, oldY), 10, true);

    if (oldX < 40 && oldY < 40 && touchScreen->getLastTouchState() == iotouch::HELD) {
        giveItBack();
    }
}

void TouchScreenCalibrator::giveItBack() {
    renderer->giveBackDisplay();
    if(calibrationCompletedHandler) calibrationCompletedHandler();
}
