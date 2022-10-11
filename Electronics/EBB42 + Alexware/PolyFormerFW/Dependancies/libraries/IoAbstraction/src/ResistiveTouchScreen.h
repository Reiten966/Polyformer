
#ifndef IOA_RESISTIVETOUCHSCREEN_H
#define IOA_RESISTIVETOUCHSCREEN_H

#include "PlatformDetermination.h"
#include "AnalogDeviceAbstraction.h"
#include <TaskManagerIO.h>

/**
 * @file ResistiveTouchScreen.h A simple resistive touch screen class that converts touch coordinates into a range between
 * floating point values 0.0 and 1.0.
 */

#ifndef TOUCH_THRESHOLD
#define TOUCH_THRESHOLD 0.05F
#endif

namespace iotouch {
    enum AccelerationMode: uint8_t {
        WAITING,
        ACCELERATING,
        NEVER_ACCELERATES
    };

    class AccelerationHandler {
    private:
        uint8_t minTicks;
        uint8_t ticks;
        uint8_t accel;
        AccelerationMode mode;
    public:
        AccelerationHandler(uint8_t minTicks, bool accelerate) : minTicks(minTicks) {
            mode = accelerate ? WAITING : NEVER_ACCELERATES;
        }

        void reset() {
            if(mode == ACCELERATING) mode = WAITING;
        }

        bool tick() {
            if(mode == WAITING) {
                mode = ACCELERATING;
                ticks = 0;
                accel = 800 / SWITCH_POLL_INTERVAL;
            }
            if(ticks++ > accel) {
                ticks = 0;
                accel = max(minTicks, uint8_t(accel / 2U));
                return true;
            }
            return false;
        }
    };

    class CalibrationHandler {
    private:
        float minX, maxX;
        float minY, maxY;
        bool calibrationOn;

    public:
        CalibrationHandler() = default;

        void setCalibrationValues(float mnX, float mxX, float mnY, float mxY) {
            minX = mnX;
            minY = mnY;
            maxX = mxX;
            maxY = mxY;
            calibrationOn = true;
        }

        void enableCalibration(bool state) {
            calibrationOn = state;
        }

        float calibrateX(float rawValue, bool isInverted) const  {
            auto x = (calibrationOn) ? ((rawValue - minX) * (1.0F / (maxX - minX))) : rawValue;
            return isInverted ? 1.0F - x : x;
        }

        float calibrateY(float rawValue, bool isInverted) const {
            auto y = (calibrationOn) ? ((rawValue - minY) * (1.0F / (maxY - minY))) : rawValue;
            return isInverted ? 1.0F - y : y;
        }
    };

    enum TouchState : uint8_t {
        /** no touch has been detected */
        NOT_TOUCHED,
        /** the display has been touched */
        TOUCHED,
        /** the touch is being dragged or held */
        HELD,
        /** a debounce is needed */
        TOUCH_DEBOUNCE
    };

#define portableFloatAbs(x) ((x)<0.0F?-(x):(x))

    class TouchInterrogator {
    public:
        enum TouchRotation : uint8_t {
            PORTRAIT,
            PORTRAIT_INVERTED,
            LANDSCAPE,
            LANDSCAPE_INVERTED,
            RAW
        };

        virtual TouchState internalProcessTouch(float* ptrX, float* ptrY, TouchRotation rotation, const CalibrationHandler& calib)=0;
    };

    class TouchInterrogator;

    class TouchScreenManager : public Executable {
    public:
    private:
        AccelerationHandler accelerationHandler;
        CalibrationHandler calibrator;
        TouchInterrogator* touchInterrogator;
        TouchState touchMode;
        bool usedForScrolling = false;
        TouchInterrogator::TouchRotation rotation;
    public:
        explicit TouchScreenManager(TouchInterrogator* interrogator, TouchInterrogator::TouchRotation rot) :
                accelerationHandler(10, true), calibrator(),
                touchInterrogator(interrogator), touchMode(NOT_TOUCHED), rotation(rot) {}

