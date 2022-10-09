/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "ScrollChoiceMenuItem.h"
#include "MenuIterator.h"
#include "SecuredMenuPopup.h"
#include <IoAbstraction.h>
#include <BaseDialog.h>

MenuManager menuMgr;

void MenuManager::initForUpDownOk(MenuRenderer* renderer, MenuItem* root, pinid_t pinDown, pinid_t pinUp, pinid_t pinOk, int speed) {
	this->renderer = renderer;
	navigator.setRootItem(root);

	switches.addSwitch(pinOk, nullptr);
    switches.onRelease(pinOk, [](pinid_t /*key*/, bool held) { menuMgr.onMenuSelect(held); });
	setupUpDownButtonEncoder(pinUp, pinDown, [](int value) {menuMgr.valueChanged(value); }, speed);
	renderer->initialise();
}

void MenuManager::initForEncoder(MenuRenderer* renderer,  MenuItem* root, pinid_t encoderPinA, pinid_t encoderPinB, pinid_t encoderButton, EncoderType type) {
	this->renderer = renderer;
    navigator.setRootItem(root);

	switches.addSwitch(encoderButton, nullptr);
    switches.onRelease(encoderButton, [](pinid_t /*key*/, bool held) { menuMgr.onMenuSelect(held); });
	setupRotaryEncoderWithInterrupt(encoderPinA, encoderPinB, [](int value) {menuMgr.valueChanged(value); }, HWACCEL_REGULAR, type);

	renderer->initialise();
}

void MenuManager::setBackButton(pinid_t backButtonPin) {
    switches.addSwitch(backButtonPin, [](pinid_t, bool held){
        if(!held) menuMgr.performDirectionMove(true);
    });
}

void MenuManager::setNextButton(pinid_t nextButtonPin) {
    switches.addSwitch(nextButtonPin, [](pinid_t, bool held){
        if(!held) menuMgr.performDirectionMove(false);
    });    
}

void MenuManager::performDirectionMove(bool dirIsBack) {
    if(currentEditor != nullptr && isMenuRuntimeMultiEdit(currentEditor)) {
        auto editableItem = reinterpret_cast<EditableMultiPartMenuItem*>(currentEditor);
		
        int editorRange = dirIsBack ? editableItem->previousPart() : editableItem->nextPart();
		if (editorRange != 0) {
			switches.changeEncoderPrecision(0, editorRange, editableItem->getPartValueAsInt(),
                                            isWrapAroundEncoder(editableItem));
		}
        else {
            stopEditingCurrentItem(false);
        }
    }
    else if(currentEditor != nullptr) {
        stopEditingCurrentItem(false);
    }
    else if(currentEditor == nullptr && dirIsBack) {
        getParentAndReset();
        resetMenu(false);
    }
    else if(currentEditor == nullptr && !dirIsBack) {
        MenuItem* currentActive = findCurrentActive();
        if(currentActive != nullptr && currentActive->getMenuType() == MENUTYPE_SUB_VALUE) {
            actionOnSubMenu(currentActive);
        }
    }
}

void MenuManager::initWithoutInput(MenuRenderer* renderer, MenuItem* root) {
	this->renderer = renderer;
    navigator.setRootItem(root);
	renderer->initialise();
}

/**
 * Called when the rotary encoder value has changed, if we are editing this changes the value in the current editor, if we are
 * showing menu items, it changes the index of the active item (renderer will move into display if needed).
 */
