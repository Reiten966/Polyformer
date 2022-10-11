/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "PlatformDetermination.h"
#include "RuntimeTitleMenuItem.h"

namespace tcgfx {

    RuntimeTitleMenuItem appTitleMenuItem(0, nullptr);

    int appTitleRenderingFn(RuntimeMenuItem *item, uint8_t, RenderFnMode mode, char *buffer, int bufferSize) {
        if (item->getMenuType() != MENUTYPE_TITLE_ITEM) return 0;
        auto *pTitleItem = reinterpret_cast<RuntimeTitleMenuItem *>(item);

        switch (mode) {
            case RENDERFN_INVOKE:
                if (pTitleItem->getCallback()) {
                    auto cb = pTitleItem->getCallback();
                    cb(item->getId());
                }
                return true;
            case RENDERFN_VALUE: {
                buffer[0] = '^';
                buffer[1] = 0;
                return true;
            }
            case RENDERFN_NAME: {
                safeProgCpy(buffer, pTitleItem->getTitleHeaderPgm(), bufferSize);
                return true;
            }
            default:
                return false;
        }
    }

};
