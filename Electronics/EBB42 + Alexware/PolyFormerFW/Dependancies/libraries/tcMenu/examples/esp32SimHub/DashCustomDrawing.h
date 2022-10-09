
#ifndef TCMENU_EXAMPLE_DASHCUSTOMDRAWING_H
#define TCMENU_EXAMPLE_DASHCUSTOMDRAWING_H

#include "BaseRenderers.h"
#include "esp32SimHub_menu.h"

#define LED_STATES 10
#define UPDATE_COUNTDOWN 4;

class DashDrawParameters {
public:
    enum DashAlign {
        TITLE_LEFT_VALUE_LEFT, TITLE_LEFT_VALUE_RIGHT,
        NO_TITLE_VALUE_LEFT, NO_TITLE_VALUE_RIGHT,
        TITLE_RIGHT_VALUE_LEFT, TITLE_RIGHT_VALUE_RIGHT };
protected:
    DashAlign alignment;
    const GFXfont* font;
    uint16_t fgColor;
    uint16_t bgColor;
public:
    DashDrawParameters(uint16_t fgColor_, uint16_t bgColor_, const GFXfont* font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT) {
        alignment = align;
        font = font_;
        fgColor = fgColor_;
        bgColor = bgColor_;
    }

    bool isTitleDrawn() {
        return alignment != NO_TITLE_VALUE_LEFT && alignment != NO_TITLE_VALUE_RIGHT;
    }

    bool isTitleLeftAlign() {
        return alignment == TITLE_LEFT_VALUE_LEFT || alignment == TITLE_LEFT_VALUE_RIGHT;
    }
    bool isValueLeftAlign() {
        return alignment == TITLE_RIGHT_VALUE_LEFT || alignment == TITLE_LEFT_VALUE_LEFT || alignment == NO_TITLE_VALUE_LEFT;
    }
    const GFXfont* getFont() {
        return font;
    }

    virtual uint16_t getBgColor(MenuItem *item, bool updated)  {
        return bgColor;
    }

    virtual uint16_t getFgColor(MenuItem *item, bool updated)  {
        return fgColor;
    }

    virtual uint16_t getTitleBgColor(MenuItem *item, bool updated)  {
        return bgColor;
    }

    virtual uint16_t getTitleFgColor(MenuItem *item, bool updated)  {
        return fgColor;
    }
};

class DashDrawParametersUpdate : public DashDrawParameters {
private:
    uint16_t fgUpdateColor;
    uint16_t bgUpdateColor;
public:
    DashDrawParametersUpdate(uint16_t fgColor_, uint16_t bgColor_, uint16_t fgUpdateColor_, uint16_t bgUpdateColor_,
                                 const GFXfont* font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
                                 DashDrawParameters(fgColor_, bgColor_, font_, align) {

        fgUpdateColor = fgUpdateColor_;
        bgUpdateColor = bgUpdateColor_;
    }

    uint16_t getBgColor(MenuItem* item, bool updated) override {
        return updated ? bgUpdateColor : bgColor;
    }

    uint16_t getFgColor(MenuItem* item, bool updated) override {
        return updated ? fgUpdateColor : fgColor;
    }
};

class DashDrawParametersIntUpdateRange : public DashDrawParametersUpdate {
public:
    struct IntColorRange {
        uint16_t fgColor;
        uint16_t bgColor;
        int minValue;
        int maxValue;
    };
private:
    const IntColorRange* colorRanges;
    int numOfRanges;
    bool useUpdateColor;
public:
    DashDrawParametersIntUpdateRange(uint16_t fgColor_, uint16_t bgColor_, uint16_t fgUpdateColor_, uint16_t bgUpdateColor_,
                                     const GFXfont *font_, const IntColorRange colorRanges_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align) {
        colorRanges = colorRanges_;
        numOfRanges = numberRanges;
        useUpdateColor = fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_;
    };

private:
    int findIndexForChoice(MenuItem* item) {
        if(isMenuBasedOnValueItem(item)) {
            auto val = reinterpret_cast<ValueMenuItem*>(item)->getCurrentValue();
            for(int i=0;i<numOfRanges;i++) {
                if(val >= colorRanges[i].minValue && val <= colorRanges[i].maxValue) {
                    return i;
                }
            }
        }
        return -1;
    }

    uint16_t getBgColor(MenuItem *item, bool updated) override {
        if(useUpdateColor && updated) return DashDrawParametersUpdate::getBgColor(item, updated);
        auto idx = findIndexForChoice(item);
        return (idx != -1) ? colorRanges[idx].bgColor : bgColor;
    }