void MenuManager::valueChanged(int value) {
	if (renderer->tryTakeSelectIfNeeded(value, RPRESS_NONE)) return;

	if (currentEditor && isMenuBasedOnValueItem(currentEditor)) {
		((ValueMenuItem*)currentEditor)->setCurrentValue(value);
	}
	else if (currentEditor && isMenuRuntimeMultiEdit(currentEditor)) {
		reinterpret_cast<EditableMultiPartMenuItem*>(currentEditor)->valueChanged(value);
	}
	else if(currentEditor && currentEditor->getMenuType() == MENUTYPE_SCROLLER_VALUE) {
	    reinterpret_cast<ScrollChoiceMenuItem*>(currentEditor)->setCurrentValue(value);
	}
    else if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu())->setActiveIndex(value);
    }
	else {
        MenuItem* currentActive = menuMgr.findCurrentActive();
        currentActive->setActive(false);
        if(renderer->getRendererType() != RENDER_TYPE_NOLOCAL) {
            serlogF2(SER_TCMENU_DEBUG, "activate item ", value);
            currentActive = reinterpret_cast<BaseMenuRenderer*>(renderer)->getMenuItemAtIndex(getCurrentMenu(), value);
            if(currentActive) {
                currentActive->setActive(true);
                serlogF3(SER_TCMENU_DEBUG, "Change active (V, ID) ", value, currentActive->getId());
            }
        }
	}
}

/**
 * Called when the button on the encoder (OK button) is pressed. Most of this is left to the renderer to decide.
 */
void MenuManager::onMenuSelect(bool held) {
	if (renderer->tryTakeSelectIfNeeded(0, held ? RPRESS_HELD : RPRESS_PRESSED)) return;

	if (held) {
        if (currentEditor != nullptr && isMenuRuntimeMultiEdit(currentEditor)) {
            changeMenu();
        }
        else {
            resetMenu(true);
        }
    }
	else if (getCurrentEditor() != nullptr) {
		stopEditingCurrentItem(true);
	}
	else  {
		MenuItem* toEdit = findCurrentActive();
		actionOnCurrentItem(toEdit);
	}
}

void MenuManager::actionOnSubMenu(MenuItem* nextSub) {
	SubMenuItem* subMenu = reinterpret_cast<SubMenuItem*>(nextSub);
	if (subMenu->isSecured() && authenticationManager != nullptr) {
		serlogF2(SER_TCMENU_INFO, "Submenu is secured: ", nextSub->getId());
		SecuredMenuPopup* popup = secureMenuInstance();
		popup->start(subMenu);
		navigateToMenu(popup->getRootItem(), popup->getItemToActivate(), true);
	}
	else {
		navigateToMenu(subMenu->getChild());
	}
}

void MenuManager::actionOnCurrentItem(MenuItem* toEdit) {
	auto* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);

    if(!notifyEditStarting(toEdit)) return;

	// if there's a new item specified in toEdit, it means we need to change
	// the current editor (if it's possible to edit that value)
	if (toEdit->getMenuType() == MENUTYPE_SUB_VALUE) {
        if(toEdit != getCurrentMenu()) notifyEditEnd(getCurrentMenu());
		actionOnSubMenu(toEdit);
        return;
	}

	if (toEdit->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		if (menuMgr.getCurrentMenu() == toEdit) {
			auto* listItem = reinterpret_cast<ListRuntimeMenuItem*>(toEdit);
            serlogF2(SER_TCMENU_INFO, "List select: ", listItem->getActiveIndex());
			if (listItem->getActiveIndex() == 0) {
				resetMenu(false);
			}
			else {
				listItem->getChildItem(listItem->getActiveIndex() - 1)->triggerCallback();
				// reset to parent after doing the callback
				listItem->asParent();
			}
		}
		else navigateToMenu(toEdit);
	}
	else if (toEdit->getMenuType() == MENUTYPE_BACK_VALUE) {
	    toEdit->triggerCallback();
		toEdit->setActive(false);
		resetMenu(false);
	}
	else if (isItemActionable(toEdit)) {
        serlogF2(SER_TCMENU_INFO, "Callback trigger ", toEdit->getId());
		toEdit->triggerCallback();
	}
	else {
        serlogF2(SER_TCMENU_INFO, "Edit start ", toEdit->getId());
		menuMgr.setupForEditing(toEdit);
		baseRenderer->redrawRequirement(MENUDRAW_EDITOR_CHANGE);
	}
}

