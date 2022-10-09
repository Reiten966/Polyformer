/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_DRAWABLETOUCHCALIBRATOR_H
#define TCMENU_DRAWABLETOUCHCALIBRATOR_H

#include <PlatformDetermination.h>
#include "../graphics/GraphicsDeviceRenderer.h"
#include "../graphics/MenuTouchScreenEncoder.h"

namespace tcextras {

    /**
     * This class implements custom drawing so that it can take over the display and render a basic touch screen
     * calibration UI. It presents the position of the cursor using the current rotation so that it is easy to
     * provide a touch calibrator object.
     *
     * To use this class:
     * 1. Either using new or globally create an instance of this class.
     * 2. renderer.setCustomDrawingHandler(touchCalibrator);
     * 3. renderer.takeOverDisplay();
     */
    class TouchScreenCalibrator : public CustomDrawing {
    private:
        tcgfx::DeviceDrawable *drawable;
        tcgfx::MenuTouchScreenManager *touchScreen;
        tcgfx::GraphicsDeviceRenderer* renderer;
        int oldX = 0, oldY = 0;
        unsigned int takeOverCount = 0;
        TimerFn calibrationCompletedHandler;
    public:
        explicit TouchScreenCalibrator(tcgfx::MenuTouchScreenManager *touchScreen, tcgfx::GraphicsDeviceRenderer* renderer)
                : drawable(nullptr), touchScreen(touchScreen), renderer(renderer), calibrationCompletedHandler(nullptr) {}

        void setCalibrationCompletedHandler(TimerFn calibrationFinished) {
            calibrationCompletedHandler = calibrationFinished;
        }

        void reset() override {
            renderer->takeOverDisplay();
        }

        void started(BaseMenuRenderer *currentRenderer) override;

        void renderLoop(unsigned int currentValue, RenderPressMode userClick) override;

        void giveItBack();
    };
} // namespace tcextras

#endif // TCMENU_DRAWABLETOUCHCALIBRATOR_H
