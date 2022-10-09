/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <IoLogging.h>
#include "MenuHistoryNavigator.h"
#include "MenuIterator.h"
#include "RuntimeMenuItem.h"

void tcnav::MenuNavigationStore::setRootItem(MenuItem *item) {
    root = item;
    currentRoot = root;
    currentSub = nullptr;
    navIdx = 0;
    currentIsCustom = false;
}

void tcnav::MenuNavigationStore::navigateTo(MenuItem *activeItem, MenuItem *newRoot, bool custom) {
    // we only stack when
    if(navIdx < NAV_ITEM_ARRAY_SIZE && !currentIsCustom) {
        navItems[navIdx] = currentRoot;
        activeItems[navIdx] = activeItem ? activeItem : currentRoot;
        serlogF4(SER_TCMENU_INFO, "NavigateTo pushed ", navIdx, currentRoot->getId(), activeItems[navIdx]->getId());
        currentIsCustom = custom;
        navIdx++;
    }
    else {
        serlogF(SER_ERROR, "Nav exceeded");
    }
    currentRoot = newRoot;
    currentSub = getSubMenuFor(newRoot);
}

MenuItem *tcnav::MenuNavigationStore::popNavigationGetActive() {
    currentIsCustom = false;
    if(navIdx == 0) {
        serlogF(SER_TCMENU_INFO, "Nav pop root");
        currentSub = nullptr;
        currentRoot = root;
        return root;
    } else {
        navIdx--;
        currentRoot = navItems[navIdx];
        currentSub = getSubMenuFor(currentRoot);
        serlogF3(SER_TCMENU_INFO, "Nav pop ", navIdx, currentRoot->getId());
        return activeItems[navIdx];
    }
}

bool tcnav::MenuNavigationStore::isShowingRoot() {
    return currentRoot == root;
}