        void start() {
            touchMode = NOT_TOUCHED;
            taskManager.execute(this);
        }

        void setUsedForScrolling(bool scrolling) {
            usedForScrolling = scrolling;
        }

        void calibrateMinMaxValues(float xmin, float xmax, float ymin, float ymax) {
            calibrator.setCalibrationValues(xmin, xmax, ymin, ymax);
        }

        void enableCalibration(bool ena) {
            calibrator.enableCalibration(ena);
        }

        void exec() override {
            float x;
            float y;
            auto touch = touchInterrogator->internalProcessTouch(&x, &y, rotation, calibrator);
            if(x < 0.0F) x = 0.0F;
            if(y < 0.0F) y = 0.0F;
            // now determine what state we are in, touched, not touched or held.
            auto oldTouchMode = touchMode;
            switch(touch) {
                case NOT_TOUCHED:
                    touchMode = NOT_TOUCHED;
                    break;
                case TOUCHED:
                case HELD:
                    touchMode = (oldTouchMode == TOUCHED || oldTouchMode == HELD) ? HELD : TOUCHED;
                    break;
                case TOUCH_DEBOUNCE:
                    taskManager.scheduleOnce(5, this, TIME_MILLIS);
                    return;
            }

            // we are in a repeated not touch situation, we can slow down the polling slightly now. No update needed
            // even at 1/10th of a second, we'll still wake up pretty quick when they select something.
            if (oldTouchMode == NOT_TOUCHED && touchMode == NOT_TOUCHED) {
                taskManager.scheduleOnce(100, this, TIME_MILLIS);
                accelerationHandler.reset();
                return;
            }

            // only the held state state is subject to acceleration control
            if(touchMode != HELD || usedForScrolling || accelerationHandler.tick()) {
                if (rotation == TouchInterrogator::LANDSCAPE || rotation == TouchInterrogator::LANDSCAPE_INVERTED) {
                    sendEvent(y, x, touch, touchMode);
                } else {
                    sendEvent(x, y, touch, touchMode);
                }
            }
            taskManager.scheduleOnce(20, this, TIME_MILLIS);
        }

        TouchInterrogator::TouchRotation changeRotation(TouchInterrogator::TouchRotation newRotation) {
            auto oldRotation = rotation;
            rotation = newRotation;
            return oldRotation;
        }

        /**
         * You must create a subclass extends from this and takes the three values converting into an
         * event for processing.
         *
         * @param locationX the location between 0 and 1 in the X domain
         * @param locationY the location between 0 and 1 in the Y domain
         * @param touched if the panel is current touched
         */
        virtual void sendEvent(float locationX, float locationY, float touchPressure, TouchState touched) = 0;
    };

    /**
     * This class handles the basics of a touch screen interface, capturing the values and converting them into a usable
     * form, it is pure abstract and the sendEvent needs implementing with a suitable implemetnation for your needs.
     * It is heavily based on the Adafruit TouchScreen library but modified to work with AnalogDevice so that it can
     * work reliably across a wider range of devices.
     *
     * Important notes
     *
     * * all the GPIOs used must be OUTPUT capable, this matters on some boards such as ESP32
     * * Y+ and X- must be connected to ADC (analog input capable) pins.
     * * it uses taskManager and takes readings at the millisecond interval provided.
     */
    class ResistiveTouchInterrogator : public TouchInterrogator {
    private:
        pinid_t xpPin, xnPinAdc, ypPinAdc, ynPin;
    public:

        ResistiveTouchInterrogator(pinid_t xpPin, pinid_t xnPin, pinid_t ypPin, pinid_t ynPin)
                : xpPin(xpPin), xnPinAdc(xnPin), ypPinAdc(ypPin), ynPin(ynPin) {}

