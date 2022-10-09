#include <Arduino.h>
#include <AUnit.h>
#include <graphics/GfxMenuConfig.h>
#include <graphics/BaseGraphicalRenderer.h>
#include "fixtures_extern.h"
#include "TestCapturingRenderer.h"

using namespace tcgfx;

color_t palette1[] = {RGB(0,0,0),RGB(255,255,255),RGB(1,2,3),RGB(3,2,1)};
color_t palette2[] = {RGB(55,55,55),RGB(66,66,66),RGB(77,77,77),RGB(0,0,0)};
color_t palette3[] = {RGB(0,0,0),RGB(22,22,22),RGB(33,44,55),RGB(6,5,4)};

const uint8_t pointer1[] = { 0x01, 0x02, 0x03, 0x04 };
const uint8_t pointer2[] = { 0x01, 0x02, 0x03, 0x04 };
const uint8_t pointer3[] = { 0x01, 0x02, 0x03, 0x04 };

bool checkPadding(MenuPadding padding, uint16_t top, uint16_t right, uint16_t bottom, uint16_t left) {
    if (padding.top != top) {
        serdebugF3("Padding top mismatch: ", top, padding.top);
        return false;
    } else if (padding.right != right) {
        serdebugF3("Padding right mismatch: ", right, padding.right);
        return false;
    } else if (padding.left != left) {
        serdebugF3("Padding left mismatch: ", left, padding.left);
        return false;
    } else if (padding.bottom != bottom) {
        serdebugF3("Padding bottom mismatch: ", bottom, padding.bottom);
        return false;
    }
    return true;
}

bool checkPropertiesBasics(ItemDisplayProperties* props, const char* name, const void* expectedFont, uint8_t mag,
                           color_t colorBg, color_t colorFg, uint8_t spacing, uint8_t height, GridPosition::GridJustification just) {
    serdebugF2("Checking properties ", name);
    if(props == nullptr) {
        serdebugF("Props null");
        return false;
    }
    if(expectedFont != props->getFont() || mag != props->getFontMagnification()) {
        serdebugF3("Font or mag mismatch ", mag, props->getFontMagnification());
        return false;
    }
    if(colorBg != props->getColor(ItemDisplayProperties::BACKGROUND)) {
        serdebugFHex2("Mismatch on BG ", colorBg, props->getColor(ItemDisplayProperties::BACKGROUND));
        return false;
    }
    if(colorFg != props->getColor(ItemDisplayProperties::TEXT)) {
        serdebugFHex2("Mismatch on FG ", colorBg, props->getColor(ItemDisplayProperties::BACKGROUND));
        return false;
    }
    if(spacing != props->getSpaceAfter()) {
        serdebugF3("Spacing out ", spacing, props->getSpaceAfter());
        return false;
    }
    if(height != props->getRequiredHeight()) {
        serdebugF3("Height out ", height, props->getRequiredHeight());
        return false;
    }
    if(just != props->getDefaultJustification()) {
        serdebugF3("Justification out ", just, props->getDefaultJustification());
    }
    return true;
}

test(testEmptyItemPropertiesFactory) {
    assertEqual(4, (int)sizeof(GridPosition));
    assertEqual(4, (int)sizeof(Coord));
    assertEqual(2, (int)sizeof(MenuPadding));
    assertEqual(1, (int)sizeof(MenuBorder));

    ConfigurableItemDisplayPropertiesFactory factory;
    auto *config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
    assertTrue(checkPropertiesBasics(config, "empty not null", nullptr, 1, RGB(0, 0, 0), RGB(255, 255, 255), 2, 12, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT));
    assertTrue(checkPadding(config->getPadding(), 2, 2, 2, 2));
}

