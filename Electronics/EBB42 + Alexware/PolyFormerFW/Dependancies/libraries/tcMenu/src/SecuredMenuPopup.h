/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _SECURED_MENU_POPUP_H_
#define _SECURED_MENU_POPUP_H_

/**
 * @file SecuredMenuPopup.h contains the code to show a security screen for secured menus.
 */

#include <RuntimeMenuItem.h>
#include <RemoteAuthentication.h>

int secPopupActionRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

class ActivateSubMenuItem : public RuntimeMenuItem {
private:
	SubMenuItem* securedItem;
public:
	ActivateSubMenuItem(RuntimeRenderingFn customRenderFn, int activeStatus, MenuItem* next = NULL)
		: RuntimeMenuItem(MENUTYPE_ACTIVATE_SUBMENU, nextRandomId(), customRenderFn, activeStatus, 1, next) {
	}

	void setSecuredItem(SubMenuItem *secured) {
		securedItem = secured;
	}

	SubMenuItem* getSecuredItem() { return securedItem; }
};

/**
 * Secured menu popup is a detatched menu that is never presented remotely, it allows
 * for secured menus that can only be edited on the device after entering a pin.
 * It has a pin entry area, the ability to proceed (if the pin is correct) and the
 * ability to go back to the main menu.
 */
class SecuredMenuPopup {
private:
    BackMenuItem backMenuItem;
	TextMenuItem pinEntryItem;
	ActivateSubMenuItem actionProceedItem;
	ActivateSubMenuItem actionCancelItem;
	AuthenticationManager* authentication;
public:
	SecuredMenuPopup(AuthenticationManager *authentication);

	MenuItem* getItemToActivate() { return &pinEntryItem; }

	MenuItem* start(SubMenuItem* securedMenu);

	MenuItem* getRootItem() {
		return &backMenuItem;
	}

	bool doesPinMatch() {
		return authentication->doesPinMatch(pinEntryItem.getTextValue());
	}
};

#endif // _SECURED_MENU_POPUP_H_