        TouchState internalProcessTouch(float* ptrX, float* ptrY, TouchRotation rotation, const CalibrationHandler& calibrator) override {
            auto* analogDevice = internalAnalogIo();
            auto* device = internalDigitalIo();
            // first we calculate everything in the X dimension.
            analogDevice->initPin(ypPinAdc, DIR_IN);
            ioDevicePinMode(device, xnPinAdc, OUTPUT);
            ioDevicePinMode(device, ynPin, INPUT);
            ioDevicePinMode(device, xpPin, OUTPUT);
            ioDeviceDigitalWrite(device, xpPin, HIGH);
            ioDeviceDigitalWriteS(device, xnPinAdc, LOW);

            taskManager.yieldForMicros(20);
            float firstSample = analogDevice->getCurrentFloat(ypPinAdc);
            float secondSample = analogDevice->getCurrentFloat(ypPinAdc);

            if (portableFloatAbs(firstSample - secondSample) > 0.007) {
                return TOUCH_DEBOUNCE;
            }
            float x = calibrator.calibrateX((firstSample + secondSample) / 2.0F, (rotation == LANDSCAPE_INVERTED || rotation == PORTRAIT));

            // now we calculate everything in the Y dimension.
            analogDevice->initPin(xnPinAdc, DIR_IN);
            ioDevicePinMode(device, xpPin, INPUT);
            ioDevicePinMode(device, ypPinAdc, OUTPUT);
            ioDevicePinMode(device, ynPin, OUTPUT);
            ioDeviceDigitalWrite(device, ypPinAdc, HIGH);
            ioDeviceDigitalWriteS(device, ynPin, LOW);

            taskManager.yieldForMicros(20);
            firstSample = analogDevice->getCurrentFloat(xnPinAdc);
            secondSample = analogDevice->getCurrentFloat(xnPinAdc);

            if (portableFloatAbs(firstSample - secondSample) > 0.007) {
                return TOUCH_DEBOUNCE;
            }
            float y = calibrator.calibrateY((firstSample + secondSample) / 2.0F, (rotation == LANDSCAPE || rotation == PORTRAIT));

            // and finally the Z dimension
            ioDevicePinMode(device, xpPin, OUTPUT);
            analogDevice->initPin(ypPinAdc, DIR_IN);
            ioDeviceDigitalWrite(device, xpPin, LOW);
            ioDeviceDigitalWriteS(device, ynPin, HIGH);

            taskManager.yieldForMicros(20);

            firstSample = analogDevice->getCurrentFloat(xnPinAdc);
            secondSample = analogDevice->getCurrentFloat(ypPinAdc);

            //float touch = ((z2 / z1) * -1.0) * x * resistanceX;
            float touch = 1.0F - (secondSample - firstSample);
            *ptrX = x;
            *ptrY = y;
            return (touch > TOUCH_THRESHOLD) ? TOUCHED : NOT_TOUCHED;
        }

    };

    /**
     * Handles the touch screen interface by storing the latest values for later use. This allows for simple uses of
     * touch screen without creating a sub class, by just getting the latest values as needed.
     */
    class ValueStoringResistiveTouchScreen : public TouchScreenManager {
    private:
        float lastX, lastY, touchPressure;
        TouchInterrogator& interrogator;
        TouchState touchState;
    public:
        ValueStoringResistiveTouchScreen(TouchInterrogator& interrogator, TouchInterrogator::TouchRotation rotation)
            : TouchScreenManager(&interrogator, rotation), interrogator(interrogator) {}

        void sendEvent(float locationX, float locationY, float pressure, TouchState touched) override {
            lastX = locationX;
            lastY = locationY;
            touchState = touched;
            touchPressure = pressure;
        }

        float getTouchPressure() const {
            return touchPressure;
        }

        float getLastX() const {
            return lastX;
        }

        float getLastY() const {
            return lastY;
        }

        bool isPressed() const {
            return touchState == TOUCHED;
        }

        TouchState getTouchState() const {
            return touchState;
        }
    };
}

#endif //IOA_RESISTIVETOUCHSCREEN_H
