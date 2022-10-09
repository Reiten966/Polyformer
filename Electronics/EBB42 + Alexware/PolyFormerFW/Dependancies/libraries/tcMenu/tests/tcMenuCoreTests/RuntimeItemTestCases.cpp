
#include <AUnit.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "fixtures_extern.h"
#include <tcUtil.h>

bool renderActivateCalled = false;

int testBasicRuntimeFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch (mode) {
        case RENDERFN_NAME: {
            if (row < 10) {
                strcpy(buffer, "name");
                fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
            }
            else {
                strcpy(buffer, "hello");
            }
            break;
        }
        case RENDERFN_VALUE:
            ltoaClrBuff(buffer, row, row, NOT_PADDED, bufferSize);
            break;
        case RENDERFN_EEPROM_POS:
            return 44;
        case RENDERFN_INVOKE:
            renderActivateCalled = true;
            break;
        default: break;
    }
    return true;
}

test(testBasicRuntimeMenuItem) {
    RuntimeMenuItem item(MENUTYPE_RUNTIME_VALUE, 22, testBasicRuntimeFn, 222, 1, NULL);

    assertEqual(item.getId(), uint16_t(22));
    assertEqual(item.getEepromPosition(), uint16_t(44));
    char sz[20];
    item.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("hello", sz);
    item.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("222", sz);

    renderActivateCalled = false;
    item.runCallback();
    assertTrue(renderActivateCalled);
}

test(testListRuntimeItem) {
    ListRuntimeMenuItem item(22, 2, testBasicRuntimeFn, NULL);

    // check the name and test on the "top level" or parent item
    char sz[20];

    // ensure there are two parts
    assertEqual(uint8_t(2), item.getNumberOfParts());

    RuntimeMenuItem* child = item.getChildItem(0);
    assertEqual(MENUTYPE_RUNTIME_LIST, child->getMenuType());
    child->copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("name0", sz);
    child->copyValue(sz, sizeof(sz));
    assertStringCaseEqual("0", sz);

    child = item.getChildItem(1);
    assertEqual(MENUTYPE_RUNTIME_LIST, child->getMenuType());
    child->copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("name1", sz);
    child->copyValue(sz, sizeof(sz));
    assertStringCaseEqual("1", sz);

    RuntimeMenuItem* back = item.asBackMenu();
    assertEqual(MENUTYPE_BACK_VALUE, back->getMenuType());

    RuntimeMenuItem* parent = item.asParent();
    assertEqual(MENUTYPE_RUNTIME_LIST, back->getMenuType());
    assertEqual(parent->getId(), uint16_t(22));
    assertEqual(parent->getEepromPosition(), uint16_t(44));
    parent->copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("hello", sz);
    item.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("255", sz);
}

void renderCallback(int id) {
    renderActivateCalled = true;
}

RENDERING_CALLBACK_NAME_INVOKE(textMenuItemTestCb, textItemRenderFn, "HelloWorld", 99, renderCallback)