void MenuManager::stopEditingCurrentItem(bool doMultiPartNext) {

	if (doMultiPartNext && isMenuRuntimeMultiEdit(menuMgr.getCurrentEditor())) {
		auto* editableItem = reinterpret_cast<EditableMultiPartMenuItem*>(menuMgr.getCurrentEditor());

		// unless we've run out of parts to edit, stay in edit mode, moving to next part.
		int editorRange = editableItem->nextPart();
		if (editorRange != 0) {
			switches.changeEncoderPrecision(0, editorRange, editableItem->getPartValueAsInt(),
                                            isWrapAroundEncoder(editableItem));
			return;
		}
	}

	currentEditor->setEditing(false);

    notifyEditEnd(currentEditor);
	
    currentEditor = nullptr;
    renderingHints.changeEditingParams(CurrentEditorRenderingHints::EDITOR_REGULAR, 0, 0);
	setItemsInCurrentMenu(itemCount(menuMgr.getCurrentMenu()) - 1, offsetOfCurrentActive(menuMgr.getCurrentMenu()));

	if (renderer->getRendererType() != RENDER_TYPE_NOLOCAL) {
		auto* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);
		baseRenderer->redrawRequirement(MENUDRAW_EDITOR_CHANGE);
	}
}

MenuItem* MenuManager::getParentAndReset() {
    if(menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        auto* sub = getSubMenuFor(menuMgr.getCurrentMenu());
        if(sub) return reinterpret_cast<SubMenuItem*>(sub)->getChild();
    }

    if(menuMgr.getRenderer()->getRendererType() == RENDER_TYPE_CONFIGURABLE && navigator.isShowingRoot()) {
        auto* titleItem = reinterpret_cast<BaseMenuRenderer*>(menuMgr.getRenderer())->getMenuItemAtIndex(getCurrentMenu(), 0);
        if(titleItem) titleItem->setActive(false);
    }

	auto* pItem = getParentRootAndVisit(menuMgr.getCurrentMenu(), [](MenuItem* curr) {
		curr->setActive(false);
		curr->setEditing(false);
	});

	if(pItem == nullptr) pItem = menuMgr.getRoot();
	return pItem;
}

bool MenuManager::activateMenuItem(MenuItem *item) {
    if(renderer->getRendererType() == RENDER_TYPE_NOLOCAL) return false;
    auto* r = reinterpret_cast<BaseMenuRenderer*>(renderer);
    uint8_t count = r->itemCount(getCurrentMenu(), false);
    for(int i=0; i < count; i++) {
        auto* pItem = r->getMenuItemAtIndex(getCurrentMenu(), i);
        if(pItem != nullptr && pItem->getId() == item->getId()) {
            valueChanged(i);
            return true;
        }
    }
    return false;
}

/**
 * Finds teh currently active menu item with the selected SubMenuItem
 */
MenuItem* MenuManager::findCurrentActive() {
	MenuItem* itm = navigator.getCurrentRoot();
	while (itm != nullptr) {
		if (itm->isActive()) {
			return itm;
		}
		itm = itm->getNext();
	}

	// there's a special case for the title menu on the main page that needs to be checked against.
	if(renderer->getRendererType() == RENDER_TYPE_CONFIGURABLE) {
	    auto* pItem = reinterpret_cast<BaseMenuRenderer*>(renderer)->getMenuItemAtIndex(getCurrentMenu(), 0);
	    if(pItem && pItem->isActive()) return pItem;
	}

	return getCurrentMenu();
}

