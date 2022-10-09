/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <BaseRenderers.h>
#include "DialogRuntimeEditor.h"

DialogMultiPartEditor* DialogMultiPartEditor::theInstance;

void onScrollingChanged(int id) {
    DialogMultiPartEditor::theInstance->scrollChanged();
}

void DialogMultiPartEditor::startEditing(MenuBasedDialog* dlg, EditableMultiPartMenuItem *item) {
    dialog = reinterpret_cast<MenuBasedDialog*>(dlg);
    menuItemBeingEdited = item;
    dlg->setButtons(BTNTYPE_OK, BTNTYPE_CUSTOM0);
    dlg->showController(false, this);
    menuItemBeingEdited->beginMultiEdit();
    dialogButtonPressed(SECOND_DEFAULT_BUTTON);
}

void DialogMultiPartEditor::dialogDismissed(ButtonType buttonType) {
    menuItemBeingEdited->stopMultiEdit();
    menuItemBeingEdited = nullptr;
    dialog = nullptr;
}

bool DialogMultiPartEditor::dialogButtonPressed(int buttonNum) {
    if(buttonNum == SECOND_DEFAULT_BUTTON) {
        auto range = menuItemBeingEdited->nextPart();
        if(range != 0) {
            scrollingInfo.maxValue = range;
            scrollingEditor.setCurrentValue(menuItemBeingEdited->getPartValueAsInt());
            scrollChanged();
            return false;
        }
    }
    return true;
}

void DialogMultiPartEditor::copyCustomButtonText(int buttonNumber, char *buffer, size_t bufferSize) {
    if(buttonNumber == 1) {
        strcpy(buffer, "Next");
    }
}

void DialogMultiPartEditor::initialiseAndGetHeader(BaseDialog* dlg, char *buffer, size_t bufferSize) {
    dialog->insertMenuItem(&scrollingEditor);
    strcpy(buffer, "Edit ");
    menuItemBeingEdited->copyNameToBuffer(buffer, strlen(buffer), bufferSize);
}

void DialogMultiPartEditor::scrollChanged() {
    menuItemBeingEdited->valueChanged(scrollingEditor.getCurrentValue());
    char sz[32];
    copyMenuItemValue(menuItemBeingEdited, sz, sizeof sz);

    int srcLoc = 0;
    int dstLoc = 0;
    while(sz[srcLoc] != 0) {
        dialog->getBufferMenuItem()->setCharValue(dstLoc++, sz[srcLoc]);
        if(srcLoc == menuItemBeingEdited->getItemPosition() - 1) {
            dialog->getBufferMenuItem()->setCharValue(dstLoc++, '|');
        }
        srcLoc++;
    }
    dialog->getBufferMenuItem()->setCharValue(dstLoc, 0);
}