    uint16_t getFgColor(MenuItem *item, bool updated) override {
        if(useUpdateColor && updated) return DashDrawParametersUpdate::getFgColor(item, updated);
        auto idx = findIndexForChoice(item);
        return (idx != -1) ? colorRanges[idx].fgColor : fgColor;
    }
};

class DashDrawParametersTextUpdateRange : public DashDrawParametersUpdate {
public:
    struct TextColorOverride {
        const char* text;
        uint16_t fgColor;
        uint16_t bgColor;
    };
private:
    const TextColorOverride* colorOverrides;
    int numOfRanges;
    bool useUpdateColor;
public:
    DashDrawParametersTextUpdateRange(uint16_t fgColor_, uint16_t bgColor_, uint16_t fgUpdateColor_, uint16_t bgUpdateColor_,
                                     const GFXfont *font_, const TextColorOverride colorOverrides_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align) {
        colorOverrides = colorOverrides_;
        numOfRanges = numberRanges;
        useUpdateColor = fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_;

    };

private:
    int findIndexForChoice(MenuItem* item) {
        if(isMenuRuntime(item)) {
            auto rtMenu = reinterpret_cast<RuntimeMenuItem*>(item);
            for(int i=0;i<numOfRanges;i++) {
                char sz[32];
                rtMenu->copyValue(sz, sizeof sz);
                if(strcmp(colorOverrides[i].text, sz) == 0) {
                    return i;
                }
            }
        }
        return -1;
    }

    uint16_t getBgColor(MenuItem *item, bool updated) override {
        if(useUpdateColor && updated) return DashDrawParametersUpdate::getBgColor(item, updated);
        auto idx = findIndexForChoice(item);
        return (idx != -1) ? colorOverrides[idx].bgColor : bgColor;
    }

    uint16_t getFgColor(MenuItem *item, bool updated) override {
        if(useUpdateColor && updated) return DashDrawParametersUpdate::getFgColor(item, updated);
        auto idx = findIndexForChoice(item);
        return (idx != -1) ? colorOverrides[idx].fgColor : fgColor;
    }
};

class DashMenuItem {
private:
    MenuItem *item;
    Coord screenLoc;
    DashDrawParameters *parameters;
    int updateCountDown;
    Coord titleExtents;
    int numChars;
    int valueWidth;
    char titleText[12];
public:
    DashMenuItem() : screenLoc(0, 0), titleExtents(0, 0) {
        parameters = nullptr;
        item = nullptr;
        updateCountDown = 0;
        numChars = 0;
        valueWidth = 0;
        titleText[0]=0;
    }

    DashMenuItem(const DashMenuItem &other) : screenLoc(other.screenLoc), titleExtents(0, 0) {
        item = other.item;
        parameters = other.parameters;
        updateCountDown = other.updateCountDown;
        numChars = other.numChars;
        valueWidth = 0;
        titleText[0] = 0;
    }

    DashMenuItem& operator= (const DashMenuItem& other) = default;

    DashMenuItem(MenuItem *theItem, Coord topLeft, DashDrawParameters* params, int numCharsInValue, const char* titleOverride)
            : screenLoc(topLeft), titleExtents(0, 0) {
        item = theItem;
        parameters = params;
        updateCountDown = UPDATE_COUNTDOWN;
        numChars = numCharsInValue;
        valueWidth = 0;
        if(titleOverride) {
            strncpy(titleText, titleOverride, sizeof(titleText));
        }
        else {
            theItem->copyNameToBuffer(titleText, sizeof(titleText));
        }
        titleText[sizeof(titleText)-1] = 0; // make sure it's null terminated.
    }

    uint16_t getKey() const {
        return item != nullptr ? item->getId() : 0;
    }

    bool needsPainting() {
        if (item == nullptr) return false;

        if(item->isChanged()) updateCountDown = UPDATE_COUNTDOWN;

        if(updateCountDown > 0) --updateCountDown;
        return item->isChanged() || updateCountDown != 0;
    }

    void paintTitle(Adafruit_GFX *myGfx) {
        myGfx->setFont(parameters->getFont());
        int baseline;
        titleExtents = gfxDrawable.textExtents(parameters->getFont(), 1, titleText, &baseline);
        valueWidth = gfxDrawable.textExtents(parameters->getFont(), 1,"0", &baseline).x * numChars;
        valueWidth = int(valueWidth * 1.20);

        if(!parameters->isTitleDrawn()) return;
        
        auto startX = (parameters->isTitleLeftAlign()) ? screenLoc.x : screenLoc.x + valueWidth + 1;

        if(parameters->getTitleBgColor(item, false) != ILI9341_BLACK) {
            gfx.fillRect(startX, screenLoc.y, titleExtents.x, titleExtents.y, parameters->getTitleBgColor(item, false));
        }
        myGfx->setTextColor(parameters->getTitleFgColor(item, false));
        myGfx->setCursor(startX, screenLoc.y + titleExtents.y);
        myGfx->print(titleText);

    }

