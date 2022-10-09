/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseGraphicalRenderer.h"
#include "BaseDialog.h"
#include "RuntimeTitleMenuItem.h"

namespace tcgfx {


void BaseGraphicalRenderer::render() {
    checkIfRootHasChanged();

    uint8_t locRedrawMode = redrawMode;
    redrawMode = MENUDRAW_NO_CHANGE;

    drawingCommand(DRAW_COMMAND_START);

    if (locRedrawMode == MENUDRAW_COMPLETE_REDRAW) {
        serlogF(SER_TCMENU_DEBUG, "Complete redraw")
        drawingCommand(DRAW_COMMAND_CLEAR);
        taskManager.yieldForMicros(0);
    }

    countdownToDefaulting();

    bool forceDrawWidgets = false;

    // if the root menu rootItem has changed then it's a complete re-draw and we need to calculate the orders again.
    MenuItem* rootItem = menuMgr.getCurrentMenu();
    if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST ) {
        if (rootItem->isChanged() || locRedrawMode != MENUDRAW_NO_CHANGE) {
            renderList();
            forceDrawWidgets = true;
        }
        titleOnDisplay = (titleMode != NO_TITLE);
    }
    else if(itemOrderByRow.count()) {
        // first we find the first currently active rootItem in our single linked list
        int activeIndex = findActiveItem(rootItem);
        uint16_t totalHeight = calculateHeightTo(activeIndex, rootItem);
        int startRow = 0;
        uint16_t adjustedHeight = height + (lastRowExactFit ? 0 : 1);
        bool drawCompleteScreen = locRedrawMode != MENUDRAW_NO_CHANGE;
        uint16_t startY = 0;

        if(titleMode == TITLE_ALWAYS) {
            startRow++;
            startY = heightOfRow(0, true);
            adjustedHeight -= startY;
            totalHeight -= startY;
        }

        while (totalHeight > adjustedHeight) {
            totalHeight -= heightOfRow(startRow, true);
            startRow++;
        }

        // the screen has moved, we must completely redraw the area, and we need a clear first.
        if(locRedrawMode != MENUDRAW_COMPLETE_REDRAW && lastOffset != startRow) {
            locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
            serlogF3(SER_TCMENU_DEBUG, "Screen Row moved ", lastOffset, startRow);
            drawCompleteScreen = true;
        }
        lastOffset = startRow;

        if(titleMode == TITLE_ALWAYS && (drawCompleteScreen  || itemOrderByRow.itemAtIndex(0)->getMenuItem()->isChanged())) {
            auto* pEntry = itemOrderByRow.itemAtIndex(0);
            drawMenuItem(pEntry, Coord(0,0), Coord(width, startY), drawCompleteScreen);
            forceDrawWidgets = true;
        }

        // and then we start drawing items until we run out of screen or items
        if(drawTheMenuItems(startRow, startY, drawCompleteScreen)) forceDrawWidgets = true;
        titleOnDisplay = (titleMode == TITLE_FIRST_ROW && startRow == 0) || titleMode == TITLE_ALWAYS;
    }

    redrawAllWidgets(locRedrawMode != MENUDRAW_NO_CHANGE || forceDrawWidgets);
    drawingCommand(DRAW_COMMAND_ENDED);
}

