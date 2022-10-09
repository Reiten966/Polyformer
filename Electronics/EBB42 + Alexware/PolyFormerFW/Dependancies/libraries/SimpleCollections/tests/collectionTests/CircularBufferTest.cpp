
#include <SimpleCollections.h>
#include <SCCircularBuffer.h>
#include <AUnit.h>
#include <IoLogging.h>

void putIntoBuffer(SCCircularBuffer& buffer, const char* data) {
    while(*data) {
        buffer.put(*data);
        data++;
    }
}

bool verifyBuffer(SCCircularBuffer& buffer, const char* data) {
    bool ret = true;
    while(*data) {
        if(!buffer.available()) {
            serdebugF2("Not available at ", *data);
            return false;
        }
        else {
            auto act = buffer.get();
            serdebugF3("Read expected, actual: ", *data, (char)act);
            if(act != *data) ret = false;
        }
        data++;
    }
    return ret;
}

test(testWritingAndThenReadingWithoutLoss) {
    SCCircularBuffer buffer(20);

    putIntoBuffer(buffer, "hello");
    assertTrue(verifyBuffer(buffer, "hello"));
    assertFalse(buffer.available());

    putIntoBuffer(buffer, "world");
    assertTrue(verifyBuffer(buffer, "world"));
    assertFalse(buffer.available());

    putIntoBuffer(buffer, "0123456789");
    assertTrue(verifyBuffer(buffer, "0123456789"));
    assertFalse(buffer.available());
}

test(testWritingAndThenReadingMoreThanAvailable) {
    SCCircularBuffer buffer(20);

    putIntoBuffer(buffer, "this is longer than 20 chars");
    // the buffer has wrapped, so we have lost everything before the wrapping point basically
    assertTrue(verifyBuffer(buffer, "20 chars"));
    assertFalse(buffer.available());
}

#ifdef ESP32
#include <pthread.h>

volatile bool testRunning = true;
SCCircularBuffer glBuffer(200);
uint8_t counter = 0;

void* threadProc(void*) {
    while(testRunning && counter < 200) {
        glBuffer.put(counter);
        counter++;
        vTaskDelay(1);
    }
    return nullptr;
}

test(testThreadedWriterAndReader) {
    pthread_t myThreadPtr;
    pthread_create(&myThreadPtr, nullptr, threadProc, (void*) nullptr);

    auto millisThen = millis();
    uint8_t myCount = 0;
    int itemsReceived = 0;
    while(myCount < 199U && (millis() - millisThen) < 1000) {
        if (glBuffer.available()) {
            myCount = glBuffer.get();
            serdebugF2("Thread read ", myCount);
            itemsReceived++;
        }
        else vPortYield();
    }

    assertMore(myCount, (uint8_t)198);
    assertMore(itemsReceived, (uint8_t)190);
}

#endif