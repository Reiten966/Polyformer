#ifndef BASE_DIALOG_TESTS_H
#define BASE_DIALOG_TESTS_H

#include <AUnit.h>
#include <tcMenu.h>
#include <BaseDialog.h>
#include <RemoteTypes.h>
#include <RemoteConnector.h>

//
// test implementation of the dialog, just counts up the number of render calls.
// and gives access to the button text.
//
class DialogTestImpl : public BaseDialog {
private:
	int renderCount;
	int valueWhenRenderered;
	char szTemp[20];
public:
	DialogTestImpl(bool compressed) {
		bitWrite(flags, DLG_FLAG_SMALLDISPLAY, compressed);
		reset();
	}

	void reset() {
		renderCount = 0;
		valueWhenRenderered = -1;
	}

	void internalRender(int currentValue) override {
		renderCount++;
		valueWhenRenderered = currentValue;
	}

	int getValueWhenRenderered() { return valueWhenRenderered; }
	int getRenderCount() { return renderCount; }
	const char* getButtonText(int btn) {
	    copyButtonText(szTemp, btn, valueWhenRenderered, lastBtnVal == btn);
	    return szTemp;
	}
};

const char myHeader[] = "Hello";
ButtonType lastPressedBtn = BTNTYPE_NONE;
void* copyOfUserData;

void dialogTestCallback(ButtonType buttonPressed, void* yourData) {
	lastPressedBtn = buttonPressed;
	copyOfUserData = yourData;
}

test(testBaseDialogInfo) {
	NoRenderer noRenderer;
	DialogTestImpl dialog(true);
	dialog.setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
	dialog.show(myHeader, false, NULL); // cannot send remotely.
	dialog.copyIntoBuffer("Blah blah");

	// should be inuse when shown
	assertTrue(dialog.isInUse());

	// when not a remote dialog, should never be remotely sent
	assertFalse(dialog.isRemoteUpdateNeeded(0));
	assertFalse(dialog.isRemoteUpdateNeeded(1));
	assertFalse(dialog.isRemoteUpdateNeeded(2));
	dialog.setRemoteUpdateNeeded(1, true);
	assertFalse(dialog.isRemoteUpdateNeeded(1));

	// now try the rendering
	dialog.reset();
	dialog.dialogRendering(0, false);
	assertEqual(1, dialog.getRenderCount());
	assertEqual(0, dialog.getValueWhenRenderered());
	assertStringCaseEqual("Blah blah", MenuRenderer::getInstance()->getBuffer());
	assertStringCaseEqual("CLOSE", dialog.getButtonText(1));

	// and dismiss it by the button being clicked.
	dialog.dialogRendering(0, true);
	assertFalse(dialog.isInUse());
}

test(testBaseDialogQuestion) {
	NoRenderer noRenderer;
	DialogTestImpl dialog(false);
	dialog.setUserData((void*)myHeader);
	dialog.setButtons(BTNTYPE_OK, BTNTYPE_CANCEL);
	dialog.show(myHeader, true, dialogTestCallback); // can send remote, has callback.
	dialog.copyIntoBuffer("buffer text");
	assertTrue(dialog.isInUse());

	// check that the buffer is copied and it will need a remote send.
	assertStringCaseEqual("buffer text         ", MenuRenderer::getInstance()->getBuffer());
	assertTrue(dialog.isRemoteUpdateNeeded(0));
	assertTrue(dialog.isRemoteUpdateNeeded(1));
	assertTrue(dialog.isRemoteUpdateNeeded(2));

	// clear only one remote send and check.
	dialog.setRemoteUpdateNeeded(1, false);
	assertTrue(dialog.isRemoteUpdateNeeded(0));
	assertFalse(dialog.isRemoteUpdateNeeded(1));
	assertTrue(dialog.isRemoteUpdateNeeded(2));

	// reset the text and ensure it requires remote send again
	dialog.copyIntoBuffer("newText");
	assertTrue(dialog.isRemoteUpdateNeeded(1));

	// now simualte render
	dialog.reset();
	dialog.dialogRendering(0, false);
	assertEqual(1, dialog.getRenderCount());
	assertEqual(0, dialog.getValueWhenRenderered());
	assertStringCaseEqual("newText             ", MenuRenderer::getInstance()->getBuffer());
	assertStringCaseEqual("OK", dialog.getButtonText(0));
	assertStringCaseEqual("cancel", dialog.getButtonText(1));

	// and again, this time choose 2nd button.
	dialog.dialogRendering(1, false);
	assertEqual(2, dialog.getRenderCount());
	assertStringCaseEqual("ok", dialog.getButtonText(0));
	assertStringCaseEqual("CANCEL", dialog.getButtonText(1));

	// and now simulate the remote cancelling
	dialog.remoteAction(BTNTYPE_CANCEL);
	assertEqual(BTNTYPE_CANCEL, lastPressedBtn);
	assertEqual(myHeader, copyOfUserData);

	// it should not be in use after this.
	assertFalse(dialog.isInUse());
}

#endif //BASE_DIALOG_TESTS_H