GridPositionRowCacheEntry* BaseGraphicalRenderer::findMenuEntryAndDimensions(const Coord& screenPos, Coord& localStart, Coord& localSize) {
    if((dialog!=nullptr && dialog->isRenderNeeded()) || displayTakenMode != NOT_TAKEN_OVER) {
        return nullptr;
    }

    if(currentRootMenu && currentRootMenu->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        auto* runList = reinterpret_cast<ListRuntimeMenuItem*>(currentRootMenu);
        auto* itemProps = getDisplayPropertiesFactory().configFor(runList, ItemDisplayProperties::COMPTYPE_ITEM);
        auto* titleProps = getDisplayPropertiesFactory().configFor(runList, ItemDisplayProperties::COMPTYPE_TITLE);
        int titleHeight = titleProps->getRequiredHeight() + titleProps->getSpaceAfter();
        int rowHeight = itemProps->getRequiredHeight() + + itemProps->getSpaceAfter();
        if(screenPos.y < titleHeight) {
            cachedEntryItem = GridPositionRowCacheEntry(runList, GridPosition(GridPosition::DRAW_TITLE_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, titleHeight), titleProps);
            return &cachedEntryItem;
        }
        else {
            auto rowNum = min(int((screenPos.y - titleHeight) / rowHeight), int(runList->getNumberOfRows() - 1));
            cachedEntryItem = GridPositionRowCacheEntry(runList, GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, rowNum + 1, titleHeight), titleProps);
            return &cachedEntryItem;
        }
    }

    // if we are in title always mode, then the title must always be calculated
    int rowStartY = 0;
    if(titleMode == TITLE_ALWAYS) {
        auto* titleProps = getDisplayPropertiesFactory().configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
        rowStartY = titleProps->getRequiredHeight() + titleProps->getSpaceAfter();
        if(screenPos.y <= rowStartY) {
            return itemOrderByRow.itemAtIndex(0);
        }
    }

    auto* icon = getDisplayPropertiesFactory().iconForMenuItem(SPECIAL_ID_ACTIVE_ICON);
    int iconWidth = icon ? icon->getDimensions().x : 0;
    uint8_t currentRow = -1;

    for(bsize_t i=lastOffset; i<itemOrderByRow.count(); i++) {
        auto* pEntry = itemOrderByRow.itemAtIndex(i);
        int rowHeight = heightOfRow(pEntry->getPosition().getRow());
        int rowEndY = rowStartY + rowHeight;

        // this seems odd but we are only doing this loop to find the heights, so we
        // only check all the first entries in the row. There is code further down to
        // deal with columns.
        if(pEntry->getPosition().getRow() == currentRow) continue;
        currentRow = i;

        // if we are within the y bounds of of item
        if(screenPos.y > rowStartY && screenPos.y < rowEndY) {
            localStart.y = rowStartY;
            localSize.y = rowHeight;
            if(pEntry->getPosition().getGridSize() == 1) {
                // single column row, so we must be within the item
                auto iconAdjust = iconWidth + pEntry->getDisplayProperties()->getPadding().left;
                localStart.x = iconAdjust;
                localSize.x = width - iconAdjust;
                return pEntry;
            }
            else {
                // multi column row, so we must work out which column we are in.
                int colWidth = width / pEntry->getPosition().getGridSize();
                int column = (screenPos.x / colWidth);
                localStart.x = column * colWidth;
                localSize.x = colWidth;
                return itemOrderByRow.getByKey(rowCol(pEntry->getPosition().getRow(), column + 1));
            }
        }
        rowStartY += rowHeight + pEntry->getDisplayProperties()->getSpaceAfter();
    }

    // we did not find anything at that point.
    return nullptr;
}

bool BaseGraphicalRenderer::drawTheMenuItems(int startRow, int startY, bool drawEveryLine) {
    uint16_t ypos = startY;
    int lastRow = 0;
    int addAmount = 0;
    bool didDrawTitle = false;
    uint16_t totalHeight = 0;

    for(bsize_t i=startRow; i < itemOrderByRow.count(); i++) {
        auto* itemCfg = itemOrderByRow.itemAtIndex(i);
        auto* item = itemCfg->getMenuItem();
        if(item->isVisible())
        {
            if(lastRow != itemCfg->getPosition().getRow()) ypos += addAmount;
            totalHeight = ypos + itemCfg->getHeight();
            auto extentsY = (lastRowExactFit) ? totalHeight : ypos;
            if(extentsY > height) break;

            if (drawEveryLine || item->isChanged()) {
                serlogF4(SER_TCMENU_DEBUG, "draw item (pos,id,chg)", i, item->getId(), item->isChanged());
                item->setChanged(false);
                taskManager.yieldForMicros(0);
                if(itemCfg->getPosition().getGridSize() > 1) {
                    int colWidth = width / itemCfg->getPosition().getGridSize();
                    int colOffset = colWidth * (itemCfg->getPosition().getGridPosition() - 1);
                    drawMenuItem(itemCfg, Coord(colOffset, ypos), Coord(colWidth - 1, itemCfg->getHeight()), drawEveryLine);
                }
                else {
                    drawMenuItem(itemCfg, Coord(0, ypos), Coord(width, itemCfg->getHeight()), drawEveryLine);
                }
                if(itemCfg->getPosition().getDrawingMode() == GridPosition::DRAW_TITLE_ITEM && itemCfg->getPosition().getRow() == 0) {
                    didDrawTitle = true;
                }
            }
            lastRow = itemCfg->getPosition().getRow();
            addAmount = itemCfg->getHeight() + itemCfg->getDisplayProperties()->getSpaceAfter();
        }

    }

    // and lastly, if we are drawing every line, we must clear down everything
    totalHeight++;
    if(drawEveryLine && totalHeight < height) {
        fillWithBackgroundTo(totalHeight);
    }

    return didDrawTitle;
}

