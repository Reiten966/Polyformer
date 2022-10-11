/**
 * This list is not designed to replace the collections in the standard library, it is a fairly light collection
 * that works across a very wide range of boards. It is the underpinnings of switches, and we use it the mbed
 * support too. So it is pretty much production ready.
 *
 * Here we create a class that we are going to store in the btree list, items in btree lists are ordered so
 * insertion is a little slow, but retrieval, even by key is very quick - Log(NumItems). It dynamically reallocates
 * if needed too, so there's no need in most cases to fiddle directly with the sizes.
 *
 * The only limitation of putting an item into the btree is that the method getKey() must be implemented by the type
 * that is stored. You can see the default sizes of the list and other parameters in file SimpleCollections.h for
 * advanced users.
 *
 * It is only designed to store small, simple class types that are not polymorphic, however, you can wrap a pointer
 * to a more dynamic class type if you needed too as a field of this class.
 *
 * Important note for advanced users: This list is not thread safe. If you use it within task manager only this is not
 * an issue, but if you use it elsewhere too, it must be protected by some kind of mutex or atomic block.
 */

#include <Arduino.h>
#include <SimpleCollections.h>

class MyStorage {
private:
    uint8_t key;
    uint32_t value;
public:
    // we must define
    MyStorage() = default;
    MyStorage(const MyStorage& other) = default;
    MyStorage& operator=(const MyStorage& other) = default;

    MyStorage(uint8_t key, uint32_t value) : key(key), value(value) {}

    uint8_t getKey() const { return key; }
    uint32_t getValue() const { return value; }
};

BtreeList<uint8_t, MyStorage> myList;

void setup() {
    myList.add(MyStorage(0, 2093));
    myList.add(MyStorage(1, 0xf00dface));
    myList.add(MyStorage(2, 0xdeadbeef));
}

void loop() {
    Serial.println("Range iteration");
    for(auto item : myList) {
        Serial.println(item.getValue());
    }

    Serial.println("ForEach iteration");
    myList.forEachItem([] (MyStorage* storage) {
        Serial.println(storage->getValue());
    });

    auto item = myList.getByKey(2);
    if(item) {
        Serial.println("Get By Key");
        Serial.println(item->getValue());
    }
    else {
        Serial.println("Get By Key Failed");
    }

    Serial.println("Count and capacity");
    Serial.println(myList.count());
    Serial.println(myList.capacity());
    delay(4000);
}