test(testTextMenuItemFromEmpty) {
    TextMenuItem textItem(textMenuItemTestCb, 33, 10, NULL);

    // first simulate eeprom loading back from storage.
    uint8_t* data = (uint8_t*)textItem.getTextValue();
    data[0] = 0;
    data[1] = 'Z';
    data[2] = 'Y';
    data[3] = 'X';
    data[4] = '[';
    data[5] = ']';
    textItem.cleanUpArray();

    // start off with an empty string
    char sz[20];
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("", sz);

    // ensure we can edit an empty string position
    assertEqual(uint8_t(10), textItem.beginMultiEdit());
    assertTrue(textItem.isEditing());
    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    assertEqual(0, textItem.getPartValueAsInt());

    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[]", sz);

    // add char to empty string
    textItem.valueChanged(findPositionInEditorSet('N'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[N]", sz);

    // add another char to empty string
    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('E'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("N[E]", sz);

    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('T'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("NE[T]", sz);

    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('_'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("NET[_]", sz);

    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());

    // check that the edit worked ok
    textItem.stopMultiEdit();
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("NET_", sz);

    // now start editing again and clear down the string to zero terminated at position 0
    assertEqual(uint8_t(10), textItem.beginMultiEdit());
    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(0);
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[]", sz);

    // should be empty now
    textItem.stopMultiEdit();
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("", sz);

    // check every byte of buffer is 0.
    for (int i = 0; i < textItem.textLength(); i++) assertEqual(int(data[i]), 0);
}

test(testFindEditorSetFunction) {
    assertEqual(13, findPositionInEditorSet('9'));
    assertEqual(24, findPositionInEditorSet('K'));
    assertEqual(94, findPositionInEditorSet('~'));
    assertEqual(1, findPositionInEditorSet(' '));
    assertEqual(2, findPositionInEditorSet('.'));
    assertEqual(0, findPositionInEditorSet(0));
}

test(testTextPasswordItem) {
    // this menu item is fully tested in the main tests
    // here we concentrate on password functions
    TextMenuItem textItem(textMenuItemTestCb, 33, 5, NULL);
    textItem.setPasswordField(true);
    textItem.setTextValue("1234");

    char sz[20];
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("****", sz);

    assertEqual(uint8_t(5), textItem.beginMultiEdit());
    assertTrue(textItem.isEditing());
    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('9'));
    assertEqual(findPositionInEditorSet('9'), textItem.getPartValueAsInt());
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[9]***", sz);
    assertStringCaseEqual("9234", textItem.getTextValue());
}

test(testTextRuntimeItem) {
    TextMenuItem textItem(textMenuItemTestCb, 33, 10, NULL);
    textItem.setTextValue("Goodbye");

    assertEqual(textItem.getId(), uint16_t(33));
    assertEqual(textItem.getEepromPosition(), uint16_t(99));

    // check the name and test on the "top level" or parent item
    char sz[20];
    textItem.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("HelloWorld", sz);
    copyMenuItemValue(&textItem, sz, sizeof(sz));
    assertStringCaseEqual("Goodbye", sz);

    assertEqual(uint8_t(10), textItem.beginMultiEdit());
    assertTrue(textItem.isEditing());
    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    assertEqual(findPositionInEditorSet('G'), textItem.getPartValueAsInt());

    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[G]oodbye", sz);

    textItem.valueChanged(findPositionInEditorSet('0'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[0]oodbye", sz);

    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    assertEqual(findPositionInEditorSet('o'), textItem.getPartValueAsInt());
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("0[o]odbye", sz);

    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("0o[o]dbye", sz);

    textItem.valueChanged(findPositionInEditorSet('1'));
    assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("0o1[d]bye", sz);

    renderActivateCalled = false;
    textItem.stopMultiEdit();
    assertTrue(renderActivateCalled);
    assertFalse(textItem.isEditing());
    textItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("0o1dbye", sz);
}

RENDERING_CALLBACK_NAME_INVOKE(rtSubMenuFn, backSubItemRenderFn, "My Sub", 0xffff, NULL)
SubMenuItem rtSubMenu(101, rtSubMenuFn, &menuVolume, &menuContrast);

test(testSubMenuItem) {
    char sz[20];
    menuSub.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("Settings", sz);
    assertTrue(&menuBackSub == menuSub.getChild());
    assertTrue(isMenuRuntime(&menuSub));
    assertTrue(menuSub.getMenuType() == MENUTYPE_SUB_VALUE);
    assertEqual((uint16_t)7, menuSub.getId());
    assertEqual((uint16_t)-1, menuSub.getEepromPosition());

    rtSubMenu.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("My Sub", sz);
    assertTrue(&menuVolume == rtSubMenu.getChild());
    assertTrue(&menuContrast == rtSubMenu.getNext());
    assertTrue(isMenuRuntime(&rtSubMenu));
    assertTrue(rtSubMenu.getMenuType() == MENUTYPE_SUB_VALUE);
    assertEqual((uint16_t)101, rtSubMenu.getId());
    assertEqual((uint16_t)-1, rtSubMenu.getEepromPosition());
}

int actionCbCount = 0;
void myActionCb(int id) {
    actionCbCount++;
}

test(testActionMenuItem) {
    char sz[20];
    menuPressMe.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("Press me", sz);
    assertTrue(!isMenuRuntime(&menuPressMe));
    assertTrue(menuPressMe.getMenuType() == MENUTYPE_ACTION_VALUE);
    assertEqual((uint16_t)7, menuSub.getId());
    assertEqual((uint16_t)-1, menuSub.getEepromPosition());
    auto oldCbCount = actionCbCount;
    menuPressMe.triggerCallback();
    assertEqual(oldCbCount + 1, actionCbCount);

    copyMenuItemNameAndValue(&menuPressMe, sz, sizeof sz);
    assertStringCaseEqual("Press me: >>", sz);
}