void BaseGraphicalRenderer::renderList() {
    auto* runList = reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu());
    auto* itemProps = getDisplayPropertiesFactory().configFor(runList, ItemDisplayProperties::COMPTYPE_ITEM);
    auto* titleProps = getDisplayPropertiesFactory().configFor(runList, ItemDisplayProperties::COMPTYPE_TITLE);
    int titleHeight = titleProps->getRequiredHeight();
    int rowHeight = itemProps->getRequiredHeight();
    int totalTitleHeight = titleHeight + titleProps->getSpaceAfter();
    int totalRowHeight = rowHeight + itemProps->getSpaceAfter();

    runList->setActive(true);

    uint8_t maxOnScreen = ((height + (rowHeight - 1)) - totalTitleHeight) / totalRowHeight;
    uint8_t currentActive = runList->getActiveIndex();

    uint8_t offset = 0;
    while((currentActive - offset) > maxOnScreen) offset++;

    cachedEntryItem = GridPositionRowCacheEntry(runList->asBackMenu(), GridPosition(GridPosition::DRAW_TITLE_ITEM,
                                                                             titleProps->getDefaultJustification(),
                                                                             0, titleHeight), titleProps);
    drawMenuItem(&cachedEntryItem, Coord(0, 0), Coord(width, titleHeight), true);

    for (int i = 0; i <= maxOnScreen; i++) {
        uint8_t current = offset + i;
        if(current >= runList->getNumberOfRows()) break;
        RuntimeMenuItem* toDraw = runList->getChildItem(current);
        cachedEntryItem = GridPositionRowCacheEntry(toDraw, GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, current + 1, rowHeight), itemProps);
        drawMenuItem(&cachedEntryItem, Coord(0, totalTitleHeight), Coord(width, rowHeight), true);
        taskManager.yieldForMicros(0);
        totalTitleHeight += totalRowHeight;
    }

    fillWithBackgroundTo(totalTitleHeight);

    // reset the list item to a normal list again.
    runList->asParent();
    runList->setChanged(false);
    titleOnDisplay = true;
}

GridPosition::GridDrawingMode modeFromItem(MenuItem* item, bool useSlider) {
    switch(item->getMenuType()) {
        case MENUTYPE_INT_VALUE:
            return useSlider ? GridPosition::DRAW_INTEGER_AS_SCROLL : GridPosition::DRAW_INTEGER_AS_UP_DOWN;
        case MENUTYPE_ENUM_VALUE:
        case MENUTYPE_SCROLLER_VALUE:
            return GridPosition::DRAW_INTEGER_AS_UP_DOWN;
        default:
            return GridPosition::DRAW_TEXTUAL_ITEM;
    }
}

void BaseGraphicalRenderer::checkIfRootHasChanged() {
    auto* rootItem = menuMgr.getCurrentMenu();
    if(currentRootMenu != rootItem)
    {
        serlogF(SER_TCMENU_INFO, "root has changed");
        currentRootMenu = rootItem;
        redrawMode = MENUDRAW_COMPLETE_REDRAW;
        recalculateDisplayOrder(rootItem, false);
    }
}

