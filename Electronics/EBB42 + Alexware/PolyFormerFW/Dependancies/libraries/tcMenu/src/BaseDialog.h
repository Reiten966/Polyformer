/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_BASE_DIALOG_H_
#define TCMENU_BASE_DIALOG_H_

/**
 * @file BaseDialog.h
 * 
 * The definition of the base dialog object, that can be used with any display type. These are designed
 * to handle simple user informational pages and questions.
 * 
 * Dialogs are a core function of tcMenu, and must be supported on all available platforms.
 */

#define FIRST_DEFAULT_BUTTON 0
#define SECOND_DEFAULT_BUTTON 1
#define CUSTOM_DIALOG_BUTTON_START 2

/**
 * The types of button that can be passed to setButton for button 1 and button 2
 */
enum ButtonType: uint8_t {
    BTNTYPE_OK, BTNTYPE_ACCEPT, BTNTYPE_CANCEL, BTNTYPE_CLOSE, BTNTYPE_NONE,
    BTNTYPE_CUSTOM0 = 15, BTNTYPE_CUSTOM1, BTNTYPE_CUSTOM2, BTNTYPE_CUSTOM3,
    BTNTYPE_CUSTOM4, BTNTYPE_CUSTOM5, BTNTYPE_CUSTOM6, BTNTYPE_CUSTOM7 };


class BaseDialog;
/**
 * If you need to capture the finished state of the dialog, then you create a function to this spec
 * and it will be called back once the dialog is dismissed. Example:
 * `void completedCallback(ButtonType buttonPressed);`
 */
typedef void (*CompletedHandlerFn)(ButtonType buttonPressed, void* yourData);

#define DLG_FLAG_SMALLDISPLAY 0
#define DLG_FLAG_INUSE 1
#define DLG_FLAG_CAN_SEND_REMOTE 2 
#define DLG_FLAG_MENUITEM_BASED 3
#define DLG_FLAG_NEEDS_RENDERING 4
#define DLG_FLAG_USING_OO_CONTROLLER 5
#define DLG_FLAG_REMOTE_0 8
#define DLG_FLAG_REMOTE_1 9
#define DLG_FLAG_REMOTE_2 10
#define DLG_FLAG_REMOTE_3 11
#define DLG_FLAG_REMOTE_4 12

#define DLG_FLAG_REMOTE_MASK 0xff00

#define DLG_VISIBLE 'S'
#define DLG_HIDDEN 'H'
#define DLG_ACTION 'A'

class TagValueRemoteConnector; // forward reference

/**
 * This class provides a means of controlling a dialog, in terms of what happens when a dialog is presented, when
 * buttons are pressed, the text that should appear on custom buttons, and also when a dialog is dismissed. You can
 * add extra menu items to the dialog during initialiseAndGetHeader(), the button grid positioning will make space
 * for every additional element that you add.
 */
class BaseDialogController {
public:
    /**
     * Initialise the dialog and get the text for the header. It is called once during the dialog being shown. If you
     * need to add extra menu items or buttons, this is the place to add them.
     * @param dialog the dialog object
     * @param buffer the buffer to copy the title to
     * @param bufferSize the size of the buffer
     */

    virtual void initialiseAndGetHeader(BaseDialog* dialog, char* buffer, size_t bufferSize)=0;
    /**
     * Called when the dialog is dismissed, along with with the button type that was pressed
     * @param buttonType the button type that was pressed
     */
    virtual void dialogDismissed(ButtonType buttonType)=0;

    /**
     * Called when a dialog button is pressed, the button number from 0..N not the button type.
     * @param buttonNum the button number
     * @return true if the action results in the dialog being dismissed
     */
    virtual bool dialogButtonPressed(int buttonNum)=0;

    /**
     * Copy the button text for any custom buttons by their button number
     * @param buttonNumber the button number
     * @param buffer the buffer to copy text into
     * @param bufferSize the size of the buffer
     */
    virtual void copyCustomButtonText(int buttonNumber, char* buffer, size_t bufferSize)=0;
};

/**
 * A dialog is able to take over the display completely in order to present the user with information
 * or a question. Similar to a message box on desktop platforms. These allow for the simplest of user
 * interactions in a display independent way. This base class has most of the functions needed to
 * handle the non-rendering related activities. Never create one directly, always use the instance
 * that's available from the renderer using `renderer.getDialog()`.
 */
