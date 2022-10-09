/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "tcUtil.h"
#include "graphics/BaseGraphicalRenderer.h"
#include <BaseRenderers.h>
#include <RemoteConnector.h>
#include <BaseDialog.h>
#include <SwitchInput.h>

using namespace tcgfx;

const char buttonOK[] PROGMEM = "ok";
const char buttonCancel[] PROGMEM = "cancel";
const char buttonClose[] PROGMEM = "close";
const char buttonAccept[] PROGMEM = "accept";
const char* standardDialogButton[] = { buttonOK, buttonAccept, buttonCancel, buttonClose };

BaseDialog::BaseDialog() : header{0}, headerPgm(nullptr) {
    flags = 0;
    bitWrite(flags, DLG_FLAG_INUSE, false);
    button1 = button2 = BTNTYPE_NONE;
}

void BaseDialog::show(const char* headerPgm, bool allowRemote, CompletedHandlerFn completionCallback) {
    this->headerPgm = headerPgm;
    bitClear(flags, DLG_FLAG_USING_OO_CONTROLLER);
    this->completedHandler = completionCallback;
    safeProgCpy(this->header, headerPgm, sizeof(this->header));
    internalShow(allowRemote);
}

void BaseDialog::showRam(const char* headerRam, bool allowRemote, CompletedHandlerFn completionCallback) {
    this->headerPgm = nullptr;
    bitClear(flags, DLG_FLAG_USING_OO_CONTROLLER);
    this->completedHandler = completionCallback;
    strncpy(this->header, headerRam, sizeof(this->header));
    this->header[sizeof(this->header) - 1] = 0;
    internalShow(allowRemote);
}

void BaseDialog::showController(bool allowRemote, BaseDialogController* dialogController) {
    if(dialogController == nullptr) {
        serlogF(SER_TCMENU_INFO, "Dialog controller was null, not showing");
        return;
    }
    this->headerPgm = nullptr;
    bitSet(flags, DLG_FLAG_USING_OO_CONTROLLER);
    this->controller = dialogController;
    controller->initialiseAndGetHeader(this, header, sizeof(header));
    this->header[sizeof(header) - 1] = 0;
    internalShow(allowRemote);
}

void BaseDialog::internalShow(bool allowRemote) {
    serlogF(SER_TCMENU_INFO, "showing new dialog");
    setInUse(true);
    setRemoteAllowed(allowRemote);
    setRemoteUpdateNeededAll();
    internalSetVisible(true);
    needsDrawing = MENUDRAW_COMPLETE_REDRAW;

    // we must do this after control of the menu has been taken for the dialog, otherwise it may recursively get called
    // for list and actions where there is a risk that changing the encoder will call again.
    int noOfOptions = (button1 != BTNTYPE_NONE && button2 != BTNTYPE_NONE)  ? 1 : 0;
    if(switches.getEncoder() && !isMenuItemBased()) switches.getEncoder()->changePrecision(noOfOptions, lastBtnVal);
}

void BaseDialog::internalSetVisible(bool visible) {
    if(MenuRenderer::getInstance()->getRendererType() == RENDER_TYPE_BASE) {
        ((BaseMenuRenderer*)MenuRenderer::getInstance())->giveBackDisplay();
    }
    bitWrite(flags, DLG_FLAG_NEEDS_RENDERING, visible);
}

void BaseDialog::hide() {
    serlogF(SER_TCMENU_INFO, "hide() - give back display");
    setRemoteUpdateNeededAll();

    // stop the renderer from doing any more rendering, and tell it to reset the menu
    setInUse(false);
    internalSetVisible(false);

    // clear down all structures.
    button1 = button2 = BTNTYPE_NONE;
    setNeedsDrawing(false);
    header[0] = 0;
}

ButtonType BaseDialog::findActiveBtn(unsigned int currentValue) {
    ButtonType b = button2;
    if(currentValue==0) {
        b = (button1 == BTNTYPE_NONE) ? button2 : button1;
    }
    serlogF4(SER_TCMENU_DEBUG,"findActiveBtn: ", currentValue, button1, button2);
    return b;
}

