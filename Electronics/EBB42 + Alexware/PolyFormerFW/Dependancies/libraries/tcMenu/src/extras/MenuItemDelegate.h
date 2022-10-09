/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_MENUITEMDELEGATE_H
#define TCMENU_MENUITEMDELEGATE_H

/**
 * @file MenuItemDelegate.h contains a delegate class that allows some menu item operations on more than one item at once
 */

#include <MenuItems.h>

namespace tccore {

    /**
     * This delegate allows menu item operations to take place on more than once item at once. You provide an array of menu
     * item references in the constructor, and then calling the delegated methods applies that action to all menu items.
     *
     * An example is:
     * ```
     * MenuItem*[] items = { &item1, &item2 };
     * MenuItemDelegate delegate(items, 2);
     * delegate.setReadOnly(true); // makes item1 and item2 read only.
     * ```
     */
    class MenuItemDelegate {
    private:
        MenuItem** itemArray;
        int numberOfItems;
        bool internalFlag;
    public:
        /**
         * For cases where a boolean value is returned, you can check if any item or all items match.
         * For example are any values read only, or are all values read only.
         */
        enum AnyOrAll { ANY, ALL };

        /**
         * If you use the onEachItem method, this is the signature of the function that is provided.
         */
        typedef bool (*ItemDelegateFn)(MenuItem* item, bool internalFlag);

        /**
         * Create a delegate providing the array of menu items to delegate to, and also the number of items
         * @param itemArray the array of menu item pointers
         * @param items the number of items in the array
         */
        MenuItemDelegate(MenuItem** itemArray, int items) : itemArray(itemArray), numberOfItems(items), internalFlag(false) {}

        /**
         * Set the read only flag for all items in the delegate
         * @param readOnly the new read only state.
         */
        void setReadOnly(bool readOnly);

        /**
         * Set the local only flag for all items in the delegate
         * @param localOnly if the value is local only.
         */
        void setLocalOnly(bool localOnly);

        /**
         * Set the visibility flag for all items in the delegate - dont forget you need to mark the menu structurally
         * changed after changing this.
         * @param visible the new visibility.
         */
        void setVisible(bool visible);

        /** Set all delegate items as changed and ready to send remotely */
        void setChangedAndRemoteSend();

        /** Set all delegate items as changed without marking for remote transmission */
        void setChangedOnly();

        /**
         * Check if either ANY or ALL of the values are read only depending on the mode chosen.
         * @param mode either ANY or ALL
         * @return true if matched otherwise false
         */
        bool isReadOnly(AnyOrAll mode);

        /**
         * Check if either ANY or ALL of the values are local only depending on the mode chosen.
         * @param mode either ANY or ALL
         * @return true if matched otherwise false
         */
        bool isLocalOnly(AnyOrAll mode);

        /**
         * Check if either ANY or ALL of the values are visible depending on the mode chosen.
         * @param mode either ANY or ALL
         * @return true if matched otherwise false
         */
        bool isVisible(AnyOrAll mode);

        /**
         * Check if either ANY or ALL of the values are changed depending on the mode chosen.
         * @param mode either ANY or ALL
         * @return true if matched otherwise false
         */
        bool isChanged(AnyOrAll mode);

        /**
         * Iterates over each of the items calling the itemDelegateFn which takes two parameters, the
         * current menu item as a pointer, and the mode. It will return the return value of the delegate
         * function based on the mode - ANY or ALL.
         * @param itemDelegateFn the function that is called for each menu item
         * @param modeAny ANY or ALL
         * @return the result based on either ANY or ALL flag
         */
        bool onEachItem(ItemDelegateFn itemDelegateFn, AnyOrAll modeAny);
    };
}

#endif //TCMENU_MENUITEMDELEGATE_H
