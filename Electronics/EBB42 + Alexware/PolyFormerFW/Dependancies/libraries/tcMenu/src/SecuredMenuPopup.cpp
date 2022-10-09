/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "SecuredMenuPopup.h"
#include "tcMenu.h"
#include "BaseDialog.h"

const char pgmProceedText[] PROGMEM = "Proceed";
const char pgmCancelText[] PROGMEM = "Cancel";
const char pgmHeaderNotAuth[] PROGMEM = "Pin incorrect";

RENDERING_CALLBACK_NAME_INVOKE(fnpopupPasswordRtCall, textItemRenderFn, "Password", -1, NULL)
RENDERING_CALLBACK_NAME_INVOKE(fnpopupSubmenuSecured, backSubItemRenderFn, "Enter PIN", -1, NULL)

SecuredMenuPopup::SecuredMenuPopup(AuthenticationManager * authentication) 
	: backMenuItem(fnpopupSubmenuSecured, &pinEntryItem),
	  pinEntryItem(fnpopupPasswordRtCall, nextRandomId(), MAX_PIN_LENGTH, &actionProceedItem),
	  actionProceedItem(secPopupActionRenderFn, 1, &actionCancelItem),
	  actionCancelItem(secPopupActionRenderFn, 0, NULL) {

	this->authentication = authentication;
}

MenuItem* SecuredMenuPopup::start(SubMenuItem* securedMenu) {
	actionProceedItem.setSecuredItem(securedMenu);
	actionProceedItem.setActive(false);
	actionCancelItem.setSecuredItem(NULL);
	actionCancelItem.setActive(false);
	backMenuItem.setActive(false);

	pinEntryItem.setTextValue("", true);
	pinEntryItem.setPasswordField(true);
	pinEntryItem.setActive(false);
	pinEntryItem.setEditing(false);
	return &backMenuItem;
}

int secPopupActionRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	if (item->getMenuType() != MENUTYPE_ACTIVATE_SUBMENU) return false;
	ActivateSubMenuItem* act = reinterpret_cast<ActivateSubMenuItem*>(item);

	switch (mode) {
	case RENDERFN_VALUE: 
		buffer[0] = 0;
		return true;
	case RENDERFN_NAME:
		strncpy_P(buffer, (row != 0) ? pgmProceedText : pgmCancelText, bufferSize);
		return true;
	case RENDERFN_EEPROM_POS:
		return -1;
	case RENDERFN_INVOKE:
		if(act->getSecuredItem() != NULL) {
			if (menuMgr.secureMenuInstance()->doesPinMatch()) {
				menuMgr.navigateToMenu(act->getSecuredItem()->getChild());
			}
			else {
                menuMgr.resetMenu(false);
                BaseDialog* dlg = menuMgr.getRenderer()->getDialog();
				dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
				char sz[15];
				act->getSecuredItem()->copyNameToBuffer(sz, sizeof(sz));
				dlg->show(pgmHeaderNotAuth, false);
				dlg->copyIntoBuffer(sz);
			}
		}
		else {
			menuMgr.resetMenu(false);
		}
		return true;
	default:
		return false;
	}

}