void populatePropsWithDefaults(ConfigurableItemDisplayPropertiesFactory& factory) {
    factory.setSelectedColors(RGB(1, 2, 3), RGB(3, 2, 1));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, palette1, MenuPadding(4, 3, 2, 1), pointer1, 4, 22, 30, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette2, MenuPadding(2), pointer2, 1, 4, 40, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, palette2, MenuPadding(1), pointer3, 2, 2, 50, GridPosition::JUSTIFY_LEFT_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesAllInSub(ItemDisplayProperties::COMPTYPE_ITEM, menuSub.getId(), palette3, MenuPadding(3), pointer1, 3, 10, 60, GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesForItem(ItemDisplayProperties::COMPTYPE_ITEM, menuSubAnalog.getId(), palette1, MenuPadding(6), pointer2, 3, 12, 80, GridPosition::JUSTIFY_CENTER_WITH_VALUE, MenuBorder(0));
    factory.addGridPosition(&menuVolume, GridPosition(GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 2, 100));
    menuMgr.initWithoutInput(&noRenderer, &textMenuItem1);
    taskManager.reset();
}

test(testDefaultItemPropertiesFactory) {
    ConfigurableItemDisplayPropertiesFactory factory;
    populatePropsWithDefaults(factory);

    // check that when we request with null item we get the default
    auto* config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
    assertTrue(checkPropertiesBasics(config, "default item", pointer2, 1, palette2[1], palette2[0], 4, 40, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT));
    assertTrue(checkPadding(config->getPadding(), 2,2,2,2));

    // check that when we request with null title we get the default
    config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
    assertTrue(checkPropertiesBasics(config, "default title", pointer1, 4, palette1[1], palette1[0], 22, 30, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT));
    assertTrue(checkPadding(config->getPadding(), 4,3,2,1));

    // check that when we request with null action we get the default
    config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ACTION);
    assertTrue(checkPropertiesBasics(config, "default action", pointer3, 2, palette2[1], palette2[0], 2, 50, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT));
    assertTrue(checkPadding(config->getPadding(), 1,1,1,1));

    // now change the default item and ensure that it picks up the new settings.
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette1, MenuPadding(2,3,4,5), pointer1, 2, 14, 15, GridPosition::JUSTIFY_RIGHT_NO_VALUE, MenuBorder(0));
    config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
    assertTrue(checkPropertiesBasics(config, "default changed item", pointer1, 2, palette1[1], palette1[0], 14, 15, GridPosition::JUSTIFY_RIGHT_NO_VALUE));
    assertTrue(checkPadding(config->getPadding(), 2,3,4,5));
}

test(testSubAndItemSelectionPropertiesFactory) {
    ConfigurableItemDisplayPropertiesFactory factory;
    populatePropsWithDefaults(factory);

    menuMgr.navigateToMenu(menuSub.getChild());

    // using the submenu level settings because it is in menuSub, with no item level override.
    auto* config = factory.configFor(&menuIpAddr, ItemDisplayProperties::COMPTYPE_ITEM);
    assertTrue(checkPropertiesBasics(config, "override sub", pointer1, 3, palette3[1], palette3[0], 10, 60, GridPosition::JUSTIFY_CENTER_NO_VALUE));
    assertTrue(checkPadding(config->getPadding(), 3,3,3,3));

    // using the item level settings because it has an override at item and sub, item takes priority.
    config = factory.configFor(&menuSubAnalog, ItemDisplayProperties::COMPTYPE_ITEM);
    assertTrue(checkPropertiesBasics(config, "override item", pointer2, 3, palette1[1], palette1[0], 12, 80, GridPosition::JUSTIFY_CENTER_WITH_VALUE));
    assertTrue(checkPadding(config->getPadding(), 6,6,6,6));
}

test(testIconStorageAndRetrival) {
    ConfigurableItemDisplayPropertiesFactory factory;
    factory.addImageToCache(DrawableIcon(menuVolume.getId(), Coord(1, 3), DrawableIcon::ICON_XBITMAP, pointer1, pointer2));
    factory.addImageToCache(DrawableIcon(menuSub.getId(), Coord(2, 1), DrawableIcon::ICON_NATIVE, pointer3));

    assertTrue(nullptr == factory.iconForMenuItem(menuPressMe.getId()));

    auto* icon = factory.iconForMenuItem(menuVolume.getId());
    assertTrue(icon != nullptr);
    assertEqual(icon->getIconType(), DrawableIcon::ICON_XBITMAP);
    int iconX = (int)(icon->getDimensions().x);
    int iconY = (int)(icon->getDimensions().y);
    assertEqual(1, iconX);
    assertEqual(3, iconY);
    assertEqual(pointer1, icon->getIcon(false));
    assertEqual(pointer2, icon->getIcon(true));

    icon = factory.iconForMenuItem(menuSub.getId());
    assertTrue(icon != nullptr);
    assertEqual(icon->getIconType(), DrawableIcon::ICON_NATIVE);
    iconX = (int)(icon->getDimensions().x);
    iconY = (int)(icon->getDimensions().y);
    assertEqual(2, iconX);
    assertEqual(1, iconY);
    assertEqual(pointer3, icon->getIcon(false));
    assertEqual(pointer3, icon->getIcon(true));
}

