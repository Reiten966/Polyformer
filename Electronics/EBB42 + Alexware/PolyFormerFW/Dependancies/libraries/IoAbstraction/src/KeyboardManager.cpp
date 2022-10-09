
/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * KeyboadManager.cpp contains the classes needed to deal with editing using matrix keyboards
 */

#include "KeyboardManager.h"
#include "IoLogging.h"

MatrixKeyboardManager* MatrixKeyboardManager::INSTANCE = nullptr;

ISR_ATTR void rawKeyboardInterrupt() {
    auto kbMgr = MatrixKeyboardManager::INSTANCE;
    if(kbMgr->keyMode == KEYMODE_NOT_PRESSED) {
        // we only need to be notified when not pressed. As in other states we are polling
        kbMgr->markTriggeredAndNotify();
    }
}

MatrixKeyboardManager::MatrixKeyboardManager() {
    this->ioRef = nullptr;
    this->layout = nullptr;
    this->listener = nullptr;
    currentKey = 0;
    keyMode = KEYMODE_NOT_PRESSED;
    interruptMode = false;
    counter = 0;
    INSTANCE = this;
}

void MatrixKeyboardManager::initialise(IoAbstractionRef ref, KeyboardLayout* layout_, KeyboardListener* listener_, bool interruptMode_) {
    this->ioRef = ref;
    this->layout = layout_;
    this->listener = listener_;
    this->interruptMode = interruptMode_;

    for(int i=0; i<layout->numColumns(); i++) {
        ioDevicePinMode(ioRef, layout->getColPin(i), OUTPUT);
        ioDeviceDigitalWrite(ioRef, layout->getColPin(i), LOW);
    }
    for(int i=0; i<layout->numRows(); i++) {
        ioDevicePinMode(ioRef, layout->getRowPin(i), INPUT_PULLUP);
        if(interruptMode && INSTANCE) {
            ioRef->attachInterrupt(layout->getRowPin(i), rawKeyboardInterrupt, CHANGE);
        }
    }

    ioDeviceSync(ioRef);

    currentKey = 0;
    taskManager.registerEvent(this);
}

void MatrixKeyboardManager::setToOutput(int col) {
    for(int i=0; i<layout->numColumns(); i++) {
        ioDeviceDigitalWrite(ioRef, layout->getColPin(i), col != i);
    }
}

void MatrixKeyboardManager::setRepeatKeyMillis(int startAfterMillis, int repeatMillis) {
    repeatStartTicks = startAfterMillis / KEYBOARD_TASK_MILLIS; 
    repeatTicks = repeatMillis / KEYBOARD_TASK_MILLIS; 
}

inline bool isDebouncing(KeyMode keyMode) {
    return keyMode == KEYMODE_DEBOUNCE || keyMode == KEYMODE_DEBOUNCE1 || keyMode == KEYMODE_DEBOUNCE2;
}

void MatrixKeyboardManager::exec() {
    if(ioRef == nullptr) {
        serlogF(SER_ERROR, "ioRef null");
        return;
    }

    char pressThisTime = 0;


    // then we read back the right state
    for(int c=0;c<layout->numColumns();c++) {
        setToOutput(c);
        ioDeviceSync(ioRef); // first we set the right column low.
        taskManager.yieldForMicros(500); // let things settle while other tasks run.
        ioDeviceSync(ioRef); // then we read the latest row states back

        for(int r=0; r<layout->numRows(); r++) {
            if(!ioDeviceDigitalRead(ioRef, layout->getRowPin(r))) {
                pressThisTime = layout->keyFor(r, c);
                serlogF4(SER_IOA_DEBUG, "Pressed: ", r, c, (int)pressThisTime);
            }
        }
    }

    // if the key is the same as last time and not zero
    if(pressThisTime == currentKey && pressThisTime) {
        // then we either have finished debouncing or are repeating
        if(isDebouncing(keyMode)) {
            keyMode = KEYMODE_PRESSED;
            counter = repeatStartTicks;
            listener->keyPressed(currentKey, false);
        }
        else if(keyMode == KEYMODE_PRESSED) {
            if(counter-- == 0) {
                counter = repeatTicks;
                listener->keyPressed(currentKey, true);
            }
        }
        else keyMode = KEYMODE_DEBOUNCE;
    } else {
        // first clear any existing state
        if(keyMode == KEYMODE_PRESSED) {
            keyMode = KEYMODE_NOT_PRESSED;
            counter = 0;
            listener->keyReleased(currentKey);
            currentKey = 0;
        }
        doDebounce(pressThisTime);
    }

    enableAllOutputsForInterrupt();
}

uint32_t MatrixKeyboardManager::timeOfNextCheck() {

    if(interruptMode && (keyMode == KEYMODE_NOT_PRESSED)) {
        return secondsToMicros(1);
    } else {
        setTriggered(true);
        return millisToMicros(KEYBOARD_TASK_MILLIS);
    }
}

void MatrixKeyboardManager::enableAllOutputsForInterrupt() {
    if(interruptMode && keyMode == KEYMODE_NOT_PRESSED) {
        // in interrupt mode we set all output pins low when nothing is pressed so that any change will be detected.
        // this effectively means that each column pin is low and will pull down the input line. We don't need to
        // know what is pressed, just that something was pressed.
        for(int i=0; i < layout->numColumns(); i++) {
            ioDeviceDigitalWrite(ioRef, layout->getColPin(i), 0);
        }
        ioDeviceSync(ioRef);
    }
}

void MatrixKeyboardManager::doDebounce(char pressedNow) {
    if(isDebouncing(keyMode) && currentKey == pressedNow) {
        currentKey = pressedNow;
        keyMode = KEYMODE_PRESSED;
        return;
    }

    if(pressedNow && keyMode == KEYMODE_NOT_PRESSED) {
        currentKey = pressedNow;
        keyMode = KEYMODE_DEBOUNCE;
    } else if(keyMode == KEYMODE_DEBOUNCE) {
        keyMode = KEYMODE_DEBOUNCE1;
    } else if(keyMode == KEYMODE_DEBOUNCE1) {
        keyMode = KEYMODE_DEBOUNCE2;
    } else if (keyMode == KEYMODE_DEBOUNCE2) {
        keyMode = KEYMODE_NOT_PRESSED;
    }
}