void BaseDialog::actionPerformed(int btnNum) {
    bool canDismiss = true;
    if(controller && isUsingOOController()) canDismiss = controller->dialogButtonPressed(btnNum);

    if(canDismiss) {
        // must be done before hide, which resets the encoder
        ButtonType btn = findActiveBtn(btnNum);
        serlogF2(SER_TCMENU_INFO, "User clicked button: ", btn);
        hide();
        if (completedHandler) {
            if(isUsingOOController()) {
                controller->dialogDismissed(btn);
            } else {
                completedHandler(btn, userData);
            }
        }
    }
}

void BaseDialog::dialogRendering(unsigned int currentValue, bool userClicked) {
    if(currentValue != lastBtnVal) {
        setNeedsDrawing(true);
    }

    lastBtnVal = currentValue;

    if(userClicked) {
        actionPerformed((int)currentValue);
        return;
    }

    if(needsDrawing != MENUDRAW_NO_CHANGE) {
        internalRender((int)currentValue);
        setNeedsDrawing(false);
    }
}

bool BaseDialog::copyButtonText(char* data, int buttonNum, int currentValue, bool sel) {
    data[0]=0;
    if(controller && buttonNum > 1) {
        controller->copyCustomButtonText(buttonNum, data, 14);
    }
    else if(controller && ((buttonNum == 0 && button1 >= BTNTYPE_CUSTOM0) || (buttonNum == 1 && button2 >= BTNTYPE_CUSTOM0))) {
        controller->copyCustomButtonText(buttonNum, data, 14);
    }
    else {
        uint8_t bt = (buttonNum == 0) ? button1 : button2;
        if(bt < BTNTYPE_NONE) {
            strcpy_P(data, standardDialogButton[bt]);
        }
    }

    if(currentValue == -1) {
        if(data[0]) data[0] = toupper(data[0]);
    }
    else if((button1 == BTNTYPE_NONE || button2 == BTNTYPE_NONE) || sel) {
        while(*data) {
            *data = toupper(*data);
            ++data;
        }
        return true;
    }
    return false;
}

char* BaseDialog::getBufferData() {
    return MenuRenderer::getInstance()->getBuffer();
}

void BaseDialog::copyIntoBuffer(const char* sz) {
    if(isInUse()) {
        char* buffer = getBufferData();
        uint8_t bufferSize = MenuRenderer::getInstance()->getBufferSize();
        strncpy(buffer, sz, bufferSize);
        int l = strlen(buffer);

        if(!isCompressedMode()) {
            for(int i=l; i<bufferSize; i++) {
                buffer[i]=32;
            }
        }

        buffer[bufferSize]=0;
        setNeedsDrawing(true);
        setRemoteUpdateNeededAll();
        
    }
}

void BaseDialog::setButtons(ButtonType btn1, ButtonType btn2, int defVal) {
    serlogF3(SER_TCMENU_INFO, "Set buttons on dialog", btn1, btn2);
    button1 = btn1;
    button2 = btn2;
    lastBtnVal = defVal;
    setNeedsDrawing(true);
}

void BaseDialog::encodeMessage(TagValueRemoteConnector* remote) {
    remote->encodeDialogMsg(isInUse() ? DLG_VISIBLE : DLG_HIDDEN, button1, button2, header, getBufferData());
}

void BaseDialog::remoteAction(ButtonType btn) {
    serlogF2(SER_TCMENU_INFO, "Remote clicked button: ", btn);
    hide();
    if(completedHandler) {
        if(isUsingOOController())
            controller->dialogDismissed(btn);
        else
            completedHandler(btn, userData);
    }
}

//
// Menu based dialog
//

int dialogBackRenderFn(RuntimeMenuItem* item, uint8_t /*row*/, RenderFnMode mode, char* buffer, int bufferSize) {
    auto* renderer =  reinterpret_cast<BaseGraphicalRenderer*>(MenuRenderer::getInstance());
    auto* dlg = reinterpret_cast<MenuBasedDialog*>(renderer->getDialog());
    switch (mode) {
        case RENDERFN_INVOKE:
            dlg->remoteAction(BTNTYPE_CANCEL);
            return true;
        case RENDERFN_NAME:
            dlg->copyHeader(buffer, bufferSize);
            return true;
        case RENDERFN_EEPROM_POS:
            return -1;
        case RENDERFN_VALUE:
            buffer[0] = 0;
            return true;
        default: return false;
    }
}