class BaseDialog {
protected:
    char header[20];
    const char* headerPgm; // for backwards compatibility with 1.7, not used in 2.0
    union {
        BaseDialogController* controller;
        CompletedHandlerFn completedHandler;
    };
    void* userData;
    ButtonType button1;
    ButtonType button2;
    uint8_t lastBtnVal;
    uint16_t flags;
    MenuRedrawState needsDrawing;
public:
    /**
     * Create the base dialog and clear down all the fields
     */
    BaseDialog();
    virtual ~BaseDialog() = default;

    /**
     * Sets the buttons that are available for pressing. One of the ButtonType definitions.
     * You can optionally define if 0 or 1 should be the default.
     * @param btn1 the first button type (can be BTNTYPE_NONE, and btn2 becomes the only button)
     * @param btn2 the second button type
     * @param defVal optionally, set which button is active (0 based).
     */
    void setButtons(ButtonType btn1, ButtonType btn2, int defVal = 0);

    /**
     * Create a dialog that takes over the display and presents the header and
     * buffer, the header text is in program memory, with the buttons set up as per `setButtons`
     */
    void show(const char* headerPgm, bool allowRemote, CompletedHandlerFn completedHandler = NULL);

    /**
     * Create a dialog that takes over the display and presents the header and
     * buffer, the header text is in RAM, with the buttons set up as per `setButtons`
     */
    void showRam(const char* headerRam, bool allowRemote, CompletedHandlerFn completedHandler = NULL);

    /**
     * Create a dialog that takes over the display and presents the header and
     * buffer, the header text is in program memory, with the buttons set up as per `setButtons`
     *
     * A valid controller object pointer is mandatory.
     */
    void showController(bool allowRemote, BaseDialogController* controller);

    /**
     * You can set an item of data that will be passed to the callback when executed.
     * @param data a pointer to your own data that will be given back to you in the call back.
     */
    void setUserData(void *data) { this->userData = data; }

    /**
     * Copy text into the buffer, that will be displayed along with the header.
     * @param sz the text to copy
     */
    virtual void copyIntoBuffer(const char* sz);

    /** overriden in other types to handle show and hide differently */
    virtual void internalSetVisible(bool visible);

    virtual char* getBufferData();

    /**
     * Close the current dialog by taking off the display and clearing the inuse flag.
     */
    void hide();

    /** Indicates that the registered completion / callback is a controller */
    bool isUsingOOController() { return bitRead(flags, DLG_FLAG_USING_OO_CONTROLLER); }

    /** Indicates that this dialog is presently in use */
    bool isInUse() {return bitRead(flags, DLG_FLAG_INUSE);}

    /** Indicates that this menu is on a very limited display size */
    bool isCompressedMode() {return bitRead(flags, DLG_FLAG_SMALLDISPLAY);}

    /**
     * Called by the renderer to draw our dialog, in a game loop style as per all tcMenu
     * rendering.
     * @param currentValue the value of the encoder used to handle which button is active.
     * @param userClicked indicates when the user clicks an item.
     */
    void dialogRendering(unsigned int currentValue, bool userClicked);

    bool isMenuItemBased() { return bitRead(flags, DLG_FLAG_MENUITEM_BASED); }
    bool isRenderNeeded() { return bitRead(flags, DLG_FLAG_NEEDS_RENDERING); }
    bool isRemoteUpdateNeeded(int remote) { return bitRead(flags, remote + DLG_FLAG_REMOTE_0) && bitRead(flags, DLG_FLAG_CAN_SEND_REMOTE); }
    void setRemoteUpdateNeeded(int remote, bool b) { bitWrite(flags, remote + DLG_FLAG_REMOTE_0, b); }
    void setRemoteUpdateNeededAll() { flags |= DLG_FLAG_REMOTE_MASK; }
    void clearRemoteUpdateNeededAll() { flags &= ~DLG_FLAG_REMOTE_MASK; }
    void setRemoteAllowed(bool allowed) {bitWrite(flags, DLG_FLAG_CAN_SEND_REMOTE, allowed); }

    void encodeMessage(TagValueRemoteConnector* remote);
    void remoteAction(ButtonType type);

    /**
     * Copies button text into the buffer provided by data, based on the button
     * number (0 or 1) and the current value of the encoder.
     * @param data the buffer to copy into, must fit at least the largest button copyButtonText
     * @param buttonNum the button number either 0, 1
     * @param currentValue current value from encoder.
     * @return true if this button is active, otherwise false.
     */
    bool copyButtonText(char* data, int buttonNum, int currentValue, bool isActive);