test(testGridPositionStorageAndRetrival) {
    ConfigurableItemDisplayPropertiesFactory factory;
    factory.addGridPosition(&menuVolume, GridPosition(GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 2, 10));
    factory.addGridPosition(&menuStatus, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 2, 4, 0));

    assertTrue(nullptr == factory.gridPositionForItem(&menuSub));

    auto* grid = factory.gridPositionForItem(&menuVolume);
    assertEqual(grid->getPosition().getDrawingMode(), GridPosition::DRAW_INTEGER_AS_UP_DOWN);
    assertEqual(grid->getPosition().getRow(), 2);
    assertEqual(grid->getPosition().getGridPosition(), 1);
    assertEqual(grid->getPosition().getGridSize(), 1);
    assertEqual(grid->getPosition().getGridHeight(), 10);
    assertEqual(grid->getPosition().getJustification(), GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT);

    grid = factory.gridPositionForItem(&menuStatus);
    assertEqual(grid->getPosition().getDrawingMode(), GridPosition::DRAW_AS_ICON_ONLY);
    assertEqual(grid->getPosition().getRow(), 4);
    assertEqual(grid->getPosition().getGridPosition(), 2);
    assertEqual(grid->getPosition().getGridSize(), 3);
    assertEqual(grid->getPosition().getGridHeight(), 0);
    assertEqual(grid->getPosition().getJustification(), GridPosition::JUSTIFY_CENTER_NO_VALUE);
}

const uint8_t* const icons1[] PROGMEM {pointer1, pointer2};
const uint8_t* const icons2[] PROGMEM {pointer1, pointer2, pointer3};

TitleWidget widget2 = TitleWidget(icons2, 3, 8, 4, nullptr);
TitleWidget widget1 = TitleWidget(icons1, 2, 8, 3, &widget2);
const char pgmName[] PROGMEM = "Test";

test(testWidgetFunctionality) {
    assertEqual((uint8_t)8, widget1.getWidth());
    assertEqual((uint8_t)3, widget1.getHeight());
    assertEqual((uint8_t)2, widget1.getMaxValue());
    assertEqual((uint8_t)8, widget2.getWidth());
    assertEqual((uint8_t)4, widget2.getHeight());
    assertEqual((uint8_t)3, widget2.getMaxValue());

    widget1.setCurrentState(0);
    assertEqual((uint8_t)0, widget1.getCurrentState());
    assertEqual(pointer1, widget1.getCurrentIcon());
    widget1.setCurrentState(19);
    assertEqual((uint8_t)0, widget1.getCurrentState());
    widget1.setCurrentState(1);
    assertEqual((uint8_t)1, widget1.getCurrentState());
    assertEqual(pointer2, widget1.getCurrentIcon());

    widget2.setCurrentState(2);
    assertEqual((uint8_t)2, widget2.getCurrentState());
    assertEqual(pointer3, widget2.getCurrentIcon());
    assertEqual(pointer3, widget2.getIcon(2));
    assertEqual(pointer1, widget2.getIcon(20));
}

bool checkWidget(int widNum, WidgetDrawingRecord* item, Coord where, color_t* palette, int updatesExpected) {
    serdebugF2("check widget", widNum);
    if(item == nullptr) {
        serdebugF("widget not found");
        return false;
    }
    if(where.x != item->where.x || where.y != item->where.y) {
        serdebugF3("widget pos wrong ", item->where.x, item->where.y);
        return false;
    }
    if(item->bg != palette[1] || item->fg != palette[2] || item->updated != updatesExpected) {
        serdebugF4("clr/update wrong", item->fg, item->bg, item->updated);
        return false;
    }
    return true;
}

bool checkItem(MenuDrawingRecord* record, Coord where, Coord size, const void* font, GridPosition::GridDrawingMode mode, GridPosition::GridJustification justification, int expectedUpdates, MenuItem* pItem = nullptr) {
    if(record == nullptr) {
        serdebugF("null rec");
        return false;
    }
    serdebugF3("check item ", record->position.getRow(), record->position.getGridPosition());

    if(where.x != record->where.x || where.y != record->where.y) {
        serdebugF3("where err ", record->where.x, record->where.y);
        return false;
    }
    if(size.x != record->size.x || size.y != record->size.y) {
        serdebugF3("size err ", record->size.x, record->size.y);
        return false;
    }

    if(font != record->properties->getFont() || mode != record->position.getDrawingMode() || justification != record->position.getJustification()) {
        serdebugF3("drawing err ", record->position.getDrawingMode(), record->position.getJustification());
        return false;
    }

    if(pItem != nullptr && pItem != record->theItem) {
        serdebugF("Item ptr out");
    }

    if(expectedUpdates != record->updated) {
        serdebugF2("updates out", record->updated);
        return false;
    }

    return true;
}

