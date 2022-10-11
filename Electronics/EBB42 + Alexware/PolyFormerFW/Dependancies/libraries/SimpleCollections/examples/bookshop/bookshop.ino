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
 *
 * Note that this example is more complete than others and integrates with TaskManagerIO. You'll need this library too
 */

#include <TaskManagerIO.h>
#include <SimpleCollections.h>

//
// This is the type that we will store in our btree list, it will be stored indexed by it's key, and must implement
// the getKey() method to return the key. In this type the key is an int.
//
class Book {
private:
    int key;
    char name[20];
public:
    // You must include a default constructor that initialises the fields to empty, we store the actual
    // items in the internal array in an initially empty state.
    Book() = default;

    // You must include a copy constructor that copies from an existing item. This is needed by the
    // collection to copy items into the internal array.
    Book(const Book& otherBook) = default;

    // we must have a copy operator
    Book& operator=(const Book& other) = default;

    // And you'll probably need another constructor to initialise the fields for your own use.
    Book(int bookKey, const char* bookName) {
        strncpy(name, bookName, sizeof(name));
        key = bookKey;
    }

    // you must implement a method that returns the key field with this name, it must return the same type as the
    // key you specify for the list, (IE template parameter 1).
    int getKey() const {
        return key;
    }

    // Just return the name we were given earlier
    const char* getName() {
        return name;
    }
};

//
// This is the declaration of the list, we declare it as a list of Book objects keyed using an int.
//
BtreeList<int, Book> myList;

void setup() {
    Serial.begin(115200);
    while(!Serial);

    myList.add(Book(3, "All about Arduino"));
    myList.add(Book(2, "All about Xamarin"));
    myList.add(Book(1, "Record players"));

    // This shows us the current list capacity, do not worry if it drops to a very low value as it
    // will reallocate automatically.
    Serial.print("Capacity is now ");
    Serial.println(myList.capacity());

    // Get the index that is CLOSEST to a provided key, it is important to note that it's not an exact match
    Serial.print("The nearest index to key 100 is ");
    Serial.println(myList.nearestLocation(100));

    // now every 10 seconds lets look for a book by it's ID
    taskManager.scheduleFixedRate(10, [] {
        auto book2 = myList.getByKey(2);
        if(book2 != nullptr) {
            Serial.print("Book 2 found: ");
            Serial.println(book2->getName());
        }
        else {
            Serial.println("Book 2 not found");
        }
    }, TIME_SECONDS);

    // now every 15 seconds lets iterate all books in the collection
    taskManager.scheduleFixedRate(15, [] {
        myList.forEachItem([] (Book* book) {
            Serial.print(book->getName());
            Serial.print(", ");
            Serial.println(book->getKey());
        });
    }, TIME_SECONDS);

    // after a minute lets clean down the collection
    taskManager.scheduleOnce(60, [] {
        Serial.println("Now we will tear down the list");
        myList.clear();
    }, TIME_SECONDS);
}

void loop() {
    taskManager.runLoop();
}