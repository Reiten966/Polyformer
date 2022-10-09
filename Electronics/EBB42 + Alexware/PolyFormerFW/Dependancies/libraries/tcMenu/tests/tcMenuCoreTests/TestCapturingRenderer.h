#ifndef TCMENU_TESTCAPTURINGRENDERER_H
#define TCMENU_TESTCAPTURINGRENDERER_H

#include <graphics/BaseGraphicalRenderer.h>
#include <graphics/GfxMenuConfig.h>

using namespace tcgfx;

struct WidgetDrawingRecord {
    WidgetDrawingRecord() : index(nullptr), where(0,0), bg(0), fg(0), updated(0) {}
    WidgetDrawingRecord(TitleWidget* w, Coord wh, color_t b, color_t f) : index(w), where(wh), bg(b), fg(f), updated(0) {}
    WidgetDrawingRecord(const WidgetDrawingRecord& other) = default;
    WidgetDrawingRecord& operator=(const WidgetDrawingRecord& other) = default;
    TitleWidget* index;
    Coord where;
    color_t bg, fg;
    int updated;
    uint32_t getKey() const {return (unsigned long)index;}
};

struct MenuDrawingRecord {
    MenuDrawingRecord() : rowIndex(0), where(0,0), size(0, 0), properties(nullptr), theItem(nullptr), position{}, updated(0) {}
    MenuDrawingRecord(uint8_t row, Coord wh, Coord sz, ItemDisplayProperties* prop, MenuItem* item, GridPosition pos)
    : rowIndex(row), where(wh), size(sz), properties(prop), theItem(item), position(pos), updated(0) {}
    MenuDrawingRecord(const MenuDrawingRecord& other) = default;
    MenuDrawingRecord& operator=(const MenuDrawingRecord& other) = default;
    uint8_t rowIndex;
    Coord where;
    Coord size;
    ItemDisplayProperties* properties;
    MenuItem* theItem;
    GridPosition position;
    int updated;
    uint8_t getKey() const {return rowIndex;}
};

class TestCapturingRenderer : public BaseGraphicalRenderer {
private:
    bool clearScreen, startCmd, endCmd;
    BtreeList<uint32_t, WidgetDrawingRecord> widgetRecordings;
    BtreeList<uint8_t, MenuDrawingRecord> menuItemRecordings;
    ConfigurableItemDisplayPropertiesFactory propertiesFactory;
public:
    TestCapturingRenderer(int wid, int hei, bool lastRowExact, const char *appTitle)
    : BaseGraphicalRenderer(32, wid, hei, lastRowExact, appTitle), widgetRecordings(3) {
        clearScreen = startCmd = endCmd = false;
    }

    bool checkCommands(bool clear, bool start, bool end) {
        if(clear == clearScreen && start == startCmd && end == endCmd) return true;
        serdebugF4("Command diff ", startCmd, endCmd, clearScreen);
        return false;
    }

    BtreeList<uint32_t, WidgetDrawingRecord>& getWidgetRecordings() { return widgetRecordings; }
    BtreeList<uint8_t, MenuDrawingRecord>& getMenuItemRecordings() { return menuItemRecordings; }

    void resetCommandStates() {
        clearScreen = false;
        startCmd = false;
        endCmd = false;
    }

    BaseDialog *getDialog() override {
        return nullptr;
    }

    void drawWidget(Coord where, TitleWidget *widget, color_t colorFg, color_t colorBg) override {
        auto *widRec = widgetRecordings.getByKey((unsigned long)widget);
        if(widRec) {
            widRec->where = where;
            widRec->bg = colorBg;
            widRec->fg = colorFg;
            widRec->updated++;
        }
        else widgetRecordings.add(WidgetDrawingRecord(widget, where, colorBg, colorFg));
    }

    void drawMenuItem(GridPositionRowCacheEntry *entry, Coord where, Coord areaSize, bool /*drawAll*/) override {
        uint8_t row = entry->getPosition().getRow() + ((entry->getPosition().getGridPosition() - 1) * 100);
        auto* itemRec = menuItemRecordings.getByKey(row);
        if(itemRec) {
            serdebugF2("Draw update at ", row);
            itemRec->where = where;
            itemRec->size = areaSize;
            itemRec->theItem = entry->getMenuItem();
            itemRec->properties = entry->getDisplayProperties();
            itemRec->position = entry->getPosition();
            itemRec->updated++;
        }
        else {
            serdebugF2("Draw at ", row);
            menuItemRecordings.add(MenuDrawingRecord(row, where, areaSize, entry->getDisplayProperties(), entry->getMenuItem(), entry->getPosition()));
        }
    }

    void drawingCommand(RenderDrawingCommand command) override {
        switch(command) {
            case DRAW_COMMAND_CLEAR:
                clearScreen = true;
                return;
                case DRAW_COMMAND_START:
                    startCmd = true;
                    return;
                    case DRAW_COMMAND_ENDED:
                        endCmd = true;
                        return;
        }
    }

    ItemDisplayPropertiesFactory &getDisplayPropertiesFactory() override {
        return propertiesFactory;
    }

    void fillWithBackgroundTo(int startY) override { }
};

#endif //TCMENU_TESTCAPTURINGRENDERER_H
