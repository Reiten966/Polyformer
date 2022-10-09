/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "tcUtil.h"
#include "RemoteMenuItem.h"
#include "BaseRenderers.h"
#include "BaseDialog.h"
#include "graphics/BaseGraphicalRenderer.h"

MenuRenderer* MenuRenderer::theInstance = nullptr;

class RenderingMenuMgrObserver : public MenuManagerObserver {
public:
    void structureHasChanged() override {
        auto myRenderer = MenuRenderer::getInstance();
        if(myRenderer->getRendererType() == RENDER_TYPE_CONFIGURABLE) {
            auto gfxRenderer = reinterpret_cast<tcgfx::BaseGraphicalRenderer*>(myRenderer);
            gfxRenderer->displayPropertiesHaveChanged();
        } else if(myRenderer->getRendererType() != RENDER_TYPE_NOLOCAL) {
            serlogF(SER_TCMENU_DEBUG, "Completely invalidate the display");
            reinterpret_cast<BaseMenuRenderer*>(myRenderer)->invalidateAll();
        }
    }

    bool menuEditStarting(MenuItem *item) override {
        return true;
    }

    void menuEditEnded(MenuItem *item) override {}
} menuMgrListener;


BaseMenuRenderer::BaseMenuRenderer(int bufferSize, RendererType rType) : MenuRenderer(rType, bufferSize) {
	ticksToReset = 0;
    lastOffset = 0;
    resetValInTicks = 30 * TC_DISPLAY_UPDATES_PER_SECOND;
	renderCallback = NULL;
    resetCallback = NULL;
	redrawMode = MENUDRAW_COMPLETE_REDRAW;
	this->lastOffset = 0;
    this->firstWidget = NULL;
    this->dialog = NULL;
    isCustomDrawing = false;
    displayTakenMode = NOT_TAKEN_OVER;
    updatesPerSecond = TC_DISPLAY_UPDATES_PER_SECOND;
    MenuRenderer::theInstance = this;
}

void BaseMenuRenderer::initialise() {
	ticksToReset = resetValInTicks;
	renderCallback = NULL;
	redrawMode = MENUDRAW_COMPLETE_REDRAW;

    menuMgr.changeMenu();

    if(updatesPerSecond == 0) updatesPerSecond = TC_DISPLAY_UPDATES_PER_SECOND;

    taskManager.scheduleOnce(250, this);
	menuMgr.addChangeNotification(&menuMgrListener);
}

bool BaseMenuRenderer::tryTakeSelectIfNeeded(int currentReading, RenderPressMode pressType) {
	// always set the menu as altered.
	menuAltered();

	if (displayTakenMode != NOT_TAKEN_OVER || (dialog != nullptr && dialog->isRenderNeeded()) ) {
		// When there's a dialog, or render function, just record the change until exec().
		renderFnPressType = pressType;
		return true;
	}

	// standard processing of the event required.
	return false;
}

void BaseMenuRenderer::exec() {

	if(dialog!=nullptr && dialog->isRenderNeeded()) {
		dialog->dialogRendering(menuMgr.getCurrentRangeValue(), renderFnPressType);
    } else if(displayTakenMode == NOT_TAKEN_OVER) {
        render();
    } else if(displayTakenMode == START_CUSTOM_DRAW) {
        customDrawing->started(this);
        displayTakenMode = RUNNING_CUSTOM_DRAW;
    } else if(displayTakenMode == RUNNING_CUSTOM_DRAW) {
        customDrawing->renderLoop(menuMgr.getCurrentRangeValue(), renderFnPressType);
    } else if(displayTakenMode == TAKEN_OVER_FN) {
	    renderCallback(menuMgr.getCurrentRangeValue(), renderFnPressType);
	}

    int refreshInterval = 1000 / updatesPerSecond;
    taskManager.scheduleOnce(refreshInterval, this);
}

