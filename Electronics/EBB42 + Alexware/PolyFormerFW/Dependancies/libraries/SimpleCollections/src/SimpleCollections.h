/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file SimpleCollections.h memory efficient collections for use on all embedded devices.
 */

#ifndef SIMPLE_COLLECTIONS_H
#define SIMPLE_COLLECTIONS_H

#include <string.h>
#include <inttypes.h>

namespace tccollection {

    enum GrowByMode : unsigned char {
        GROW_NEVER,
        GROW_BY_5,
        GROW_BY_DOUBLE
    };

}

#ifdef __AVR_ATmega2560__
# define DEFAULT_LIST_SIZE 10
# define DEFAULT_GROW_MODE GROW_BY_5
typedef uint8_t bsize_t;
#elif defined(__AVR__)
# define DEFAULT_LIST_SIZE 5
# define DEFAULT_GROW_MODE GROW_BY_5
typedef uint8_t bsize_t;
#else
# define DEFAULT_LIST_SIZE 10
# define DEFAULT_GROW_MODE GROW_BY_DOUBLE
typedef size_t bsize_t;
#endif


namespace ioaTreeInternal {

    typedef uint32_t (*KeyAccessor)(const void *item);

    typedef void (*CopyOperator)(void *dest, const void *src);

    /**
     * This is an internal btree storage engine that used by all the templates to actually store the data (and
     * hopefully save a lot of FLASH). It implements the costly features of the btree in a one off way. However,
     * as a result the keys must now be unsigned integer, and cannot exceed uint32_t. If you need more than this
     * use a std container.
     */
    class BtreeStorage {
    private:
        void *binTree;
        void *swapperSpace;
        KeyAccessor keyAccessor;
        CopyOperator copier;
        bsize_t currentCapacity;
        bsize_t currentSize;
        bsize_t sizeOfAnItem;
        tccollection::GrowByMode growByMode;
    public:
        BtreeStorage(bsize_t size, tccollection::GrowByMode howToGrow, bsize_t eachItemSize, KeyAccessor compareOperator,
                     CopyOperator copyOperator);

        ~BtreeStorage();

        bool add(const void *newItem);

        bool removeByKey(uint32_t key);

        void removeIndex(bsize_t index);

        bsize_t nearestLocation(uint32_t key) const;

        void *getByKey(uint32_t key) const;

        void clear() { currentSize = 0; }

        void *underlyingData() const { return binTree; }

        bsize_t getCapacity() const { return currentCapacity; }

        bsize_t getCurrentSize() const { return currentSize; }

    private:
        bool checkCapacity();
        void *memoryOf(void *baseMem, bsize_t item) const { return ((uint8_t *) baseMem) + (item * sizeOfAnItem); }
    };
}

namespace tccollection {

/**
 * An iterator that fulfills the range interface so that it can be used with range statements in c++ 17.
 * @tparam T
 */
    template<class T>
    class BtreeIterator {
    private:
        bsize_t currentPosition;
        const ioaTreeInternal::BtreeStorage &storage;
    public:
        BtreeIterator(bsize_t position, const ioaTreeInternal::BtreeStorage &storage) : currentPosition(position),
                                                                                  storage(storage) {}

        void operator++() { ++currentPosition; }

        bool operator!=(const BtreeIterator &other) const { return currentPosition != other.currentPosition; }

        T &operator*() { return ((T *) storage.underlyingData())[currentPosition]; }
    };

/**
 * A very simple binary search based list. It is useful for storage of items by key or just regular storage,
 * it is very efficient for reading, but relatively inefficient for insertions, but only when compared to
 * a linked list. It is however very memory efficient. There are some restrictions on it's use:
 * * Class V must have a method to get the key: K getKey()
 * * Class K must be directly comparable using less than, greater than and equals (any primitive would generally work).
 *
 * On inserts, the list is re-sorted to get it back into order by key, and will grow automatically if needed using the
 * howToGrow parameter passed to the constructor. The list is therefore always sorted by key, and this will generally
 * always involve a memory copy to move around the items.
 *
 * All keys must be of an unsigned integer type not exceeding uint32_t in length, you look up an item by it's key value,
 * the list will either return NULL or a valid item. You can also obtain all values too, which is always sorted by key
 * and is marked const, do not alter this array.
 */
    template<class K, class V>
    class BtreeList {
    private:
        ioaTreeInternal::BtreeStorage treeStorage;
    public:
        /**
         * Create a btree list that can be used to store simple objects that are not polymorphic. If no parameters are
         * provided it will construct with the defaults for your board.
         * @param size the initial size of the list, optional
         * @param howToGrow the way in which it should grow if space runs out, optional
         */
        explicit BtreeList(bsize_t size = DEFAULT_LIST_SIZE, GrowByMode howToGrow = DEFAULT_GROW_MODE)
                : treeStorage(size, howToGrow, sizeof(V), keyAccessor, copyInternal) {}
        /**
         * Advanced usage constructor, prefer using the other constructor whenever possible.
         *
         * This constructor allows you to provide your own key accessor, some other libraries require slightly different
         * ways of accessing the object. For the key accessor you'll be provided with a pointer to a memory area, this
         * area contains the item for which we need the key, you return the key value from the accessor as a uint32_t.
         *
         * The default key accessor is as follows:
         *
         * ```
         * static uint32_t keyAccessor(const void *itm) {
         *    return reinterpret_cast<const V *>(itm)->getKey();
         * }
         * ```
         *
         * @param customAccessor the custom accessor that is called to return the key
         * @param size the size of the list, optional parameter.
         * @param howToGrow the method for growing when space runs out, optional parameter
         */
        explicit BtreeList(ioaTreeInternal::KeyAccessor customAccessor, bsize_t size = DEFAULT_LIST_SIZE, GrowByMode howToGrow = DEFAULT_GROW_MODE)
                : treeStorage(size, howToGrow, sizeof(V), customAccessor, copyInternal) {}