void MenuManager::setupForEditing(MenuItem* item) {
	// if the item is NULL, or it's read only, then it can't be edited.
	if (item == nullptr || item->isReadOnly()) return;

	MenuType ty = item->getMenuType();
	if ((ty == MENUTYPE_ENUM_VALUE || ty == MENUTYPE_INT_VALUE)) {
		// these are the only types we can edit with a rotary encoder & LCD.
		setCurrentEditor(item);
        int step = (ty == MENUTYPE_INT_VALUE) ? reinterpret_cast<AnalogMenuItem*>(item)->getStep() : 1;
		switches.changeEncoderPrecision(0, item->getMaximumValue(), reinterpret_cast<ValueMenuItem*>(currentEditor)->getCurrentValue(),
                                        isWrapAroundEncoder(currentEditor), step);
		if(switches.getEncoder()) switches.getEncoder()->setUserIntention(CHANGE_VALUE);
	}
	else if (ty == MENUTYPE_BOOLEAN_VALUE) {
		// we don't actually edit boolean items, just toggle them instead
        auto* boolItem = (BooleanMenuItem*)item;
		boolItem->setBoolean(!boolItem->getBoolean());
		notifyEditEnd(item);
	}
	else if (ty == MENUTYPE_SCROLLER_VALUE) {
        setCurrentEditor(item);
	    switches.changeEncoderPrecision(0, item->getMaximumValue(), reinterpret_cast<ScrollChoiceMenuItem*>(item)->getCurrentValue(),
                                        isWrapAroundEncoder(currentEditor));
	}
	else if (isMenuRuntimeMultiEdit(item)) {
        setCurrentEditor(item);
        auto* editableItem = reinterpret_cast<EditableMultiPartMenuItem*>(item);
        editableItem->beginMultiEdit();
        int range = editableItem->nextPart();
        switches.changeEncoderPrecision(0, range, editableItem->getPartValueAsInt(), editableItem->getId());
        switches.getEncoder()->setUserIntention(CHANGE_VALUE);
    }
}

void MenuManager::setCurrentEditor(MenuItem * editor) {
	if (currentEditor != nullptr) {
		currentEditor->setEditing(false);
		currentEditor->setActive(editor == nullptr);
	}
	currentEditor = editor;

    if(currentEditor != nullptr) {
        currentEditor->setEditing(true);
    }

    renderingHints.changeEditingParams(CurrentEditorRenderingHints::EDITOR_REGULAR, 0, 0);
}

void MenuManager::changeMenu(MenuItem* possibleActive) {
    if (renderer->getRendererType() == RENDER_TYPE_NOLOCAL) return;

    serlogF2(SER_TCMENU_DEBUG, "changeMenu: ", navigator.getCurrentRoot()->getId());

    // clear the current editor and ensure all active / editing flags removed.
	menuMgr.setCurrentEditor(nullptr);
    getParentAndReset();

	// now we set up the encoder to represent the right value and mark an item as active.
    if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        auto* listMenu = reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu());
        listMenu->setActiveIndex(0);
        menuMgr.setItemsInCurrentMenu(listMenu->getNumberOfRows());
    } else {
        auto* toActivate = (possibleActive) ? possibleActive : navigator.getCurrentRoot();
        toActivate->setActive(true);
        setItemsInCurrentMenu(itemCount(navigator.getCurrentRoot(), false) - 1, offsetOfCurrentActive(navigator.getCurrentRoot()));
    }

    // lastly force a redraw.
    reinterpret_cast<BaseMenuRenderer*>(renderer)->redrawRequirement(MENUDRAW_COMPLETE_REDRAW);
}

SecuredMenuPopup* MenuManager::secureMenuInstance() {
	if (securedMenuPopup == nullptr) securedMenuPopup = new SecuredMenuPopup(authenticationManager);
	return securedMenuPopup;
}

MenuManager::MenuManager() : navigator(), structureNotifier() {
    this->currentEditor = nullptr;
    this->renderer = nullptr;
    this->securedMenuPopup = nullptr;
    this->authenticationManager = nullptr;
    this->eepromRef = nullptr;
}

void MenuManager::addMenuAfter(MenuItem *existing, MenuItem* toAdd, bool silent) {
    MenuItem* endOfAddedList = toAdd;
    while(endOfAddedList->getNext() != nullptr) {
        endOfAddedList = endOfAddedList->getNext();
    }
    endOfAddedList->setNext(existing->getNext());
    existing->setNext(toAdd);
    if(!silent) notifyStructureChanged();
}

void MenuManager::addChangeNotification(MenuManagerObserver *observer) {
    for(auto & i : structureNotifier) {
        if(i == nullptr) {
            i = observer;
            return;
        }
    }
}

