// ADVANCED: here we show how to add additional callbacks when you've declared callbacks using the advanced
// @functionName syntax in the onChange field in the designer UI.
//
// In this case only the header entries are created for you leaving you to create the functions whereever you wish.
// The usual way to proceed here is to get the header definitions from the projectName_menu.h

#include "esp32Amplifier_menu.h"

void CALLBACK_FUNCTION valveHeatingChanged(int id) {
    serdebugF("valve heating callback called");
}

void CALLBACK_FUNCTION warmUpChanged(int id) {
    serdebugF("warm up callback called");
}
