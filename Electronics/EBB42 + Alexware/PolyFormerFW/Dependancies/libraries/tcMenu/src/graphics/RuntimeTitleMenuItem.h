/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file RuntimeTitleMenuItem.h the menu item that is presented for the title
 */

#ifndef TCMENU_RUNTIMETITLEMENUITEM_H
#define TCMENU_RUNTIMETITLEMENUITEM_H

#include <BaseRenderers.h>

namespace tcgfx {

    /**
     * forward reference of the title rendering function
     * @param item menu item for title
     * @param mode rendering mode
     * @param buffer the buffer
     * @param bufferSize the buffers size
     * @return status code
     */
    int appTitleRenderingFn(RuntimeMenuItem *item, uint8_t, RenderFnMode mode, char *buffer, int bufferSize);

    /**
     * This menu item extension class handles the title row, for the root menu. It stores a header in program memory
     * and a possible callback function for when it is actioned. This menu item also allows the header text to be
     * overridden too.
     */
    class RuntimeTitleMenuItem : public RuntimeMenuItem {
    private:
        const char *titleHeaderPgm;
        const char *titleOverridePgm;
        MenuCallbackFn callback;
    public:
        /**
         * There should only be one instance of this class, it is constructed globally by the library, you can access
         * this instance as
         * @param id
         * @param next
         */
        RuntimeTitleMenuItem(uint16_t id, MenuItem *next) : RuntimeMenuItem(MENUTYPE_TITLE_ITEM, id,
                                                                            appTitleRenderingFn, 0, 1, next) {
            titleHeaderPgm = nullptr;
            titleOverridePgm = nullptr;
            callback = nullptr;
        }

        /**
         * This should only be called by the menu library, to override what's displayed on the title you use
         * the `setTitleOverridePgm` instead.
         * @param header the new header text
         */
        void setTitleHeaderPgm(const char *header) {
            titleHeaderPgm = header;
            setChanged(true);
        }

        /**
         * In order to override the title with user specific text that overrides the regular title set use this method.
         * @param overrideTitle the text to override
         */
        void setTitleOverridePgm(const char* overrideTitle) {
            titleOverridePgm = overrideTitle;
            setChanged(true);
        }

        /**
         * Call this to clear an override that was previously set, so that the regular header text is displayed instead.
         */
        void clearTitleOverride() {
            titleOverridePgm = nullptr;
            setChanged(true);
        }

        const char *getTitleHeaderPgm() {
            return (titleOverridePgm != nullptr) ? titleOverridePgm : titleHeaderPgm;
        }

        void setCallback(MenuCallbackFn titleCb) {
            callback = titleCb;
        }

        MenuCallbackFn getCallback() {
            return callback;
        }
    };

    /**
     * the global instance of the title menu item
     */
    extern RuntimeTitleMenuItem appTitleMenuItem;

    /**
     * Sets the callback that will be triggered when the title is clicked on.
     * @param cb the title click callback.
     */
    inline void setTitlePressedCallback(MenuCallbackFn cb) {
        appTitleMenuItem.setCallback(cb);
    }
}

#endif //TCMENU_RUNTIMETITLEMENUITEM_H
