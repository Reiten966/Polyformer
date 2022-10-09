/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include "GfxMenuConfig.h"
#include "BaseGraphicalRenderer.h"
#include "MenuIterator.h"
#include "tcMenu.h"

/**
 * The default editing icon for approx 100-150 dpi resolution displays
 */
const uint8_t defEditingIcon[] PROGMEM = {
        0x00, 0x08, 0x00, 0x1c, 0x00, 0x3e, 0x00, 0x1d, 0x80, 0x08, 0x40, 0x04,
        0x20, 0x02, 0x10, 0x01, 0x88, 0x00, 0x44, 0x00, 0x2c, 0x00, 0x1c, 0x00
};

/**
 * The default active icon for approx 100-150 dpi resolution displays
 */
const uint8_t defActiveIcon[] PROGMEM = {
        0x00, 0x01, 0x80, 0x03, 0x80, 0x07, 0x80, 0x0f, 0x80, 0x1f, 0xff, 0x3f,
        0xff, 0x3f, 0x80, 0x1f, 0x80, 0x0f, 0x80, 0x07, 0x80, 0x03, 0x00, 0x01
};

/**
 * The low resolution icon for editing status
 */
const uint8_t loResEditingIcon[] PROGMEM = { 0x7c, 0x06, 0x18, 0x18, 0x06, 0x7c };

/**
 * The low resolution icon for indicating active status
 */
const uint8_t loResActiveIcon[] PROGMEM = { 0x20, 0x60, 0xfe, 0xfe, 0x60, 0x20 };

namespace tcgfx {

/**
 * @deprecated don't use in new designs, for backward compatibility, may be removed in future.
 * @param config the config object to fill in.
 */
void prepareDefaultGfxConfig(ColorGfxMenuConfig<void*>* config) {
    makePadding(config->titlePadding, 5, 5, 20, 5);
    makePadding(config->itemPadding, 5, 5, 3, 5);
    makePadding(config->widgetPadding, 5, 10, 0, 5);

    config->bgTitleColor = RGB(255, 255, 0);
    config->fgTitleColor = RGB(0, 0, 0);
    config->titleFont = NULL;
    config->titleBottomMargin = 10;

    config->bgItemColor = RGB(0, 0, 0);
    config->fgItemColor = RGB(222, 222, 222);
    config->itemFont = NULL;

    config->bgSelectColor = RGB(0, 0, 200);
    config->fgSelectColor = RGB(255, 255, 255);
    config->widgetColor = RGB(30, 30, 30);

    config->titleFontMagnification = 4;
    config->itemFontMagnification = 2;
}

ItemDisplayProperties *ConfigurableItemDisplayPropertiesFactory::configFor(MenuItem *pItem, ItemDisplayProperties::ComponentType compType) {
    // make sure that we never return null, in the worst case, provide a default row for this.
    if(displayProperties.count()==0) {
        color_t defaultColors[] = { RGB(255,255,255), RGB(0,0,0), RGB(192, 192, 192), RGB(255, 255, 255)};
        setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, defaultColors, MenuPadding(2), nullptr,
                                    1, 2, 12, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder());
    }

    ItemDisplayProperties *pConf = nullptr;
    if(pItem != nullptr) {
        pConf = displayProperties.getByKey(MakePropsKey(pItem->getId(), false, compType));
        if(pConf) return pConf;
        auto* pSubMenu = menuMgr.getCurrentSubMenu();
        uint16_t subId = pSubMenu ? pSubMenu->getId() : 0;
        pConf = displayProperties.getByKey(MakePropsKey(subId, true, compType));
        if(pConf) return pConf;
    }
    pConf = displayProperties.getByKey(MakePropsKey(MENUID_NOTSET, false, compType));
    if(pConf) return pConf;
    return displayProperties.getByKey(MakePropsKey(MENUID_NOTSET, false, ItemDisplayProperties::COMPTYPE_ITEM));

}

void ConfigurableItemDisplayPropertiesFactory::setDrawingProperties(uint32_t key, const color_t *palette, MenuPadding pad,
                                                                    const void *font, uint8_t mag, uint8_t spacing,
                                                                    uint8_t requiredHeight, GridPosition::GridJustification just,
                                                                    MenuBorder border) {
    auto* pDisplayProperties = displayProperties.getByKey(key);
    if(pDisplayProperties) {
        pDisplayProperties->setColors(palette);
        pDisplayProperties->setFontInfo(font, mag);
        pDisplayProperties->setPadding(pad);
        pDisplayProperties->setRequiredHeight(requiredHeight);
        pDisplayProperties->setSpaceAfter(spacing);
        pDisplayProperties->setDefaultJustification(just);
        pDisplayProperties->setBorder(border);
    }
    else {
        displayProperties.add(ItemDisplayProperties(key, palette, pad, font, mag, spacing, requiredHeight, just, border));
    }
}

    void ConfigurableItemDisplayPropertiesFactory::refreshCache() {
        if(BaseMenuRenderer::getInstance()->getRendererType() == RENDER_TYPE_CONFIGURABLE) {
            reinterpret_cast<BaseGraphicalRenderer *>(MenuRenderer::getInstance())->displayPropertiesHaveChanged();
        }
    }

} // namespace