        static uint32_t keyAccessor(const void *itm) {
            return reinterpret_cast<const V *>(itm)->getKey();
        }

        static void copyInternal(void *dest, const void *src) {
            auto *itemDest = reinterpret_cast<V *>(dest);
            auto *itemSrc = reinterpret_cast<const V *>(src);
            *itemDest = *itemSrc;
        }

        /**
         * Adds an item into the current list of items, the list will be sorted by using getKey()
         * on the object passed in. If the list cannot fit the item and cannot resize to do so, then
         * false is returned.
         * @param item the item to add
         * @return true if added, otherwise false
         */
        bool add(const V &item) { return treeStorage.add(&item); }

        /**
         * Get a value by it's key using a binary search algorithm.
         * @param key the key to be looked up
         * @return the value at that key position or null.
         */
        V *getByKey(K key) const { return reinterpret_cast<V *>(treeStorage.getByKey(key)); };

        /**
         * Remove an item using the key it was added with
         * @param key the key to remove
         * @return true if successful otherwise false.
         */
        bool removeByKey(K key) { return treeStorage.removeByKey(key); }

        /**
         * Remove an item by index in the underlying array
         * @param index the index in the array
         */
        void removeIndex(bsize_t index) { treeStorage.removeIndex(index); }

        /**
         * gets the nearest location to the key, this is an in-exact method in that it gives the exact match if available
         * or otherwise the nearest one that is lower.
         * @param key the key to lookup
         * @return the position in the list
         */
        bsize_t nearestLocation(const K &key) const { return treeStorage.nearestLocation(key); }

        /**
         * @return a list of all items
         */
        const V *items() const { return reinterpret_cast<V *>(treeStorage.underlyingData()); };

        /**
         * gets an item by it's index
         * @param idx the index to find
         * @return the item at the index or null.
         */
        V *itemAtIndex(bsize_t idx) const {
            auto *binTree = reinterpret_cast<V *>(treeStorage.underlyingData());
            return (idx < treeStorage.getCurrentSize()) ? &binTree[idx] : NULL;
        }

        /**
         * @return number of items in the list
         */
        bsize_t count() const {
            return treeStorage.getCurrentSize();
        }

        /**
         * @return current capacity of the list
         */
        bsize_t capacity() const { return treeStorage.getCapacity(); }

        /**
         * Completely clear down the list such that calls to count return 0.
         */
        void clear() { treeStorage.clear(); }

        /**
         * Implements the range begin function for use with C++ ranges, represents the beginning of the list, supports
         * using ++ to get the next item, but if using manually check against end() before accessing the value.
         *
         * For example to iterate you can
         *
         * ```for(auto& item : myBtreeList) { item.doSomething(); } ```
         *
         * @return an iterator for the beginning of the list.
         */
        BtreeIterator<V> begin() const { return BtreeIterator<V>(0, treeStorage); }

        /**
         * An iterator that represnts the value beyond the end of the list, for use with C++ ranges.
         * @return an iterator that represents the end of the list. Normally used with begin().
         */
        BtreeIterator<V> end() const { return BtreeIterator<V>(treeStorage.getCurrentSize(), treeStorage); }

        /**
         * Implements a simple foreach item loop, where your callback function is called once for each item in the list.
         * This may be helpful for those that think the introduction of an iterator is too heavy for their board.
         *
         * For example:
         *
         * ```myBtreeList.forEachItem( [] (BtreeType* item) { item.doSomething(); } );
         *
         * @param paramFn
         */
        void forEachItem(void(*paramFn)(V *)) {
            for (bsize_t i = 0; i < count(); ++i) {
                paramFn(itemAtIndex(i));
            }
        }
    };
}

#ifndef TC_COLLECTION_MANUAL_NAMESPACE
using namespace tccollection;
#endif

#endif // SIMPLE_COLLECTIONS_H
