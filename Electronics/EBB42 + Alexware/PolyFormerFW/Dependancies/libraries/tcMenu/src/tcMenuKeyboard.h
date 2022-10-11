/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TCMENU_KEYBOARD_H_
#define _TCMENU_KEYBOARD_H_

#include <KeyboardManager.h>

/**
 * @file tcMenuKeyboard.h contains a keyboard listener for use with MatrixKeyboardManager that can
 * control a menu system
 */

/** internal enumeration used by the keyboard listener to hold state */
enum MenuEditingKeyMode: uint8_t {
	KEYEDIT_NONE,
	KEYEDIT_ANALOG_EDIT_WHOLE,
	KEYEDIT_ANALOG_EDIT_FRACT,
	KEYEDIT_MULTIEDIT_INT_START = 100,
};

typedef void (*DirectionalItemCallback)(MenuItem* item, uint16_t currentValue);

class EditableLargeNumberMenuItem;
class ScrollChoiceMenuItem;

#define KEY_NOT_CONFIGURED 0xff

/**
 * An implementation of the key listener that can be used with TcMenu to edit menu items and control the menu where
 * nearly all types of menu item and navigation are supported. For some cases it may be beneficial to have a rotary
 * encoder with center button, but it is not needed as every menu operation can be performed via the keyboard.
 *
 * If you have enough keys you can set two keys to be next and back, where they will allow the next menu item to become
 * When not in edit mode, the keyboard 0-9 keys can be used to select items where 1 is the first item to select. For
 * any keys you do not wish to map, set them to `KEY_NOT_CONFIGURED` or `-1`.
 *
 */
class MenuEditingKeyListener : public KeyboardListener {
private:
    BaseMenuRenderer* baseRenderer;
	WholeAndFraction currentValue;
	MenuItem* currentEditor;
	MenuEditingKeyMode mode;
    uint8_t deleteKey;
    uint8_t enterKey;
    uint8_t backKey;
    uint8_t nextKey;
public:
    /**
     * Construct the key listener that will control TcMenu based on key presses. It is passed as the listener to an instance
     * of IoAbstraction's `KeyboardManager`. You can set which keys act in certain roles such as enter and delete.
     * @param enterKey the key code for enter - defaulted to '#'
     * @param deleteKey the key code for del/exit, defaulted to '*'
     * @param backKey the key code for back, defaulted to 'A'
     * @param nextKey the key code for next, defaulted to 'B'
     */
	MenuEditingKeyListener(uint8_t enterKey = '*', uint8_t deleteKey = '#', uint8_t backKey = 'A', uint8_t nextKey = 'B') :
            currentEditor(nullptr), mode(KEYEDIT_NONE), deleteKey(deleteKey), enterKey(enterKey), backKey(backKey), nextKey(nextKey) {}

    /**
     * Implements the key pressed interface method from KeyboardListener, this should not be called by user code
     * @param key the keycode
     * @param held if it is held down.
     */
    void keyPressed(char key, bool held) override;

    /**
     * Implements the key released interface method from KeyboardListener, this should not be called by user code
     * @param key the keycode
     */
    void keyReleased(char key) override;
private:
    void processDirectionalIndexItem(MenuItem *item, uint16_t currVal, char key, DirectionalItemCallback callback);
    void processSimpleValueKeyPress(ValueMenuItem* item, char key);
    void processScrollValueKeyPress(ScrollChoiceMenuItem* item, char key);
    void processAnalogKeyPress(AnalogMenuItem* item, char key);
	void processMultiEditKeyPress(TextMenuItem* item, char key);
	void processIntegerMultiEdit(EditableMultiPartMenuItem* item, char key);
    void processLargeNumberPress(EditableLargeNumberMenuItem*, char key);
    void processListMenuSelection(ListRuntimeMenuItem *item, char key);
	void clearState();

    void workOutEditorPosition();
};

#endif // _TCMENU_KEYBOARD_H_
