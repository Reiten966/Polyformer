#include <SimpleCollections.h>
#include <AUnit.h>
#include <IoLogging.h>

class TestStorage {
private:
    int testItem;
    int testKey;
public:
    TestStorage() {
        testItem = testKey = 0;
    }

    TestStorage(int key, int item) {
        this->testKey = key;
        this->testItem = item;
    }

    TestStorage(const TestStorage& other) {
        this->testItem = other.testItem;
        this->testKey = other.testKey;
    }

    TestStorage& operator=(const TestStorage& other) {
        if(this == &other) return *this;
        this->testItem = other.testItem;
        this->testKey = other.testKey;
        return *this;
    }

    int getKey() const {
        return testKey;
    }

    int getItem() const {
        return testItem;
    }
};

void printArray(const TestStorage* list, int size) {
    char sz[120];
    strcpy(sz, "array: ");
    for(int i=0;i<size;i++) {
        char intBuf[10];
        itoa(list[i].getKey(), intBuf, 10);
        strcat(sz, intBuf);
    }
    serdebug(sz);
}

TestStorage storage1(1, 100);
TestStorage storage2(2, 101);
TestStorage storage3(3, 102);
TestStorage storage4(4, 103);
TestStorage storage5(5, 104);
TestStorage storage6(6, 105);
TestStorage storage7(7, 106);
TestStorage storage8(8, 107);
TestStorage storage9(9, 108);
TestStorage storage10(10, 109);

test(testNearestLocationEdgeCases) {
    BtreeList<int, TestStorage> btreeList(5, GROW_NEVER);
    assertEqual((bsize_t)0, btreeList.nearestLocation(1));

    assertTrue(btreeList.add(storage4));
    printArray(btreeList.items(), btreeList.count());

    assertEqual((bsize_t)0, btreeList.nearestLocation(1));
    assertEqual((bsize_t)1, btreeList.nearestLocation(5));
}

test(testAddingWithoutSortOrResize) {
    BtreeList<int, TestStorage> btreeList(5, GROW_NEVER);

    assertTrue(btreeList.add(storage1));
    assertTrue(btreeList.add(storage2));
    assertTrue(btreeList.add(storage3));
    assertTrue(btreeList.add(storage4));
    assertTrue(btreeList.add(storage5));
    assertFalse(btreeList.add(storage6));
    printArray(btreeList.items(), btreeList.count());

    assertTrue(btreeList.getByKey(1) != nullptr);
    assertTrue(btreeList.getByKey(2) != nullptr);
    assertTrue(btreeList.getByKey(3) != nullptr);
    assertTrue(btreeList.getByKey(4) != nullptr);
    assertTrue(btreeList.getByKey(5) != nullptr);
    assertTrue(btreeList.getByKey(6) == nullptr);

    assertEqual(btreeList.getByKey(1)->getItem(), 100);
    assertEqual(btreeList.getByKey(2)->getItem(), 101);
    assertEqual(btreeList.getByKey(3)->getItem(), 102);
    assertEqual(btreeList.getByKey(4)->getItem(), 103);
    assertEqual(btreeList.getByKey(5)->getItem(), 104);
}

test(testAddingWithSortNoResize) {
    BtreeList<int, TestStorage> btreeList(5, GROW_NEVER);

    assertTrue(btreeList.add(storage2));
    assertTrue(btreeList.add(storage1));
    assertTrue(btreeList.add(storage5));
    assertTrue(btreeList.add(storage4));
    assertTrue(btreeList.add(storage3));
    assertFalse(btreeList.add(storage6));
    printArray(btreeList.items(), btreeList.count());

    assertTrue(btreeList.getByKey(1) != nullptr);
    assertTrue(btreeList.getByKey(2) != nullptr);
    assertTrue(btreeList.getByKey(3) != nullptr);
    assertTrue(btreeList.getByKey(4) != nullptr);
    assertTrue(btreeList.getByKey(5) != nullptr);
    assertTrue(btreeList.getByKey(6) == nullptr);

    const TestStorage* allItems = btreeList.items();
    assertEqual(allItems[0].getKey(), 1);
    assertEqual(allItems[1].getKey(), 2);
    assertEqual(allItems[2].getKey(), 3);
    assertEqual(allItems[3].getKey(), 4);
    assertEqual(allItems[4].getKey(), 5);

    assertEqual(allItems[0].getKey(), btreeList.itemAtIndex(0)->getKey());
    assertEqual(allItems[1].getKey(), btreeList.itemAtIndex(1)->getKey());

    assertEqual(btreeList.getByKey(1)->getItem(), 100);
    assertEqual(btreeList.getByKey(2)->getItem(), 101);
    assertEqual(btreeList.getByKey(3)->getItem(), 102);
    assertEqual(btreeList.getByKey(4)->getItem(), 103);
    assertEqual(btreeList.getByKey(5)->getItem(), 104);
}

