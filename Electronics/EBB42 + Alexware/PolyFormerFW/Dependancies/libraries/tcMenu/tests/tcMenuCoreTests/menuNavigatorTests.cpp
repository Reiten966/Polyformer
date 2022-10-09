
#include <AUnit.h>
#include <MenuHistoryNavigator.h>
#include "fixtures_extern.h"

using namespace tcnav;

test(creatingAndInitialisation) {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);

    assertEqual(&menuVolume, nav.getRoot());
    assertEqual(&menuVolume, nav.getCurrentRoot());
    assertEqual(nullptr, nav.getCurrentSubMenu());
}

test(navigationPushAndPop) {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    nav.navigateTo(&menuChannel, menuStatus.getChild(), false);
    nav.navigateTo(&menuLHSTemp, menuSecondLevel.getChild(), false);
    nav.navigateTo(&menu12VStandby, menuSettings.getChild(), false);
    nav.navigateTo(&menuRHSTemp, &menuSub, false); // should not be stored
    nav.navigateTo(&menuRHSTemp, &menuSub, false);

    auto* act = nav.popNavigationGetActive();
    assertEqual(act, &menuRHSTemp);
    assertEqual(menuSettings.getChild(), nav.getCurrentRoot());
    assertEqual(&menuSettings, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEqual(act, &menu12VStandby);
    assertEqual(menuSecondLevel.getChild(), nav.getCurrentRoot());
    assertEqual(&menuSecondLevel, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEqual(act, &menuLHSTemp);
    assertEqual(menuStatus.getChild(), nav.getCurrentRoot());
    assertEqual(&menuStatus, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEqual(act, &menuChannel);
    assertEqual(&menuVolume, nav.getCurrentRoot());
    assertEqual(nullptr, nav.getCurrentSubMenu());

    // try and over pop from array.
    act = nav.popNavigationGetActive();
    assertEqual(act, &menuVolume);
    assertEqual(&menuVolume, nav.getCurrentRoot());
    assertEqual(nullptr, nav.getCurrentSubMenu());
}