void BaseGraphicalRenderer::recalculateDisplayOrder(MenuItem *root, bool safeMode) {
    serlogF2(SER_TCMENU_INFO, "Recalculate display order, safe=", safeMode);

    // implicitly must be changing menus in this case, so reset the current root menu.
    currentRootMenu = root;
    itemOrderByRow.clear();
    if(root == nullptr || root->getMenuType() == MENUTYPE_RUNTIME_LIST) return;

    if(root == menuMgr.getRoot() && titleMode != NO_TITLE) {
        serlogF(SER_TCMENU_DEBUG, "Add title");
        auto* myProps = getDisplayPropertiesFactory().configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
        appTitleMenuItem.setTitleHeaderPgm(pgmTitle);
        itemOrderByRow.add(GridPositionRowCacheEntry(&appTitleMenuItem, GridPosition(GridPosition::DRAW_TITLE_ITEM, myProps->getDefaultJustification(), 0), myProps));
    }

    auto* item = root;
    if(root->getMenuType() == MENUTYPE_BACK_VALUE) {
        serlogF2(SER_TCMENU_DEBUG, "Handling back item", root->getId());
        auto* myProps = getDisplayPropertiesFactory().configFor(root, ItemDisplayProperties::COMPTYPE_TITLE);
        itemOrderByRow.add(GridPositionRowCacheEntry(root, GridPosition(GridPosition::DRAW_TITLE_ITEM, myProps->getDefaultJustification(), 0), myProps));
        item = root->getNext();
    }

    while(item != nullptr) {
        if(item->isVisible()) {
            auto* conf = getDisplayPropertiesFactory().gridPositionForItem(item);
            if (conf && !safeMode) {
                serlogF3(SER_TCMENU_DEBUG, "Add config id at row", item->getId(), conf->getPosition().getRow());
                auto compType = toComponentType(conf->getPosition().getDrawingMode(), item);
                itemOrderByRow.add(GridPositionRowCacheEntry(item, conf->getPosition(), getDisplayPropertiesFactory().configFor(item, compType)));
            } else {
                // We just find the first unused row and put the next item there.
                int row = 0;
                while(itemOrderByRow.getByKey(rowCol(row, 1)) != nullptr) row++;
                serlogF3(SER_TCMENU_DEBUG, "Add manual id at row", item->getId(), row);
                auto mode = modeFromItem(item, useSliderForAnalog);
                auto* itemProps = getDisplayPropertiesFactory().configFor(item, toComponentType(mode, item));
                itemOrderByRow.add(GridPositionRowCacheEntry(item, GridPosition(mode, itemProps->getDefaultJustification(), 1, 1, row, 0), itemProps));
            }
        }
        item = item->getNext();
    }

    // here we try again, but this time we turn off any custom grids.
    if(areRowsOutOfOrder() && !safeMode) {
        recalculateDisplayOrder(root, true);
    }
}


bool BaseGraphicalRenderer::areRowsOutOfOrder() {
    int lastSequence = 0;
    for(bsize_t i=0; i<itemOrderByRow.count();i++) {
        if(itemOrderByRow.itemAtIndex(i)->getPosition().getRow() > (lastSequence + 1)) {
            char sz[20];
            itemOrderByRow.itemAtIndex(i)->getMenuItem()->copyNameToBuffer(sz, sizeof sz);
            serlogF2(SER_TCMENU_INFO, "Row out of order at ", sz);
            return true;
        }
        lastSequence = itemOrderByRow.itemAtIndex(i)->getPosition().getRow();
    }
    return false;
}

void BaseGraphicalRenderer::redrawAllWidgets(bool forceRedraw) {
    if(!titleOnDisplay || itemOrderByRow.count() == 0) return;
    if(itemOrderByRow.itemAtIndex(0)->getPosition().getDrawingMode() != GridPosition::DRAW_TITLE_ITEM) return;

    auto* displayProps = itemOrderByRow.itemAtIndex(0)->getDisplayProperties();
    auto widFg = displayProps->getColor(ItemDisplayProperties::HIGHLIGHT1);
    auto widBg = displayProps->getColor(ItemDisplayProperties::BACKGROUND);

    auto* widget = this->firstWidget;
    int widgetRight = width;
    while(widget) {
        widgetRight = widgetRight - (displayProps->getPadding().right + widget->getWidth());
        if(widget->isChanged() || forceRedraw) {
            widget->setChanged(false);
            drawWidget(Coord(widgetRight, displayProps->getPadding().top), widget, widFg, widBg);
        }
        widget = widget->getNext();
    }
}

int BaseGraphicalRenderer::heightOfRow(int row, bool includeSpace) {
    uint8_t i = 1;
    GridPositionRowCacheEntry *rowData = nullptr;
    while(rowData == nullptr && i < 5) {
        rowData = itemOrderByRow.getByKey(rowCol(row, i));
        i++;
    }

    // if there's no configuration at this position, crazy situation, return 1..
    if(rowData == nullptr) return 1;

    // if there's a configuration, then return it, unless it's 0 then use the default
    auto itemHeight = rowData->getPosition().getGridHeight();
    auto actualHeight = itemHeight != 0 ? itemHeight : rowData->getDisplayProperties()->getRequiredHeight();
    if(includeSpace) actualHeight += rowData->getDisplayProperties()->getSpaceAfter();
    return actualHeight;
}

int BaseGraphicalRenderer::calculateHeightTo(int index, MenuItem *pItem) {
    int totalY = 0;
    index += 1;
    for(int i=0; i<index; i++) {
        totalY += heightOfRow(i, true);
    }
    return totalY;
}