test(testAddingWithSortAndResizeBy5) {
    BtreeList<int, TestStorage> btreeList(5, GROW_BY_5);

    assertTrue(btreeList.add(storage9));
    assertTrue(btreeList.add(storage8));
    assertTrue(btreeList.add(storage5));
    assertTrue(btreeList.add(storage4));
    assertTrue(btreeList.add(storage3));

    assertEqual((bsize_t)5, btreeList.capacity());
    assertTrue(btreeList.add(storage6));
    assertTrue(btreeList.add(storage7));
    assertTrue(btreeList.add(storage2));
    assertTrue(btreeList.add(storage1));
    assertTrue(btreeList.add(storage10));
    assertEqual((bsize_t)10, btreeList.capacity());
    printArray(btreeList.items(), btreeList.count());

    assertTrue(btreeList.getByKey(1) != nullptr);
    assertTrue(btreeList.getByKey(2) != nullptr);
    assertTrue(btreeList.getByKey(3) != nullptr);
    assertTrue(btreeList.getByKey(4) != nullptr);
    assertTrue(btreeList.getByKey(5) != nullptr);
    assertTrue(btreeList.getByKey(6) != nullptr);
    assertTrue(btreeList.getByKey(7) != nullptr);
    assertTrue(btreeList.getByKey(8) != nullptr);
    assertTrue(btreeList.getByKey(9) != nullptr);
    assertTrue(btreeList.getByKey(10) != nullptr);

    assertEqual(btreeList.getByKey(1)->getItem(), 100);
    assertEqual(btreeList.getByKey(2)->getItem(), 101);
    assertEqual(btreeList.getByKey(3)->getItem(), 102);
    assertEqual(btreeList.getByKey(4)->getItem(), 103);
    assertEqual(btreeList.getByKey(5)->getItem(), 104);
    assertEqual(btreeList.getByKey(6)->getItem(), 105);
    assertEqual(btreeList.getByKey(7)->getItem(), 106);
    assertEqual(btreeList.getByKey(8)->getItem(), 107);
    assertEqual(btreeList.getByKey(9)->getItem(), 108);
    assertEqual(btreeList.getByKey(10)->getItem(), 109);

    // clear the tree and ensure it clears
    btreeList.clear();
    assertEqual((bsize_t)10, btreeList.capacity());
    assertEqual((bsize_t)0, btreeList.count());

    // now add an item back and make sure we find it.
    assertTrue(btreeList.add(storage9));
    assertTrue(btreeList.getByKey(9) != nullptr);
    assertEqual(btreeList.getByKey(9)->getItem(), 108);

}

class NumericStorageItem {
private:
    uint32_t item;

public:
    NumericStorageItem() : item(0xffffffff) {}
    explicit NumericStorageItem(uint32_t value) : item(value) {}
    NumericStorageItem(const NumericStorageItem& other) = default;
    NumericStorageItem& operator= (const NumericStorageItem& other) = default;
    uint32_t getKey() const { return item; }
};

void printArray(const NumericStorageItem* list, int size) {
    char sz[160];
    strcpy(sz, "array: ");
    for(int i=0;i<size;i++) {
        char intBuf[10];
        itoa((int)(list[i].getKey()), intBuf, 10);
        strcat(sz, intBuf);
        strcat(sz, " ");
    }
    serdebug(sz);
}


test(testAddingThenRemovingThenAddingItems) {
    BtreeList<uint32_t, NumericStorageItem> myList;

    for(int i=0;i<20;i++) {
        myList.add(NumericStorageItem(i + 100));
    }

    // should have 20 items now
    assertEqual(myList.count(), bsize_t(20));

    // remove key 102
    assertTrue(myList.removeByKey(102));
    assertFalse(myList.removeByKey(10002));

    printArray(myList.items(), myList.count());

    // should have 19 items
    assertEqual(myList.count(), bsize_t(19));

    // ensure that only 102 was removed
    assertTrue(myList.getByKey(102) == nullptr);
    assertTrue(myList.getByKey(103) != nullptr);
    assertTrue(myList.getByKey(101) != nullptr);

    // put 102 back
    myList.add(NumericStorageItem(102));

    printArray(myList.items(), myList.count());

    for(int i=0;i<20;i++) {
        auto entry = myList.itemAtIndex(i);
        assertEqual(entry->getKey(), uint32_t(i + 100));
    }

    // now test removing the edge cases, IE head and tail of list
    myList.removeIndex(0);
    myList.removeIndex(19);
    printArray(myList.items(), myList.count());

    assertEqual(myList.count(), bsize_t(18));
    assertTrue(myList.getByKey(100) == nullptr);
    assertTrue(myList.getByKey(119) == nullptr);
}