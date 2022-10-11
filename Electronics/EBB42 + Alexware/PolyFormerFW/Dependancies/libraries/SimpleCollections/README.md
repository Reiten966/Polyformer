# SimpleCollections for Arduino and mbed summary

Dave Cherry / TheCodersCorner.com make this library available for you to use. It takes me significant effort to keep all my libraries current and working on a wide range of boards. Please consider making at least a one off donation via the sponsor button if you find it useful. In forks, please keep text to here intact.

This library provides two collections. Firstly, a btree-list implementation that can be used as a straight list, but it is always associative. BtreeList works on a very wide range of boards from Uno right up to most mbed devices. It's benefit for library writers especially is the very wide range of devices it can target with low memory requirements on the smallest of boards. Secondly, a circular buffer that is safe for concurrent use on many Arduino and mbed boards. 

Why? Because on many embedded boards std lib is simply not available, and on others it is potentially a bit too heavy at runtime. This collection is designed to run on anything from Uno upwards with reasonable performance. 

Btree list has been expanded upon and broken out from IoAbstraction, as such it has been battle tested in IoAbstraction and TcMenu. Btree list unlike CircularBuffer should not be used across threads. It is safe within tasks on TaskManager.

There is a forum where questions can be asked, but the rules of engagement are: **this is my hobby, I make it available because it helps others**. Don't expect immediate answers, make sure you've recreated the problem in a simple sketch that you can send to me. Please consider making at least a one time donation using the sponsor link above before using the forum. 

