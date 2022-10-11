/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file MenuTouchScreenEncoder.h contains the interface between the touch screen and tcMenu.
 */

#ifndef TCMENU_MENUTOUCHSCREENENCODER_H
#define TCMENU_MENUTOUCHSCREENENCODER_H

#include "PlatformDetermination.h"
#include <IoAbstraction.h>
#include "graphics/BaseGraphicalRenderer.h"
#include <AnalogDeviceAbstraction.h>
#include <ResistiveTouchScreen.h>

namespace tcgfx {

    /**
     * A notification event sent by a touch screen implementation that provides if the event is raw (outside of menu item)
     * or within a menu item (local to the item). If it is local the coordinates are corrected to the item, otherwise they
     * are in terms of the screen. This also indicates the type of event too.
     */
    class TouchNotification {
    private:
        GridPositionRowCacheEntry* pEntry;
        Coord cursorPosition;
        Coord itemSize;
        bool withinItem;
        iotouch::TouchState touchState;
    public:
        TouchNotification(const Coord& rawCoords,  iotouch::TouchState touchState)
                : pEntry(nullptr), cursorPosition(rawCoords), itemSize(0, 0), withinItem(false), touchState(touchState) {}

        TouchNotification(GridPositionRowCacheEntry* ent, const Coord& local, const Coord& localStart, const Coord& localSize, iotouch::TouchState touchState)
                : pEntry(ent), cursorPosition(local), itemSize(localSize), withinItem(true), touchState(touchState) {}

        GridPositionRowCacheEntry* getEntry() const {
            return pEntry;
        }

        const Coord &getCursorPosition() const {
            return cursorPosition;
        }

        const Coord &getItemSize() const {
            return itemSize;
        }

        bool isWithinItem() const {
            return withinItem;
        }

        iotouch::TouchState getTouchState() const {
            return touchState;
        }
    };

    class TouchObserver {
    public:
        virtual void touched(const TouchNotification& notification)=0;
    };

    class MenuTouchScreenEncoder : public TouchObserver {
    private:
        BaseGraphicalRenderer *renderer;
    public:
        explicit MenuTouchScreenEncoder(BaseGraphicalRenderer* rend) {
            renderer = rend;
        }

        void touched(const TouchNotification& notification) override;
    };

    /**
     * This class provides support for resistive touch screens within tcMenu, it is the layer between the IoAbstraction
     * touch interface class and tcMenu's rendering system, it uses the renderer to work out if the touch was within
     * a menu item and prepares the touch events. Normally this class is instantiated in your project by tcMenu Designer
     * if you choose a touch interface.
     */
    class MenuTouchScreenManager : public iotouch::TouchScreenManager {
    private:
        GridPositionRowCacheEntry* currentlySelected;
        Coord localStart;
        Coord localSize;
        MenuTouchScreenEncoder encoder;
        BaseGraphicalRenderer* renderer;
        TouchObserver* observer;
        float lastX, lastY;
        iotouch::TouchState currentState;
    public:
        /**
         * Create a touch screen interface for a resistive touch screen, this does not start the interface, you need to
         * call start() yourself to do that.
         * @param device the analog device that will read analog inputs. - always must be internal analog pins
         * @param pins the pins that will be used for digital IO - always must be internal device pins
         * @param xpPin the X+ pin from the touch unit
         * @param xnPin the X- pin from the touch unit - requires analog input pin
         * @param ypPin the Y+ pin from the touch unit - requires analog input pin
         * @param ynPin the Y- pin from the touch unit
         * @param renderer the graphics renderer that is in use
         * @param rotation the rotation of the touch interface
         */
        MenuTouchScreenManager(iotouch::TouchInterrogator* interrogator, BaseGraphicalRenderer* renderer,
                               iotouch::TouchInterrogator::TouchRotation rotation);

        void sendEvent(float locationX, float locationY, float touchPressure, iotouch::TouchState touched) override;

        iotouch::TouchState getLastTouchState() const { return currentState; }
        float getLastX() const { return lastX; }
        float getLastY() const { return lastY; }
    };

} // namespace tcgfx

#endif //TCMENU_MENUTOUCHSCREENENCODER_H