uint8_t BaseGraphicalRenderer::itemCount(MenuItem*, bool ) {
    checkIfRootHasChanged();
    if(currentRootMenu && currentRootMenu->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        auto* listItem = reinterpret_cast<ListRuntimeMenuItem*>(currentRootMenu);
        return listItem->getNumberOfRows() + 1; // accounts for title.
    }
    else {
        return itemOrderByRow.count();
    }
}

int BaseGraphicalRenderer::findActiveItem(MenuItem* root) {
    checkIfRootHasChanged();
    if(currentRootMenu && currentRootMenu->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        auto* listItem = reinterpret_cast<ListRuntimeMenuItem*>(currentRootMenu);
        return listItem->getActiveIndex(); // accounts for title.
    }
    for(bsize_t i=0;i<itemOrderByRow.count();i++) {
        auto* possibleActive = itemOrderByRow.itemAtIndex(i);
        if(possibleActive->getMenuItem()->isActive()) return possibleActive->getPosition().getRow();
    }
    return 0; // default to the title (back menu item)
}

MenuItem *BaseGraphicalRenderer::getMenuItemAtIndex(MenuItem* item, uint8_t idx) {
    checkIfRootHasChanged();
    if(currentRootMenu && currentRootMenu->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        return currentRootMenu;
    }
    if(idx >= itemOrderByRow.count()) return menuMgr.getRoot();
    return itemOrderByRow.itemAtIndex(idx)->getMenuItem();
}

ItemDisplayProperties::ComponentType BaseGraphicalRenderer::toComponentType(GridPosition::GridDrawingMode mode, MenuItem* pMenuItem) {
    if(!pMenuItem) return ItemDisplayProperties::COMPTYPE_ITEM;
    switch(mode) {
        case GridPosition::DRAW_TITLE_ITEM:
            return ItemDisplayProperties::COMPTYPE_TITLE;
        case GridPosition::DRAW_TEXTUAL_ITEM:
        case GridPosition::DRAW_INTEGER_AS_UP_DOWN:
        case GridPosition::DRAW_INTEGER_AS_SCROLL:
        case GridPosition::DRAW_AS_ICON_ONLY:
        case GridPosition::DRAW_AS_ICON_TEXT:
        default:
            return isItemActionable(pMenuItem) || pMenuItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE
                   ? ItemDisplayProperties::COMPTYPE_ACTION : ItemDisplayProperties::COMPTYPE_ITEM;
    }
}

BaseDialog* BaseGraphicalRenderer::getDialog() {
    if(dialog == nullptr) {
        dialog = new MenuBasedDialog();
    }
    return dialog;
}

void preparePropertiesFromConfig(ConfigurableItemDisplayPropertiesFactory& factory, const ColorGfxMenuConfig<const void*>* gfxConfig, int titleHeight, int itemHeight) {
    // TEXT, BACKGROUND, HIGHLIGHT1, HIGHLIGHT2, SELECTED_FG, SELECTED_BG
    color_t paletteItems[] { gfxConfig->fgItemColor, gfxConfig->bgItemColor, gfxConfig->bgSelectColor, gfxConfig->fgSelectColor};
    color_t titleItems[] { gfxConfig->fgTitleColor, gfxConfig->bgTitleColor, gfxConfig->fgTitleColor, gfxConfig->fgSelectColor};

    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, paletteItems, gfxConfig->itemPadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification, 0, itemHeight, GridPosition::JUSTIFY_LEFT_NO_VALUE, MenuBorder());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, paletteItems, gfxConfig->itemPadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification, 0, itemHeight, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, titleItems, gfxConfig->titlePadding, gfxConfig->titleFont, gfxConfig->titleFontMagnification, gfxConfig->titleBottomMargin, titleHeight, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, MenuBorder());
    factory.setSelectedColors(gfxConfig->bgSelectColor, gfxConfig->fgSelectColor);

    factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(gfxConfig->editIconWidth, gfxConfig->editIconHeight), DrawableIcon::ICON_XBITMAP, gfxConfig->editIcon));
    factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(gfxConfig->editIconWidth, gfxConfig->editIconHeight), DrawableIcon::ICON_XBITMAP, gfxConfig->activeIcon));

    ConfigurableItemDisplayPropertiesFactory::refreshCache();
}

} // namespace tcgfx