test(testBaseRendererWithDefaults) {

    TestCapturingRenderer renderer(320, 120, false, pgmName);

    renderer.setFirstWidget(&widget1);
    auto& factory = reinterpret_cast<ConfigurableItemDisplayPropertiesFactory &>(renderer.getDisplayPropertiesFactory());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, palette1, MenuPadding(4), pointer2, 1, 10, 30, GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, palette1, MenuPadding(4), pointer1, 1, 5, 25, GridPosition::JUSTIFY_LEFT_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette1, MenuPadding(4), pointer1, 1, 5, 20, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));

    menuMgr.initWithoutInput(&renderer, &textMenuItem1);
    taskManager.reset();

    widget1.setCurrentState(0);
    menuEnum1.setCurrentValue(0);
    renderer.resetCommandStates();
    renderer.exec();

    assertTrue(renderer.checkCommands(true, true, true));

    assertEqual((bsize_t)2, renderer.getWidgetRecordings().count());
    assertTrue(checkWidget(1, renderer.getWidgetRecordings().getByKey((unsigned long)&widget1), Coord(320 - 8 - 4, 4), palette1, 0));
    assertTrue(checkWidget(2, renderer.getWidgetRecordings().getByKey((unsigned long)&widget2), Coord(320 - 16 - 8, 4), palette1, 0));

    assertEqual((bsize_t)2, renderer.getWidgetRecordings().count());
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(0), Coord(0, 0), Coord(320, 30), pointer2, GridPosition::DRAW_TITLE_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(1), Coord(0, 40), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &textMenuItem1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(2), Coord(0, 65), Coord(320, 25), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_LEFT_NO_VALUE, 0, &boolItem1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(3), Coord(0, 95), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuEnum1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(4), Coord(0, 120), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_SCROLL, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuAnalog2));
    assertFalse(textMenuItem1.isChanged());
    assertFalse(boolItem1.isChanged());
    assertFalse(menuEnum1.isChanged());
    assertFalse(menuAnalog2.isChanged());

    widget1.setCurrentState(1);
    menuEnum1.setCurrentValue(1);
    renderer.resetCommandStates();
    renderer.exec();

    assertTrue(renderer.checkCommands(false, true, true));
    assertTrue(checkWidget(1, renderer.getWidgetRecordings().getByKey((unsigned long)&widget1), Coord(320 - 8 - 4, 4), palette1, 1));
    assertTrue(checkWidget(2, renderer.getWidgetRecordings().getByKey((unsigned long)&widget2), Coord(320 - 16 - 8, 4), palette1, 0));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(0), Coord(0, 0), Coord(320, 30), pointer2, GridPosition::DRAW_TITLE_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(1), Coord(0, 40), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &textMenuItem1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(2), Coord(0, 65), Coord(320, 25), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_LEFT_NO_VALUE, 0, &boolItem1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(3), Coord(0, 95), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 1, &menuEnum1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(4), Coord(0, 120), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_SCROLL, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuAnalog2));
    assertFalse(menuEnum1.isChanged());

    renderer.getMenuItemAtIndex(menuMgr.getCurrentMenu(), 0)->setChanged(true);
    renderer.resetCommandStates();
    renderer.exec();

    assertTrue(renderer.checkCommands(false, true, true));
    assertTrue(checkWidget(1, renderer.getWidgetRecordings().getByKey((unsigned long)&widget1), Coord(320 - 8 - 4, 4), palette1, 2));
    assertTrue(checkWidget(2, renderer.getWidgetRecordings().getByKey((unsigned long)&widget2), Coord(320 - 16 - 8, 4), palette1, 1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(0), Coord(0, 0), Coord(320, 30), pointer2, GridPosition::DRAW_TITLE_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(3), Coord(0, 95), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 1, &menuEnum1));
}

void selectItem(MenuItem* root, MenuItem* toSelect) {
    while(root != nullptr) {
        root->setActive(root == toSelect);
        root = root->getNext();
    }
}