    void paintItem(Adafruit_GFX *myGfx, GFXcanvas1* canvas, BaseMenuRenderer *renderer) {
        item->setChanged(false);
        char sz[20];
        copyMenuItemValue(item, sz, sizeof(sz));
        canvas->fillScreen(0);
        canvas->setFont(parameters->getFont());
        auto padding = 0;
        if(!parameters->isValueLeftAlign()) {
            int baseline;
            Coord valueLen = gfxDrawable.textExtents(parameters->getFont(), 1, sz, &baseline);
            padding = valueWidth - (valueLen.x + 4);
        }
        canvas->setCursor(padding, titleExtents.y - 1);
        canvas->print(sz);
        auto startX = (parameters->isTitleLeftAlign()) ? screenLoc.x + titleExtents.x + 5 : screenLoc.x;
        drawCookieCutBitmap(myGfx, startX, screenLoc.y, canvas->getBuffer(), valueWidth, titleExtents.y,
                canvas->width(), 0, 0,parameters->getFgColor(item, updateCountDown > 1),
                parameters->getBgColor(item, updateCountDown > 1));
    }

};

/**
 * This is the custom rendering class, that is called back in a game loop by the renderer. It is our
 * job to render any changes since last time to the screen in this loop. It's also responsible for
 * handling the reset event when the display times out.
 */
class DashCustomDrawing : public CustomDrawing {
private:
    uint16_t ledColors[LED_STATES];
    bool ledsChanged = true;
    BaseMenuRenderer *renderer;
    Adafruit_GFX *myGfx;
    bool running;
    bool wantLeds;
    BtreeList<uint16_t, DashMenuItem> drawingItems;
    GFXcanvas1 *canvas;
public:
    DashCustomDrawing(Adafruit_GFX *gfx, bool drawLeds = true) : drawingItems() {
        serdebugF("construct drawing")
        myGfx = gfx;
        ledsChanged = running = false;
        renderer = nullptr;
        wantLeds = drawLeds;
        canvas = new GFXcanvas1(gfx->width() / 2, gfx->height());
        for (uint16_t &ledState : ledColors) ledState = 0;
    }

    ~DashCustomDrawing() override = default;

    void clearItems() {
        drawingItems.clear();
    }

    void addDrawingItem(MenuItem *theItem, Coord topLeft, DashDrawParameters* params, int numCharsInValue, const char* titleOverrideText = nullptr) {
        drawingItems.add(DashMenuItem(theItem, topLeft, params, numCharsInValue, titleOverrideText));
        serdebugF2("added item to list #", drawingItems.count())
    }

    void setLed(int i, uint16_t color) {
        if (i < 0 || i > LED_STATES) return;
        ledColors[i] = color;
        ledsChanged = wantLeds;
    }

    void stop() {
        running = false;
        renderer->giveBackDisplay();
    }

    void reset() override {
        if (!running) renderer->takeOverDisplay();
    }

    void started(BaseMenuRenderer *currentRenderer) override {
        renderer = currentRenderer;
        myGfx->setTextSize(1);
        myGfx->fillScreen(0);
        ledsChanged = wantLeds;
        serdebugF2("drawing titles #", drawingItems.count())

        for (int i = 0; i < drawingItems.count(); i++) {
            auto drawing = drawingItems.itemAtIndex(i);
            serdebugF("drawing title")

            drawing->paintTitle(myGfx);
        }
        serdebugF("started2")

    }

    /**
     * actually do the drawing, this is essentially the runloop. Work out what's changed and draw it.
     *
     * @param currentValue This is the encoder position if the menu is using an encoder
     * @param userClicked this represents the status of the select button, see RenderPressMode for more details
     */
    void renderLoop(unsigned int currentValue, RenderPressMode userClick) override {
        if (userClick != RPRESS_NONE) {
            stop();
            return;
        }
        if (ledsChanged) drawLeds();

        for(int i = 0; i < drawingItems.count(); i++) {
            auto drawing = drawingItems.itemAtIndex(i);
            if(drawing->needsPainting()) {
                drawing->paintItem(myGfx, canvas, renderer);
            }
        }
    }

    /**
     * Redraw the LEDs if they have changed
     */
    void drawLeds() {
        ledsChanged = false;
        int widthOfOneLed = myGfx->width() / LED_STATES;

        int offsetX = widthOfOneLed / 2;
        for (uint16_t ledState : ledColors) {
            myGfx->fillCircle(offsetX, 12, 11, ledState);
            offsetX += widthOfOneLed;
        }
    }
};

#endif //TCMENU_EXAMPLE_DASHCUSTOMDRAWING_H