    bool copyButtonText(char* data, int buttonNum, int currentValue) {
        return copyButtonText(data, buttonNum, currentValue, buttonNum == currentValue);
    }

    /**
     * Perform the action of the button numbered in the parameter. For example if button 1 is set to OK, then OK is
     * pressed.
     * @param btnNum
     */
    void actionPerformed(int btnNum);
protected:
    /**
     * Sets the inuse flag to true or false
     */
    void setInUse(bool b) {bitWrite(flags, DLG_FLAG_INUSE, b);}

    /**
     * Needs to be overriden by each leaf renderer implementation in order to draw the actual
     * dialog onto the display.
     * @param currentValue the encoder currentValue
     */
    virtual void internalRender(int currentValue)=0;

    /**
     * Set if redraw is needed
     * @param b true for redraw needed
     */
    void setNeedsDrawing(bool b) {
        if(b && needsDrawing == MENUDRAW_COMPLETE_REDRAW) return;
        needsDrawing = b ? MENUDRAW_EDITOR_CHANGE : MENUDRAW_NO_CHANGE;
    }

    /**
     * Finds the button that is currently selected
     * @param the current value of the encoder
     * @return the active button
     */
    ButtonType findActiveBtn(unsigned int currentValue);

    void internalShow(bool allowRemote);
};

/**
 * This is the render function that should be used with the LocalDialogButtonMenuItem
 * @param item
 * @param mode
 * @param buffer
 * @param bufferSize
 * @return
 */
int dialogButtonRenderFn(RuntimeMenuItem* item, uint8_t /*row*/, RenderFnMode mode, char* buffer, int bufferSize);

/**
 * This menu type is reserved only for use within dialogs, never use this button outside of that purpose. Button numbers
 * 0..15 are reserved and should never be used by application code. Use 15..255 in application code.
 */
class LocalDialogButtonMenuItem : public RuntimeMenuItem {
private:
    uint8_t buttonNumber;
public:
    /**
     * Create a extra local dialog button, never create on with a button number lower than 2 because 0 and 1 are
     * reserved. Ideally the button numbers should follow in sequence from 2 onwards.
     * @param renderFn nearly always set this to dialogButtonRenderFn
     * @param id the ID, if you don't care about this, set it to nextRandomId()
     * @param btnNum the button number
     * @param next the next menu item in the list
     */
    LocalDialogButtonMenuItem(RuntimeRenderingFn renderFn, int id, int btnNum, MenuItem* next)
            : RuntimeMenuItem(MENUTYPE_DIALOG_BUTTON, id, renderFn, 0, 1, next) {
        setLocalOnly(true);
        buttonNumber = btnNum;
    }
    int getButtonNumber() { return  buttonNumber; }
};

#define FIRST_USER_BUTTON_NUM 2

/**
 * This is an extended dialog based on MenuItem's that are presented to the user, you can add additional menu items that
 * go after the message text and before the first dialog button. there will always be either one or two buttons below
 * that content, as a means to dismiss the dialog. The user can also dismiss (equivalent of cancel) by selecting the
 * back menu item.
 */
class MenuBasedDialog : public BaseDialog {
private:
    BackMenuItem backItem;
    TextMenuItem bufferItem;
    LocalDialogButtonMenuItem btn1Item;
    LocalDialogButtonMenuItem btn2Item;
    int addedMenuItems;
public:
    MenuBasedDialog();
    ~MenuBasedDialog() override = default;

    uint16_t getBackMenuItemId() { return backItem.getId(); }

    void internalSetVisible(bool visible) override;
    void copyIntoBuffer(const char *sz) override;

    void insertMenuItem(MenuItem* item);

    void copyHeader(char *buffer, int bufferSize);

    char* getBufferData() override { return const_cast<char *>(bufferItem.getTextValue()); }

    TextMenuItem* getBufferMenuItem() { return &bufferItem; }

protected:
    /** not used in this implementation */
    void internalRender(int currentValue) override {}

    void resetDialogFields();
};

/** a callback method that is used alongside withMenuDialogIfAvailable(..) */
typedef void (*DialogInitialiser)(MenuBasedDialog*);

/**
 * A small helper that checks if we can use the dialog, and then calls the provided function if we can.
 * Note: only works with MenuBasedDialog
 *
 * @param dlgFn the function to be called with the dialog object pointer, if getting the dialog was successful.
 */
void withMenuDialogIfAvailable(DialogInitialiser dlgFn);

#endif //TCMENU_BASE_DIALOG_H_