void BaseMenuRenderer::resetToDefault() {
    serlogF2(SER_TCMENU_INFO, "Display reset - timeout ticks: ", resetValInTicks);
	menuMgr.resetMenu(true);
	ticksToReset = MAX_TICKS;

    // once the menu has been reset, if the reset callback is present
    // then we call it.
    if(resetCallback) {
        if(isCustomDrawing) {
            customDrawing->reset();
        }
        else {
            resetCallback();
        }
    };
}

void BaseMenuRenderer::countdownToDefaulting() {
    if(dialog != nullptr && dialog->isInUse()) {
        ticksToReset = resetValInTicks;
        return;
    }
	if (ticksToReset == 0) {
		resetToDefault();
		ticksToReset = MAX_TICKS;
	}
	else if (ticksToReset != MAX_TICKS) {
		--ticksToReset;
	}
}

void BaseMenuRenderer::takeOverDisplay(RendererCallbackFn displayFn) {
    if(displayFn == nullptr && isCustomDrawing == false) return;
	// when we set this, we are stopping tcMenu rendering and letting this take over
	renderFnPressType = RPRESS_NONE;
    displayTakenMode = displayFn ? TAKEN_OVER_FN : START_CUSTOM_DRAW;
	renderCallback = displayFn;
}

void BaseMenuRenderer::giveBackDisplay() {
	// clear off the rendering callback.
	renderFnPressType = RPRESS_NONE;
	renderCallback = nullptr;
	displayTakenMode = NOT_TAKEN_OVER;
	menuMgr.changeMenu();
	menuAltered();
}

void BaseMenuRenderer::setFirstWidget(TitleWidget* widget) {
	this->firstWidget = widget;
	this->redrawMode = MENUDRAW_COMPLETE_REDRAW;
}

int BaseMenuRenderer::findActiveItem(MenuItem *root) {
    uint8_t i = 0;
    MenuItem *itm = root;
    while (itm != nullptr) {
        if (itm->isVisible()) {
            if (itm->isActive() || itm->isEditing()) {
                return i;
            }
            i++;
        }
        itm = itm->getNext();
    }
    return 0;
}

uint8_t BaseMenuRenderer::itemCount(MenuItem* item, bool includeNonVisible) {
    uint8_t count = 0;
    while (item) {
        if (includeNonVisible || item->isVisible()) ++count;
        item = item->getNext();
    }
    return count;
}

MenuItem *BaseMenuRenderer::getMenuItemAtIndex(MenuItem *root, uint8_t pos) {
    uint8_t i = 0;
    MenuItem *itm = root;

    while (itm != nullptr) {
        if (itm->isVisible()) {
            if (i == pos) {
                return itm;
            }
            i++;
        }
        itm = itm->getNext();
    }
    return root;
}

TitleWidget::TitleWidget(const uint8_t * const* icons, uint8_t maxStateIcons, uint8_t width, uint8_t height, TitleWidget* next) {
	this->iconData = icons;
	this->maxStateIcons = maxStateIcons;
	this->width = width;
	this->height = height;
	this->currentState = 0;
	this->next = next;
	this->changed = true;
}

class NoRenderDialog : public BaseDialog {
public:
    NoRenderDialog() { 
        bitWrite(flags, DLG_FLAG_SMALLDISPLAY, false);
    }
protected:
    void internalRender(int currentValue) override { /* does nothing */ }
};

BaseDialog* NoRenderer::getDialog() {
    if(dialog == NULL) {
        dialog = new NoRenderDialog();
    }
    return dialog;
}

bool isItemActionable(MenuItem* item) {
	if (item->getMenuType() == MENUTYPE_SUB_VALUE || item->getMenuType() == MENUTYPE_ACTION_VALUE 
		|| item->getMenuType() == MENUTYPE_ACTIVATE_SUBMENU || item->getMenuType() == MENUTYPE_RUNTIME_VALUE
        || item->getMenuType() == MENUTYPE_TITLE_ITEM || item->getMenuType() == MENUTYPE_DIALOG_BUTTON) return true;

	if (item->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		return reinterpret_cast<ListRuntimeMenuItem*>(item)->isActingAsParent();
	}
	return false;
}
