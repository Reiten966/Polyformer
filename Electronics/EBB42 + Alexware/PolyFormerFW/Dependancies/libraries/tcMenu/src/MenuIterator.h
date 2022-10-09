/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>

/**
 * @file MenuIterator.h
 * 
 * Provides a number of utility functions for the processing of menu item structures.
 */

#ifndef _MENUITERATOR_H
#define _MENUITERATOR_H

#ifndef MAX_MENU_DEPTH
# define MAX_MENU_DEPTH 4
#endif // MAX_MENU_DEPTH

// forward reference of menu item
class MenuItem;

/**
 * defines a function used when needing to visit all menu items 
 */
typedef void (*MenuVisitorFn)(MenuItem* item);

/**
 * Finds the parent root menu item to the item that's passed in, that is the root item that contains
 * this menu item. Normally used by protocol and display layers when there's a need to traverse the
 * menu structure. This version is also able to provide a function that will visit each element in
 * the tree. It will always visit every item.
 * 
 * @param current the menu item that is currently menu root
 * @return the parent menu item to the present menu item
 */
MenuItem* getParentRootAndVisit(MenuItem* current, MenuVisitorFn visitor);

/**
 * Finds the parent root menu item to the item that's passed in, that is the root item that contains
 * this menu item. This version will short circuit out of the traversal as soon as the item is found.
 * Never returns NULL.
 * 
 * @param current the menu item that is currently menu root
 * @return the parent menu item to the present menu item, returns root instead of NULL.
 */
inline MenuItem* getParentRoot(MenuItem* current) { return getParentRootAndVisit(current, nullptr); }

/**
 * Finds the submenu that a particular menu item belongs to, or nullptr
 * @param current the menuitem that we are searching for
 * @return the submenu or nullptr if it was in the root.
 */
MenuItem* getSubMenuFor(MenuItem* current);

/**
 * Gets the first match by ID of a menu item in the menu structure.
 * @param the ID to locate the menu item  for
 * @return the menu item associated or NULL.
 */
MenuItem* getMenuItemById(menuid_t id);

/**
 * Gets the item at the position requested within menu root.
 * @param root the root of the menu
 * @param pos the index of the item
 * @return either root or the item at the index.
 */
MenuItem* getItemAtPosition(MenuItem* root, uint8_t pos);

/**
 * Gets the zero based offset of the active item in the menu provided 
 * @param root the root of the current menu
 * @return the offset from zero of the item 
 */
int offsetOfCurrentActive(MenuItem* root);

/**
 * returns the number of items in the current menu described by itemCount
 * @param item the root item of the menu to be counted.
 * @param includeNonVisble include menu items that are not marked visible
 * @return the number of items, may include only visible ones depending on flag
 */
uint8_t itemCount(MenuItem* item,  bool includeNonVisble = false);


/**
 * A predicate that can match upon a menu item, the match is generally performed by calling the
 * match method, which returns true for a match.
 */
class MenuItemPredicate {
public:
    /**
     * This method is used to determine if a given menuitem given by item matches the predicate
     * @param item the item to be checked for a match.
     * @return true if there is a match, false otherwise.
     */
    virtual bool matches(MenuItem* item)=0;
};

/**
 * A specialisation of the MenuItemPredicate that matches on a given remote number.
 */
class RemoteNoMenuItemPredicate : public MenuItemPredicate {
private:
    uint8_t remoteNo;
public:
    /**
     * Constructs the predicate with the remote number we are interested in.
     * @param the number of the remote we want to check for.
     */
    explicit RemoteNoMenuItemPredicate(int remoteNo) { this->remoteNo = remoteNo; }

    /**
     * Matches if the remote number is marked as changed on the item. Remote number from the constructor.
     * @param item the menu item to be checked
     * @return true if the remote has changed, false otherwise
     */
    bool matches(MenuItem* item) override;

    /**
     * Sets the remote number that we should filter on
     * @param newRemoteNum the remote to filter on
     */
    void setRemoteNo(uint8_t newRemoteNum) { remoteNo = newRemoteNum; }
};

// the modes that can be passed to the type predicate
#define TM_REGULAR  0
#define TM_INVERTED  1
#define TM_REGULAR_LOCAL_ONLY  8
#define TM_INVERTED_LOCAL_ONLY  9
#define TM_EXTRA_INCLUDE_SUBMENUS 16
#define TM_BIT_INVERT 0U
#define TM_BIT_LOCAL_ONLY 3U
#define TM_BIT_INCLUDE_SUBMENU 4U

/**
 * A specialisation of the MenuItemPredicate that matches on a given MenuType. For example sub menus or 
 * boolean menu items. The following modes can be used
 *  * TM_REGULAR anything matching the filter type
 *  * TM_INVERTED anything not matching the filter type
 *  * TM_REGULAR_NOLOCAL regular + local only not set
 *  * TM_INVERTED_NOLOCAL inverted + local only not set
 */
class MenuItemTypePredicate : public MenuItemPredicate {
private:
    MenuType filterType;
    uint8_t mode;
public:
    /**
     * Construct the predicate indicating the type of item to filter on
     * @param filterType the type to filter for
     */
    MenuItemTypePredicate(MenuType filterType, uint8_t mode = 0) { 
        this->filterType = filterType; 
        this->mode = mode;
    }

    /**
     * This predicate checks if the item matches the type in the constructor.
     * @param item the item to be checked
     * @return true if menu type is of filterType provided in the constructor
     */
    bool matches(MenuItem* item) override;
};

/**
 * This provides a way to non recursively iterate through the entire menu structure. Each call to
 * nextItem() finds the next item that matches the predicate, traversing through submenus if needed.
 * This has a limitation of traversing only up to `MAX_MENU_DEPTH` levels of menus. Adjust this value
 * in MenuIterator.h to support more menu levels.
 */
class MenuItemIterator {
private:
    MenuItem* currentItem;
    MenuItemPredicate* predicate;
    MenuItem* parentItems[MAX_MENU_DEPTH];
    uint8_t level;
    bool processingSubMenu;
public:
    MenuItemIterator() { 
        reset(); 
        predicate = nullptr;
    }

    /**
     * Set the predicate that will be used on subsequent calls
     * @param predicate a predicate that will filter returned results.
     */
    void setPredicate(MenuItemPredicate* predicate) { this->predicate = predicate; }

    /**
     * Reset the state so that the next call to nextItem() returns the first 
     */
    void reset();

    /**
     * Gets the next menu item in this iterators order, filtered by the current predicate.
     * When the last item is passed this iterator returns NULL and then calls reset().
     * @return the next item or NULL when the end is reached.
     */
    MenuItem* nextItem();

    /**
     * Returns the parent of the item that will be returned by nextItem(), always call AFTER
     * calling nextItem() as next item will possibly alter this value. Result will be NULL when
     * there is no parent (root). The result when not null will be the nearest submenu.
     * 
     * @return the parent of the next item or NULL for root, must be called before nextItem().
     */
    MenuItem* currentParent();
};

#endif
