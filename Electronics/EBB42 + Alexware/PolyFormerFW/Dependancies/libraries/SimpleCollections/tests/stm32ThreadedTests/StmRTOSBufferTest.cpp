
#include <SimpleCollections.h>
#include <SCCircularBuffer.h>
#include <AUnit.h>
#include <IoLogging.h>
#include <STM32FreeRTOS.h>

volatile bool testRunning = true;
SCCircularBuffer glBuffer(200);
uint8_t counter = 0;

void threadProc(void*) {
    while(testRunning && counter < 200) {
        glBuffer.put(counter);
        counter++;
        osDelay(1);
    }
    vTaskDelete(nullptr);
}

test(testThreadedWriterAndReaderByte) {
    testRunning = true;
    Serial.println("Starting threaded for byte");
    xTaskCreate(threadProc, "writer", configMINIMAL_STACK_SIZE * 2, nullptr, 2, nullptr);

    auto millisThen = millis();
    uint8_t myCount = 0;
    int itemsReceived = 0;
    while(myCount < 199U && (millis() - millisThen) < 1000) {
        if (glBuffer.available()) {
            myCount = glBuffer.get();
            serdebugF2("Thread read ", myCount);
            itemsReceived++;
        }
        else osThreadYield();
    }

    assertMore(myCount, (uint8_t)198);
    assertMore(itemsReceived, (uint8_t)190);
    testRunning = false;
}

struct WriterStruct {
    uint32_t sequence;
    uint32_t data1;
    uint32_t data2;

    void setData(uint32_t s, uint32_t d1, uint32_t d2) {
        sequence = s;
        data1 = d1;
        data2 = d2;
    }
};

//
// The only safe way to work with multibyte objects is to have a pool of them the same size as the buffer itself.
// We test this use case here. Many threads writing often may not be a good use case for spin locking, it's going to
// spin an awful lot of the time, and proper semaphore/mutex construct *may* be better when there are many writers.
//
GenericCircularBuffer<WriterStruct> writerMemoryAlloc(10, GenericCircularBuffer<WriterStruct>::MEMORY_POOL);
GenericCircularBuffer<WriterStruct*> actualBuffer(10);

struct ThreadRange {
    uint32_t start;
    uint32_t end;

    ThreadRange(uint32_t s, uint32_t e) {
        start = s;
        end = e;
    }
};

void threadedStructWriter(void* data) {
    auto range = reinterpret_cast<ThreadRange*>(data);
    uint32_t nextSequence = range->start;
    while(testRunning && nextSequence < range->end) {
        auto &alloc = writerMemoryAlloc.get();
        alloc.setData(nextSequence, nextSequence * 1000, nextSequence * 2000);
        actualBuffer.put(&alloc);
        nextSequence++;
        osThreadYield();
    }
    vTaskDelete(nullptr);
}

test(testTwoLevelWriterAndReader) {
    uint32_t sequencesRx[129] = {0};
    testRunning = true;
    Serial.println("Starting threaded for struct");
    ThreadRange range1(0, 1024);
    ThreadRange range2(1024, 2048);
    ThreadRange range3(2048, 4096);
    xTaskCreate(threadedStructWriter, "writer1", configMINIMAL_STACK_SIZE * 2, &range1, 2, nullptr);
    xTaskCreate(threadedStructWriter, "writer2", configMINIMAL_STACK_SIZE * 2, &range2, 2, nullptr);
    xTaskCreate(threadedStructWriter, "writer3", configMINIMAL_STACK_SIZE * 2, &range3, 2, nullptr);

    auto received = 0;
    auto then = millis();
    while(received < 4096 && (millis() - then) < 3000) {
        if(actualBuffer.available()) {
            auto myData = actualBuffer.get();
            assertEqual(myData->data1, myData->sequence * 1000U);
            assertLess(myData->sequence, (uint32_t)4096U);
            bitSet(sequencesRx[myData->sequence / 32], (myData->sequence % 32));
            received++;
        }
    }

    bool sequenceIncorrect = false;
    for(uint32_t i=0; i<4096; i++) {
        if (bitRead(sequencesRx[i / 32], (i % 32)) == false) {
            serdebugF2("Sequence was not received ", i);
            sequenceIncorrect = true;
        }
    }
    assertFalse(sequenceIncorrect);

    // on smaller cores this value will invariably be 0, I may try and compile this code onto my mac or PI so that
    // I can test it in an environment with enough cores and cache that we get more concurrency.
    serdebugF2("Number of spins during locking ", actualBuffer.casExchangeSpins);

    testRunning = false;
}