int dialogButtonRenderFn(RuntimeMenuItem* item, uint8_t /*row*/, RenderFnMode mode, char* buffer, int bufferSize) {
    auto* dlg = reinterpret_cast<MenuBasedDialog*>(MenuRenderer::getInstance()->getDialog());
    auto* btnItem = reinterpret_cast<LocalDialogButtonMenuItem*>(item);
    switch (mode) {
        case RENDERFN_INVOKE:
            dlg->actionPerformed(btnItem->getButtonNumber());
            return true;
        case RENDERFN_NAME:
            dlg->copyButtonText(buffer, btnItem->getButtonNumber(), -1);
            return true;
        case RENDERFN_EEPROM_POS:
            return -1;
        case RENDERFN_VALUE:
            buffer[0] = 0;
            return true;
        default: return false;
    }
}

RENDERING_CALLBACK_NAME_INVOKE(dialogTextRenderFn, textItemRenderFn, "Msg", -1, NO_CALLBACK)

MenuBasedDialog::MenuBasedDialog() :
        backItem(dialogBackRenderFn, nullptr),
        bufferItem(dialogTextRenderFn, nextRandomId(), 20, nullptr),
        btn1Item(dialogButtonRenderFn, nextRandomId(), 0, nullptr),
        btn2Item(dialogButtonRenderFn, nextRandomId(), 1, nullptr) {
    flags = 0;
    addedMenuItems = 0;
    bitWrite(flags, DLG_FLAG_SMALLDISPLAY, false);
    bitWrite(flags, DLG_FLAG_MENUITEM_BASED, true);
    resetDialogFields();
}

void MenuBasedDialog::copyIntoBuffer(const char *sz) {
    bufferItem.setTextValue(sz);
}

void MenuBasedDialog::internalSetVisible(bool visible) {
    BaseDialog::internalSetVisible(visible);

    bitWrite(flags, DLG_FLAG_NEEDS_RENDERING, false);
    if(visible) {
        auto* renderer =  reinterpret_cast<BaseGraphicalRenderer*>(MenuRenderer::getInstance());
        auto& factory = static_cast<ItemDisplayPropertiesFactory&>(renderer->getDisplayPropertiesFactory());

        btn1Item.setVisible(button1 != BTNTYPE_NONE);
        btn2Item.setVisible(button2 != BTNTYPE_NONE);

        auto row = 2 + addedMenuItems;
        factory.addGridPosition(&btn1Item, GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_LEFT_NO_VALUE, 2, 1, row, 0));
        factory.addGridPosition(&btn2Item, GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_RIGHT_NO_VALUE, 2, 2, row, 0));
        factory.addGridPosition(&bufferItem, GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_LEFT_VALUE_ONLY, 1, 0));

        menuMgr.navigateToMenu(&backItem, &btn1Item, true);
    }
    else {
        resetDialogFields();
        menuMgr.resetMenu(false);
    }
}

void MenuBasedDialog::insertMenuItem(MenuItem* item) {
    if(!item) return;

    item->setNext(bufferItem.getNext());
    addedMenuItems++;
    bufferItem.setNext(item);
}

void MenuBasedDialog::copyHeader(char *buffer, int bufferSize) {
    strncpy(buffer, header, bufferSize);
    buffer[bufferSize - 1] = 0;
}

void MenuBasedDialog::resetDialogFields() {
    backItem.setNext(&bufferItem);
    bufferItem.setNext(&btn1Item);
    bufferItem.setReadOnly(true);
    btn1Item.setNext(&btn2Item);
    addedMenuItems = 0;

    backItem.setActive(false);
    bufferItem.setActive(false);
    btn1Item.setActive(false);
    btn2Item.setActive(false);
}

void withMenuDialogIfAvailable(DialogInitialiser dlgFn) {
    if(MenuRenderer::getInstance() == nullptr) return;
    BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();

    if(dlg && !dlg->isInUse() && dlg->isMenuItemBased()) {
        dlgFn((MenuBasedDialog*)dlg);
    }
}
