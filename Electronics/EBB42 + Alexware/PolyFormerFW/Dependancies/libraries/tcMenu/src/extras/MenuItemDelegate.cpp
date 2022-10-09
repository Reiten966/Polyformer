/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuItemDelegate.h"

using namespace tccore;

bool MenuItemDelegate::onEachItem(MenuItemDelegate::ItemDelegateFn itemDelegateFn, AnyOrAll modeAny) {
    bool combinedReturn = true;
    for(int i=0; i<numberOfItems; i++) {
        bool ret = itemDelegateFn(itemArray[i], internalFlag);
        if(ret && modeAny == ANY) return ret;
        combinedReturn = ret && combinedReturn;
    }
    return combinedReturn;
}

void MenuItemDelegate::setReadOnly(bool readOnly) {
    internalFlag = readOnly;
    onEachItem([](MenuItem* item, bool flg) {
        item->setReadOnly(flg);
        return false;
    }, ALL);
}

void MenuItemDelegate::setLocalOnly(bool localOnly) {
    internalFlag = localOnly;
    onEachItem([](MenuItem* item, bool flg) {
        item->setLocalOnly(flg);
        return true;
        }, ALL);
}

void MenuItemDelegate::setVisible(bool visible) {
    internalFlag = visible;
    onEachItem([](MenuItem* item, bool flg) {
        item->setVisible(flg);
        return true;
        }, ALL);
}

void MenuItemDelegate::setChangedAndRemoteSend() {
    onEachItem([](MenuItem* item, bool flg) {
        item->setChanged(true);
        item->setSendRemoteNeededAll();
        return true;
        }, ALL);
}

void MenuItemDelegate::setChangedOnly() {
    onEachItem([](MenuItem* item, bool flg) {
        item->setChanged(true);
        return true;
        }, ALL);
}

bool MenuItemDelegate::isReadOnly(AnyOrAll anyOrAll) {
    return onEachItem([](MenuItem* item, bool flg) { return item->isReadOnly(); }, anyOrAll);
}

bool MenuItemDelegate::isLocalOnly(AnyOrAll anyOrAll) {
    return onEachItem([](MenuItem* item, bool flg) { return item->isLocalOnly(); }, anyOrAll);
}

bool MenuItemDelegate::isVisible(AnyOrAll anyOrAll) {
    return onEachItem([](MenuItem* item, bool flg) { return item->isVisible(); }, anyOrAll);
}

bool MenuItemDelegate::isChanged(AnyOrAll anyOrAll) {
    return onEachItem([](MenuItem* item, bool flg) { return item->isVisible(); }, anyOrAll);
}