test(testScrollingWithMoreThanOneItemOnRow) {
    TestCapturingRenderer renderer(320, 100, false, pgmName);
    renderer.setTitleMode(BaseGraphicalRenderer::NO_TITLE);

    // first get hold of the factory and add the drawing defaults
    auto& factory = reinterpret_cast<ConfigurableItemDisplayPropertiesFactory &>(renderer.getDisplayPropertiesFactory());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, palette1, MenuPadding(4), pointer1, 1, 5, 25, GridPosition::JUSTIFY_LEFT_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette1, MenuPadding(4), pointer1, 1, 5, 20, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));
    // now make a row 1 have 2 columns with boolItem1 in position 1 and menuSub in position 2.
    factory.addGridPosition(&boolItem1, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_WITH_VALUE, 2, 1, 1, 35));
    factory.addGridPosition(&menuSub, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 2, 2, 1, 35));

    menuMgr.initWithoutInput(&renderer, &textMenuItem1);
    taskManager.reset(); // this must be done to clear out the task created by calling initialise above.

    // run the first iteration and check the drawing positions
    renderer.resetCommandStates();
    renderer.exec();
    assertTrue(renderer.checkCommands(true, true, true));
    assertEqual((bsize_t)0, renderer.getWidgetRecordings().count());
    assertEqual((bsize_t)5, renderer.getMenuItemRecordings().count());

    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(0), Coord(0, 0), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &textMenuItem1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(1), Coord(0, 25), Coord(159, 35), pointer1, GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_WITH_VALUE, 0, &boolItem1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(101), Coord(160, 25), Coord(159, 35), pointer1, GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0, &menuSub));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(2), Coord(0, 65), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuEnum1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(3), Coord(0, 90), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_SCROLL, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuAnalog2));

    // now select an item that's off the display, it should remove the first item from the display we clear down the all the
    // states in the test renderer so we can check that we completely refreshed the display, and the first item is not drawn
    selectItem(&textMenuItem1, &menuAnalog);
    renderer.resetCommandStates();
    renderer.getMenuItemRecordings().clear();
    renderer.exec();
    assertTrue(renderer.checkCommands(false, true, true));
    assertEqual((bsize_t)0, renderer.getWidgetRecordings().count());
    assertEqual((bsize_t)4, renderer.getMenuItemRecordings().count());

    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(1), Coord(0, 0), Coord(160, 35), pointer1, GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_WITH_VALUE, 0, &boolItem1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(101), Coord(160, 0), Coord(160, 35), pointer1, GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0, &menuSub));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(2), Coord(0, 40), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuEnum1));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(3), Coord(0, 65), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_SCROLL, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuAnalog2));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(4), Coord(0, 90), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_SCROLL, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuAnalog));
}

class DisplayDrawing : public CustomDrawing {
private:
    bool hasStarted = false;
    bool resetCalled = false;
    int ticks = 0;
public:
    void started(BaseMenuRenderer *currentRenderer) override {
        hasStarted = true;
    }

    void reset() override {
        resetCalled = true;
    }

    void renderLoop(unsigned int currentValue, RenderPressMode userClick) override {
        ticks++;
    }

    bool didStart() const { return hasStarted; }
    bool didReset() const { return resetCalled; }
    int getTicks() const { return ticks; }

    void clearFlags() {
        hasStarted = false;
        resetCalled = false;
    }
};

test(testTakeOverDisplay) {
    TestCapturingRenderer renderer(320, 100, false, pgmName);
    DisplayDrawing drawingTest;
    renderer.setCustomDrawingHandler(&drawingTest);
    for(int i=0; i<400 ;i++) {
        renderer.exec();
    }
    assertTrue(drawingTest.didReset());
    assertFalse(drawingTest.didStart());

    renderer.takeOverDisplay();
    renderer.exec();
    assertTrue(drawingTest.didStart());
    assertEqual(0, drawingTest.getTicks());
    drawingTest.clearFlags();
    for(int i=0;i<500;i++) renderer.exec();
    assertEqual(500, drawingTest.getTicks());
    assertFalse(drawingTest.didReset());

    renderer.giveBackDisplay();
    for(int i=0;i<500;i++) renderer.exec();
    assertTrue(drawingTest.didReset());

    assertEqual(500, drawingTest.getTicks());
}

extern int testBasicRuntimeFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

test(testListRendering) {
    ListRuntimeMenuItem runtimeItem(101, 20, testBasicRuntimeFn, nullptr);
    TestCapturingRenderer renderer(320, 100, false, pgmName);
    DisplayDrawing drawingTest;
    menuMgr.initWithoutInput(&renderer, &runtimeItem);
    auto& factory = reinterpret_cast<ConfigurableItemDisplayPropertiesFactory &>(renderer.getDisplayPropertiesFactory());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, palette1, MenuPadding(4), pointer2, 1, 10, 30, GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette1, MenuPadding(4), pointer1, 1, 5, 20, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));
    taskManager.reset();

    renderer.resetCommandStates();
    renderer.exec();
    assertEqual((bsize_t)5, renderer.getMenuItemRecordings().count());
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(0), Coord(0, 0), Coord(320, 30), pointer2, GridPosition::DRAW_TITLE_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0, &runtimeItem));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(1), Coord(0, 40), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &runtimeItem));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(2), Coord(0, 65), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &runtimeItem));
    assertTrue(checkItem(renderer.getMenuItemRecordings().getByKey(3), Coord(0, 90), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &runtimeItem));
}