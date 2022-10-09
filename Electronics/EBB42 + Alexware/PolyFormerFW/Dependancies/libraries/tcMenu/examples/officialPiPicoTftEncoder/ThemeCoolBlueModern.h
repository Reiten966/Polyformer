/**
 * Cool blue modern theme by TheCodersCorner.com. This is part of the standard themes shipped with TcMenu.
 * This file will not be updated by the designer, you can edit.
 */
#ifndef THEME_COOL_BLUE
#define THEME_COOL_BLUE

#include <graphics/BaseGraphicalRenderer.h>

const color_t coolBlueTitlePalette[] = {RGB(0,0,0), RGB(20,132,255), RGB(192,192,192), RGB(64, 64, 64)};
const color_t coolBlueItemPalette[] = {RGB(255, 255, 255), RGB(0,64,135), RGB(20,133,255), RGB(31,100,178)};
const color_t coolBlueActionPalette[] = {RGB(255, 255, 255), RGB(0,45,120), RGB(20,133,255), RGB(31,100,178)};

#define ACTION_BORDER_WIDTH 0

void installCoolBlueModernTheme(GraphicsDeviceRenderer& bgr, const MenuFontDef& itemFont, const MenuFontDef& titleFont, bool needEditingIcons) {
    // here we get a refrerence to the drawable and then set the dimensions.
    auto* rootDrawable = bgr.getDeviceDrawable();
    bgr.setDisplayDimensions(rootDrawable->getDisplayDimensions().x, rootDrawable->getDisplayDimensions().y);

    // we need a reference to the factory object that we will use to configure the drawing.
    auto& factory = bgr.getGraphicsPropertiesFactory();

    // change the selected colours.
    factory.setSelectedColors(RGB(31, 88, 100), RGB(255, 255, 255));

    // for this theme we use the same size padding for each case, we need touchable items. We calculate the height too
    MenuPadding allPadding(4, 3, 4, 3);
    int titleHeight = bgr.heightForFontPadding(titleFont.fontData, titleFont.fontMag, allPadding);
    int itemHeight = bgr.heightForFontPadding(itemFont.fontData, itemFont.fontMag, allPadding);

    // now we configure the drawing for each item type
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, coolBlueTitlePalette, allPadding, titleFont.fontData, titleFont.fontMag, 3, titleHeight,
                                        GridPosition::JUSTIFY_CENTER_WITH_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, coolBlueItemPalette, allPadding, itemFont.fontData, itemFont.fontMag, 2, itemHeight,
                                        GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT , MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, coolBlueActionPalette, allPadding, itemFont.fontData, itemFont.fontMag, 2, itemHeight,
                                        GridPosition::JUSTIFY_CENTER_WITH_VALUE, MenuBorder(ACTION_BORDER_WIDTH));

    // and lastly, whenever changing the configuration, we must refresh.
    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();
}

#endif //THEME_COOL_BLUE