void MenuManager::load(uint16_t magicKey, TimerFn onEepromEmpty) {
    if(!loadMenuStructure(eepromRef, magicKey) && onEepromEmpty != nullptr) {
        serlogF(SER_TCMENU_INFO, "Run EEPROM empty cb");
        onEepromEmpty();
    }
}

void MenuManager::load(EepromAbstraction &eeprom, uint16_t magicKey, TimerFn onEepromEmpty) {
    eepromRef = &eeprom;
    if(!loadMenuStructure(&eeprom, magicKey) && onEepromEmpty != nullptr) {
        serlogF(SER_TCMENU_INFO, "Run EEPROM empty cb");
        onEepromEmpty();
    }
}

void MenuManager::notifyEditEnd(MenuItem *item) {
    if(item == nullptr) return; // don't notify a null pointer
    for(auto & obs : structureNotifier) {
        if(obs != nullptr) {
            obs->menuEditEnded(item);
        }
    }
    serlogF2(SER_TCMENU_INFO, "Menu edit end ", item->getId());
}

bool MenuManager::notifyEditStarting(MenuItem *item) {
    if(item == nullptr) return true; // don't notify a null pointer and allow menu to proceed.

    bool goAhead = true;
    for(auto & obs : structureNotifier) {
        if(obs != nullptr) {
            goAhead = goAhead && obs->menuEditStarting(item);
        }
    }
    if(!goAhead) {
        serlogF2(SER_TCMENU_INFO, "Edit start cancelled ", item->getId());
    }
    return goAhead;
}

void MenuManager::notifyStructureChanged() {
    serlogF(SER_TCMENU_INFO, "Menu structure change");
    for(auto & i : structureNotifier) {
        if(i != nullptr) {
            i->structureHasChanged();
        }
    }
}

void MenuManager::setItemsInCurrentMenu(int size, int offs) {
    auto enc = switches.getEncoder();
    if(!enc) return;
    serlogF3(SER_TCMENU_INFO, "Set items in menu (size, offs) ", size, offs);
    enc->changePrecision(size, offs, useWrapAroundByDefault);
    enc->setUserIntention(SCROLL_THROUGH_ITEMS);
}

void MenuManager::resetMenu(bool completeReset) {
    // we cannot reset the menu while a dialog is currently shown.
    if(renderer->getDialog() && renderer->getDialog()->isInUse()) return;

    MenuItem* currentActive;
    if(completeReset) {
        navigator.setRootItem(navigator.getRoot());
        currentActive = nullptr;
    } else {
        currentActive = navigator.popNavigationGetActive();
    }
    changeMenu(currentActive);
}

void MenuManager::navigateToMenu(MenuItem* theNewItem, MenuItem* possibleActive, bool customMenu) {
    navigator.navigateTo(findCurrentActive(), theNewItem, customMenu);
    changeMenu(possibleActive);
}

void MenuManager::addEncoderWrapOverride(MenuItem &item, bool override) {
    encoderWrapOverrides.add(EncoderWrapOverride(item.getId(), override));
}

bool MenuManager::isWrapAroundEncoder(MenuItem* menuItem) {
    for(EncoderWrapOverride& item : encoderWrapOverrides) {
        if(item.getMenuId() == menuItem->getId())  return item.getOverrideValue();
    }
    return useWrapAroundByDefault;
}

void MenuManager::majorOrderChangeApplied(int newMax) {
    if(renderer->getRendererType() == RENDER_TYPE_CONFIGURABLE && getCurrentMenu()->getMenuType() != MENUTYPE_RUNTIME_LIST) {
        setItemsInCurrentMenu(newMax);
    }
}

void MenuManager::setEditorHints(CurrentEditorRenderingHints::EditorRenderingType hint, size_t start, size_t end) {
    renderingHints.changeEditingParams(hint, start, end);
    serlogF4(SER_TCMENU_DEBUG, "SetEditorHints ", hint, start, end);
}

void CurrentEditorRenderingHints::changeEditingParams(CurrentEditorRenderingHints::EditorRenderingType ty, int startOffset, int endOffset) {
    renderingType = ty;
    editStart = startOffset;
    editEnd = endOffset;
}
