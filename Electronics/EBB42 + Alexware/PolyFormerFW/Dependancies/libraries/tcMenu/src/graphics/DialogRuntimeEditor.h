/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_DIALOGRUNTIMEEDITOR_H
#define TCMENU_DIALOGRUNTIMEEDITOR_H

#include "../MenuItems.h"
#include "../RuntimeMenuItem.h"
#include "../BaseDialog.h"

/**
 * @file DialogRuntimeEditor.h
 * Contains the functionality to handle editing runtime multipart items
 */


void onScrollingChanged(int id);

class DialogMultiPartEditor : BaseDialogController {
private:
    MenuBasedDialog *dialog;
    EditableMultiPartMenuItem* menuItemBeingEdited;
    AnalogMenuInfo scrollingInfo = {"Item Value", nextRandomId(), 0xffff, 1, onScrollingChanged, 0, 1, "" };
    AnalogMenuItem scrollingEditor = AnalogMenuItem(&scrollingInfo, 0, nullptr, INFO_LOCATION_RAM);

public:
    static DialogMultiPartEditor* theInstance;

    DialogMultiPartEditor() {
        theInstance = this;
        menuItemBeingEdited = nullptr;
        dialog = nullptr;
    };

    void startEditing(MenuBasedDialog* dlg, EditableMultiPartMenuItem* item);

    void scrollChanged();

    void dialogDismissed(ButtonType buttonType) override;

    bool dialogButtonPressed(int buttonNum) override;

    void copyCustomButtonText(int buttonNumber, char *buffer, size_t bufferSize) override;

    void initialiseAndGetHeader(BaseDialog* dialog, char* buffer, size_t bufferSize) override;
};

#endif //TCMENU_DIALOGRUNTIMEEDITOR_H
