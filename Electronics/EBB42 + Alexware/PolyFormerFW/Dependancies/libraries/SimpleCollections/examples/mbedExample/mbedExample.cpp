/**
 * This list is not designed to replace the collections in the standard library, it is a fairly light collection
 * that works across a very wide range of boards. It is the underpinnings of IoAbstraction, and we use it the mbed
 * support too. So it is very much production ready.
 *
 * Here we create a class that we are going to store in the btree list, items in btree lists are ordered so
 * insertion is a little slow, but retrieval, even by key is very quick - Log(NumItems). It dynamically reallocates
 * if needed too, so there's no need in most cases to fiddle directly with the sizes.
 *
 * The only limitation of putting an item into the btree is that the method getKey() must be implemented by the type
 * that is stored. You can see the default sizes of the list and other parameters in file SimpleCollections.h for
 * advanced users.
 *
 * It is only designed to store small, simple class types that are not polymorphic, however, you can wrap a more dynamic
 * class type if you needed too as a field of this class.
 *
 * Important note for advanced users: This list is not thread safe. If you use it within task manager only this is not
 * an issue, but if you use it elsewhere too, it must be protected by some kind of mutex or atomic block.
 */

#include <mbed.h>
#include <SimpleCollections.h>
#include <SCCircularBuffer.h>

BufferedSerial console(USBTX, USBRX, 115200);

class MbedStorage {
private:
    uint32_t hash;
    char mbedData[20];
public:
    MbedStorage() = default;
    MbedStorage(const MbedStorage& other) = default;
    MbedStorage& operator=(const MbedStorage& other) = default;
    MbedStorage(const char* sz) {
        uint32_t h = 9348451;
        for(size_t i=0; i<strlen(sz); i++) {
            h *= sz[i];
        }
        hash = h;
        strncpy(mbedData, sz, sizeof(mbedData));
    }

    uint32_t getKey() const { return hash; }

    const char* getData() const { return mbedData; }
};

BtreeList<uint32_t, MbedStorage> myList;
SCCircularBuffer buffer(20);
Thread bufferWriter;
volatile bool running = true;

//
// Yet another thread that constantly raises events to test thread safety of task manager.
//
uint8_t loopCounter = 0;
void bufferWriterThreadProc() {
    while(running) {
        ThisThread::sleep_for(500ms);
        buffer.put(loopCounter++);
    }
}

void log(const char* toLog) {
    console.write(toLog, strlen(toLog));
    console.write("\n", 1);
}

int main() {
    auto frenchWelcome = MbedStorage("Bienvenue");
    myList.add(MbedStorage("Hello world"));
    myList.add(MbedStorage("Hello mbed"));
    myList.add(MbedStorage("Aloha"));
    myList.add(MbedStorage(frenchWelcome));

    log("Iterate list using C++ range");
    for (auto d: myList) {
        log(d.getData());
    }

    log("Iterate list using forEach");
    myList.forEachItem([](MbedStorage *storage) {
        log(storage->getData());
    });

    log("Find an item by key");
    auto toCheck = myList.getByKey(frenchWelcome.getKey());
    if (toCheck && toCheck->getKey() == frenchWelcome.getKey()) {
        log("Found item using key");
    } else {
        log("Find by key failed");
    }

    bufferWriter.start(bufferWriterThreadProc);


    while(1) {
        if(buffer.available()) {
            log("received buffer");
            char sz[50];
            sprintf(sz, "received buffer %i", buffer.get());
            log(sz);
        }
        ThisThread::sleep_for(100ms);
    }
    return 0;
}
