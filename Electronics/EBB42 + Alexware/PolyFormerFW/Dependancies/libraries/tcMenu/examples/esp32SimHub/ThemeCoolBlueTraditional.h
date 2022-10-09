#ifndef THEME_COOL_BLUE
#define THEME_COOL_BLUE

//
// Note only include this file ONCE, in a CPP file. We do this automatically when using a Theme by adding to setupMenu()
//

// tcMenu drawing properties take a 4 color palette for items, titles and actions.
// this renderer shares the color configuration for items and actions.
const color_t coolBlueTitlePalette[] = {RGB(0,0,0), RGB(20,132,255), RGB(192,192,192), RGB(64, 64, 64)};
const color_t coolBlueItemPalette[] = {RGB(255, 255, 255), RGB(0,64,135), RGB(20,133,255), RGB(31,100,178)};

void installCoolBlueTraditionalTheme(GraphicsDeviceRenderer& bgr, const MenuFontDef& itemFont, const MenuFontDef& titleFont, bool needEditingIcons) {
    // first we keep a reference to the screen size, and set the dimensions on the renderer.
    auto width = bgr.getDeviceDrawable()->getDisplayDimensions().x;
    auto height = bgr.getDeviceDrawable()->getDisplayDimensions().y;
    bgr.setDisplayDimensions(width, height);

    // get hold of the item display factory that holds the drawing configuration.
    auto& factory = bgr.getGraphicsPropertiesFactory();

    // when an item is active, it will show in these colours instead of the default.
    factory.setSelectedColors(RGB(31, 88, 100), RGB(255, 255, 255));

    // here we calculate the item padding and row heights based on the resolution of the display
    bool medResOrBetter = width > 160;
    MenuPadding titlePadding(medResOrBetter ? 4 : 2);
    MenuPadding itemPadding(medResOrBetter ? 2 : 1);
    int titleHeight = bgr.heightForFontPadding(titleFont.fontData, titleFont.fontMag, titlePadding);
    int itemHeight = bgr.heightForFontPadding(itemFont.fontData, itemFont.fontMag, itemPadding);

    // we set the editing and selected icons here based on the row height.
    if(needEditingIcons && itemHeight > 12) {
        factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(16, 12),DrawableIcon::ICON_XBITMAP, defEditingIcon));
        factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(16, 12),DrawableIcon::ICON_XBITMAP, defActiveIcon));
    }
    else if(needEditingIcons) {
        factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(8, 6),DrawableIcon::ICON_XBITMAP, loResEditingIcon));
        factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(8, 6),DrawableIcon::ICON_XBITMAP, loResActiveIcon));
    }

    // we tell the library how to draw titles, items and actions by default.
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, coolBlueTitlePalette, titlePadding, titleFont.fontData, titleFont.fontMag,
                                        medResOrBetter ? 3 : 1, titleHeight, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, MenuBorder());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, coolBlueItemPalette, itemPadding, itemFont.fontData, itemFont.fontMag,
                                        1, itemHeight, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, coolBlueItemPalette, itemPadding, itemFont.fontData, itemFont.fontMag,
                                        1, itemHeight, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder());

    // after adjusting the drawing configuration, we must always refresh the cache.
    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();
}

#endif //THEME_COOL_BLUE
