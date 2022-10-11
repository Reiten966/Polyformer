/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef SIMPLECOLLECTIONS_CIRCULARBUFFER_H
#define SIMPLECOLLECTIONS_CIRCULARBUFFER_H

#include <string.h>
#include <inttypes.h>
#include <SCThreadingSupport.h>

namespace tccollection {

/**
 * Create a circular buffer that has a fixed size and can be filled and read at the same time one character at a time.
 * It is very thread safe on ESP32 and mbed based boards, it is thread safe on other boards, unless you are using that
 * board with an unexpected RTOS, in which case you would need to determine suitability yourself.
 *
 * Imagine the buffer like a circle with a given number of segments, both the reader and writer pointers start at 0.
 * As the data structure fills up the writer will move around, and the reader can try and "keep up". Once the reader
 * or writer gets to the end it will go back to 0 (start). This is by design and therefore, IT'S POSSIBLE TO LOSE DATA.
 */

    template<class T> class GenericCircularBuffer {
    public:
        enum BufferType { CIRCULAR_BUFFER, MEMORY_POOL };

#ifdef SC_DEBUG_CHECKER
        volatile uint32_t casExchangeSpins = 0;
#endif

    private:
        position_t readerPosition;
        position_t writerPosition;
        const uint16_t bufferSize;
        T *const buffer;
        bool actingAsMemoryPool;

    public:
        explicit GenericCircularBuffer(uint16_t size, BufferType asMemPool = CIRCULAR_BUFFER) : readerPosition(0), writerPosition(0),
                                bufferSize(size), buffer(new T[size]), actingAsMemoryPool(asMemPool) {}
        ~GenericCircularBuffer() { delete[] buffer; }

        bool available() const { return actingAsMemoryPool || readerPosition != writerPosition; }
        void put(const T& by) { buffer[nextPosition(&writerPosition)] = by; }
        T& get() { return buffer[nextPosition(&readerPosition)]; }

        int16_t nextPosition(position_ptr_t positionPtr) {
            bool successfullyUpdated = false;
            position_t existing;
            while(!successfullyUpdated) {
                existing = readAtomic(positionPtr);
                position_t newPos = existing + 1;
                if (newPos >= bufferSize) {
                    newPos = 0;
                }
                successfullyUpdated = casAtomic(positionPtr, existing, newPos);
#ifdef SC_DEBUG_CHECKER
                if(!successfullyUpdated) casExchangeSpins++;
#endif
            }
            return existing;
        }
    };

    class SCCircularBuffer {
    private:
        position_t readerPosition;
        position_t writerPosition;
        const uint16_t bufferSize;
        uint8_t *const buffer;

    public:
        explicit SCCircularBuffer(uint16_t size);

        ~SCCircularBuffer();

        bool available() const { return readerPosition != writerPosition; }

        void put(uint8_t by) { buffer[nextPosition(&writerPosition)] = by; }

        uint8_t get() { return buffer[nextPosition(&readerPosition)]; }

    private:
        uint16_t nextPosition(position_ptr_t by);
    };

}



#endif //SIMPLECOLLECTIONS_CIRCULARBUFFER_H