* [TCC Libraries community discussion forum](https://www.thecoderscorner.com/jforum/)
* I also monitor the Arduino forum [https://forum.arduino.cc/], Arduino related questions can be asked there too.

## Installation for Arduino IDE

This library is available in library manager on both Arduino and PlatformIO, this is the best choice for most people. It is highly recommended that you install the libraries using your library manager.

## Installation for PlatformIO (Arduino or mbed)

Use the platformIO library manager to get the library. It's called 'SimpleCollections'.

## Creating and using a list

We must understand that this list is associative and sorted by a key, it is based on a binary search algorithm, so it is relatively slow to insert into the underlying array as it will need to be inserted into the array at the right point. However, in return for this, lookup by key is very fast - in big-O notation it is approximately Log(N) or in simple terms to look up in a 256 item list by key would take maximum 8 iterations. However, insertion carries a possible copy penalty if the items need reordering.

All collections in this library are in the namespace tccollection, by default SimpleCollection.h adds a statement to use this namespace automatically.

### Restrictions on what you put in the list

This list works by copying items into the list, so the things you store must follow a couple of simple rules.

The key type can be any type that is 4 bytes or fewer. This is a limitation of the underlying way we implement the storage, to significantly reduce compiled sizes on smaller boards. For example, it could be `int`, `uint32_t`, `unit8_t` etc.

* The type must have a default constructor and a copy constructor.
* The type must have an assignment operator
* It must expose a `getKey` method that returns the key type and marked as const.
* It is best to stick to quite simple classes, as during insert operations they will be copied.
* If you want to use this as a general purpose list and are not interested in ordering, just make getKey return a larger number for each item you add.

## Quick start - create a list, iterate, get by key

Contents of the iteration example to get you started, you can either copy into your ide or open the iteration example. In short, first we create the MyStorage type that will be stored in the list, it has a key of type uint8_t. We then create the list object, populating it in the `setup()` method. In the loop we then read back the values using various techniques.

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

## List sizing and defaults

On Uno, the initial number of items is lowered to 5 by default, with grow mode set to grow by 5 each time, you can lower this in the constructor if needed. On MEGA2560 it will start with 10 items, and grow by 5 each time. On all 32 bit boards it will start at 10 and double each time. To change the default use the following method

    BtreeList<KeyType, Value> myList(size, howToGrow)

Where the size is the initial capacity of the list, and the grow by mode is one of: `GROW_NEVER, GROW_BY_5, GROW_BY_DOUBLE`

## Other helpful methods

    bsize_t nearestLocation(const K& key) // get the location nearet to key

    const V* items() // get the underlying item array.

    V* itemAtIndex(bsize_t idx) // get the item at a particular index

    bsize_t count() // the number of items in the list

    bsize_t capacity() // the current allocated size of the array

## Concurrent Circular Buffers

This library also supports concurrent circular buffers that work on most boards listed below. These buffers have independent writer and reader positions. This means that one thread can offer data, and another thread can read that data. It is entirely non-blocking and therefore safe across threads or even in interrupts. Be aware that if used in interrupts, the writer position is managed using CAS instructions (or emulation thereof) and will be slow if more than one thread does the writing (because of busy spin waiting).

There is an example that shows the usage of the circular buffer, but the API is really simple.

### Creating a circular buffer for storage of bytes (uint8_t)

We first create an instance and indicate the size needed, the size is fixed and if the writer exceeds the reader, it will wrap and data is lost. See further down for circular buffers of more complex types.

    #include <SimpleCollections.h>
    #include <SCCircularBuffer.h>

    SCCircularBuffer buffer(20);

### Writing to the buffer

Write a byte to the buffer by calling `put`, it will **wrap** if the reader gets behind.

    buffer.put(dataByte);

### Reading and checking the buffer

Only ever call `get` after checking that data is `available`, only one thread should ever be reading at once.

    if(buffer.available()) {
        uint8_t data = buffer.get();
        // do something with "data"
    }

## Creating a GenericCircularBuffer for a type other than uint8_t

You can create a circular buffer for type other than byte, to do so, you use the `GenericCircularBuffer` instead. It takes a type parameter and is a template, so only use when you need to store other than byte in it.

Bear in mind, that if the item you are storing in the circular buffer is not atomic, such as a pointer, or a machine length word, you risk it being corrupt when you see it on the other thread. To get around this we recommend that you have two circular buffers, one acting as a memory pool, and the other as the actual buffer. They should be the same type:

    // let's say we want to store this structure in the buffer 
    struct WriterStruct {
        volatile uint32_t sequence;
        volatile uint32_t data1;
        volatile uint32_t data2;

        void setData(uint32_t s, uint32_t d1, uint32_t d2) {
            sequence = s;
            data1 = d1;
            data2 = d2;
        }
    };

    // we first create a buffer that acts as a pool, notice the 2nd parameter. It has the same number of above structures as the actual queue
    GenericCircularBuffer<WriterStruct> writerMemoryAlloc(10,GenericCircularBuffer::MEMORY_POOL);
    // We then create the actual buffer, it takes pointers to the structure
    GenericCircularBuffer<WriterStruct*> actualBuffer(10);

    void putSomethingIntoQueue() {
        // first we get the next available structure from the pool
        auto &alloc = writerMemoryAlloc.get();
        // now we prepare it to be sent, it must be entirely ready!
        alloc.setData(nextSequence, nextSequence * 1000, nextSequence * 2000);
        // now we send it.
        actualBuffer.put(&alloc);
    }

The queue is read back as normal, but we get back a pointer.

    if(actualBuffer.available()) {
        auto myData = actualBuffer.get();
        auto localData1 = myData->data1;
    }

In short, you should never queue an object until it is fully and atomically ready. Again, just like with circular buffers themselves, the memory pool will wrap if the writer gets too far ahead of the reader.

## Platforms known to work

The following platforms are ones that we test with, they are generally the best choices to use with this library.

| Platform | Board / Arch   | State            | Thread safety   |
|----------|----------------|------------------|-----------------|
| Arduino  | Nano 33 BLE    | Examples tested  | CAS             |         
| Arduino  | Uno, MEGA, AVR | Examples tested  | Atomic          |
| Arduino  | SAMD MKR1300   | Examples tested  | Atomic          |
| Arduino  | SAMD Seeed     | Examples tested  | Atomic          |        
| Arduino  | STM32Duino     | Examples tested  | CAS if possible |  
| Arduino  | ESP8266        | Examples tested  | Atomic          |
| Arduino  | ESP32          | Examples tested  | CAS             |
| mbed     | STM32F4        | mbed example run | CAS             |

Thread safety key:

* Atomic - the compare and set is implemented in software and wrapped with noInterrupts / interrupts
* CAS - the compare and set is implemented using processor level instructions (STM32Duino) or in the case of mbed and ESP32, by their utility function.

The circular buffer is thread safe on nearly all boards, for larger ARM processors that are at least CortexM4 level you can enable CAS locking, we do it automatically for STM32Duino boards that meet the required cortex level. You can do this yourself by defining `SC_USE_ARM_ASM_CAS` for other ARM boards that are on at least CORTEX M4.

In the event you see issues indicating LDREX or STREX are not supported, please raise an issue here with the exact board and define `SC_NO_ARM_ASM_CAS`; which will then turn off the support while we can fix it.

## Making changes to SimpleCollections

We welcome people rolling up their sleeves and helping out, but please do reach out to us before starting any work, so we can ensure its in sync with our development. We use platformIO for development and have a specific project available to help you get started, along with tests that check many elements still work as expected. See [https://github.com/davetcc/tcLibraryDev]
