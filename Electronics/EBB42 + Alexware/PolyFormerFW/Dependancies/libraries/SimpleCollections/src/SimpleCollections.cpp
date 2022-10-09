/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifdef __AVR__
#include <Arduino.h>
#else
#include <malloc.h>
#endif
#include "SimpleCollections.h"

using namespace ioaTreeInternal;
using namespace tccollection;

BtreeStorage::BtreeStorage(bsize_t size, GrowByMode howToGrow, bsize_t itemSize, KeyAccessor keyAccess, CopyOperator copyOperator) {
    currentCapacity = size;
    growByMode = howToGrow;
    currentSize = 0;
    sizeOfAnItem = itemSize;
    keyAccessor = keyAccess;
    copier = copyOperator;
    binTree = malloc(sizeOfAnItem * size);
    swapperSpace = malloc(sizeOfAnItem);
}

BtreeStorage::~BtreeStorage() {
    free(binTree);
    free(swapperSpace);
}

bool BtreeStorage::add(const void *newItem) {
    // check capacity returns true, we always have space
    if(!checkCapacity()) return false;

    // find the insertion point.
    bsize_t insertionPoint = nearestLocation(keyAccessor(newItem));

    // given the insert position, work out the number of items to move
    int amtToMove = (currentSize - insertionPoint);

    // move the instances in reverse order using their assignment operator.
    if(amtToMove > 0) {
        for (bsize_t i = insertionPoint + amtToMove; i > insertionPoint; --i) {
            copier(memoryOf(binTree, i), memoryOf(binTree, i - 1));
        }
    }

    // and finally, insert the new item
    copier(memoryOf(binTree, insertionPoint), newItem);
    currentSize++;
    return true;
}

bool BtreeStorage::removeByKey(uint32_t key) {
    for(bsize_t i = 0; i<currentSize; ++i) {
        if(keyAccessor(memoryOf(binTree, i)) == key) {
            removeIndex(i);
            return true;
        }
    }
    return false;
}

void BtreeStorage::removeIndex(bsize_t index) {
    // given the insert position, work out the number of items to move
    int amtToMove = (currentSize - index) - 1;

    // move the instances in reverse order using their assignment operator.
    if(amtToMove > 0) {
        for (bsize_t i = index; i < index + amtToMove; ++i) {
            copier(memoryOf(binTree, i), memoryOf(binTree, i + 1));
        }
    }
    currentSize--;
}

/**
 * Check the capacity of the list, increasing the size if needed. This is normally called before an add operation
 * to ensure there will be space to add another item.
 * @return
 */
bool BtreeStorage::checkCapacity() {
    // a couple of short circuits first
    if(currentSize < currentCapacity) return true;
    if(growByMode == GROW_NEVER) return false;

    // now determine the new size and try and group the list.
    int newSize = currentSize + ((growByMode == GROW_BY_5) ? 5 : currentSize);
    auto replacement = malloc(sizeOfAnItem * newSize);
    if(replacement == nullptr) return false;

    // now copy over and replace the current tree.
    for(bsize_t i=0; i<currentSize; ++i) {
        copier(memoryOf(replacement, i), memoryOf(binTree, i));
    }
    free(binTree);
    binTree = replacement;
    currentCapacity = newSize;
    return true;
}

bsize_t BtreeStorage::nearestLocation(uint32_t key) const {
    // a few short circuits, basically handling quickly nothing in list,
    // one item in the list and an insertion at the end of the list.
    if(currentSize == 0) return 0; // always first item in this case
    else if(currentSize == 1) return (key <= keyAccessor(binTree)) ? 0 : 1;
    else if(key > keyAccessor(memoryOf(binTree, currentSize - 1))) return currentSize;

    // otherwise, we search with binary chop
    bsize_t start = 0;
    bsize_t end = currentSize - 1;
    bsize_t midPoint;
    while((end - start) > 1) {
        midPoint = start + ((end - start) / 2);
        auto midKey = keyAccessor(memoryOf(binTree, midPoint));
        if(midKey == key) return midPoint;
        else if(midKey > key) end = midPoint;
        else if(midKey < key) start = midPoint;
    }

    // when we get here we've got down to two entries and need to
    // either return the exact match, or locate the lower of the two
    // in the case there no exact match.

    // check if start or end contain the key
    if(keyAccessor(memoryOf(binTree, start)) == key) return start;
    if(keyAccessor(memoryOf(binTree, end)) == key) return end;

    // in this case we return the first item LOWER than the current one
    while(end > 0 && keyAccessor(memoryOf(binTree, end - 1)) > key) --end;
    return end;
}

void *BtreeStorage::getByKey(uint32_t key) const {
    if(currentSize == 0) return nullptr;
    bsize_t loc = nearestLocation(key);
    return (keyAccessor(memoryOf(binTree, loc)) == key && loc < currentSize) ? memoryOf(binTree, loc) : nullptr;
}
