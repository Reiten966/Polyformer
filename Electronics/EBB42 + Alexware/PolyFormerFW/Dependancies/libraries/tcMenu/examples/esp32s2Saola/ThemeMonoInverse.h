#ifndef TCMENU_THEME_MONO_INVERSE
#define TCMENU_THEME_MONO_INVERSE

color_t defaultItemPaletteMono[] = {1, 0, 1, 1};
color_t defaultTitlePaletteMono[] = {0, 1, 0, 0};

#define TITLE_PADDING 2
#define TITLE_SPACING 2

void installMonoInverseTitleTheme(GraphicsDeviceRenderer& bgr, const MenuFontDef& itemFont, const MenuFontDef& titleFont, bool needEditingIcons) {
    bgr.setDisplayDimensions(bgr.getDeviceDrawable()->getDisplayDimensions().x, bgr.getDeviceDrawable()->getDisplayDimensions().y);
    auto& factory = bgr.getGraphicsPropertiesFactory();

    factory.setSelectedColors(0, 2);

    MenuPadding titlePadding(TITLE_PADDING);
    MenuPadding itemPadding(1);
    int titleHeight = bgr.heightForFontPadding(titleFont.fontData, titleFont.fontMag, titlePadding);
    int itemHeight = bgr.heightForFontPadding(itemFont.fontData, itemFont.fontMag, itemPadding);

    factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(8, 6),DrawableIcon::ICON_XBITMAP, loResEditingIcon));
    factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(8, 6),DrawableIcon::ICON_XBITMAP, loResActiveIcon));

    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, defaultTitlePaletteMono, titlePadding, titleFont.fontData, titleFont.fontMag,
                                        TITLE_SPACING, titleHeight + 1, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, defaultItemPaletteMono, itemPadding, itemFont.fontData, itemFont.fontMag,
                                        1, itemHeight, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT , MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, defaultItemPaletteMono, itemPadding, itemFont.fontData, itemFont.fontMag,
                                        1, itemHeight, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, MenuBorder(0));

    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();
}

#endif //TCMENU_THEME_MONO_INVERSE