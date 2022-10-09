#ifndef TCMENU_ESPAMPLIFIER_TESTINGDIALOGCONTROLLER_H
#define TCMENU_ESPAMPLIFIER_TESTINGDIALOGCONTROLLER_H

#include <BaseDialog.h>

class TestingDialogController : public BaseDialogController {
private:
    MenuBasedDialog* theDialog;
public:
    void initialiseAndGetHeader(BaseDialog *dialog, char *buffer, size_t bufferSize) override {
        theDialog = reinterpret_cast<MenuBasedDialog *>(dialog);
        strcpy(buffer, "Test Dialog");
    }

    void dialogDismissed(ButtonType buttonType) override {
        if(buttonType == BTNTYPE_OK) {
            theDialog->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
            theDialog->showRam("Extra dlg", true);
            theDialog->copyIntoBuffer("more text");
        }
    }

    bool dialogButtonPressed(int buttonNum) override {
        return true;
    }

    void copyCustomButtonText(int buttonNumber, char *buffer, size_t bufferSize) override {
        buffer[0] = 0;
    }
} testingDialogController;

inline void showDialogs() {
    auto *dlg = renderer.getDialog();
    if (dlg && !dlg->isInUse()) {
        dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL);
        dlg->showController(true, &testingDialogController);
        dlg->copyIntoBuffer("some more text");
    }
}

#endif //TCMENU_ESPAMPLIFIER_TESTINGDIALOGCONTROLLER_H
