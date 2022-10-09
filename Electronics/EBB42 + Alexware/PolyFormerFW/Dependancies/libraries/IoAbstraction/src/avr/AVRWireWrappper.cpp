/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "PlatformDeterminationWire.h"

//
// Protected by IOA_DEVELOPMENT_EXPERIMENTAL
// experimental at the moment and should not be used in any code
//
#if defined(IOA_USE_AVR_TWI_DIRECT) && defined(IOA_DEVELOPMENT_EXPERIMENTAL)
#include <TaskManagerIO.h>
#include <IoLogging.h>

#ifndef DEF_TWI_FREQ
#define DEF_TWI_FREQ 100000UL
#endif

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

#define TWSR_STATUS_MASK  0xF8
#define TWSR_VAL_START 0x08
#define TWSR_VAL_REPEATED_START 0x10
#define TWSR_VAL_REPEATED_START 0x10
#define TWSR_VAL_SLW_W_ACK 0x18
#define TWSR_VAL_SLW_W_NACK 0x20
#define TWSR_VAL_DATA_W_ACK 0x28
#define TWSR_VAL_DATA_W_NACK 0x30
#define TWSR_VAL_LOST_BUS 0x38
#define TWSR_VAL_SLW_R_ACK 0x40
#define TWSR_VAL_SLW_R_NACK 0x48
#define TWSR_VAL_DATA_R_ACK 0x50
#define TWSR_VAL_DATA_R_NACK 0x58

class AvrTwiManager : public BaseEvent {
public:
    enum TwiStatus: uint8_t {
        I2C_SEND_ADDRESS, I2C_ADDRESS_SENT, I2C_SEND_DATA, I2C_RECV_DATA,
        I2C_START_NACK = 0x80, I2C_PARTIAL_READ, I2C_OPERATION_SUCCESS, READY_FOR_USE,
        NOT_ENABLED, I2C_HW_ERROR = 0xFF
    };

    enum TwiMode: uint8_t {
        TWI_MODE_READ, TWI_MODE_WRITE, TWI_MODE_WRITE_NO_STOP, TWI_MODE_IS_READY, TWI_MODE_IDLE
    };

private:
    uint8_t buffer[TWI_BUFFER_LENGTH] = {};
    TwiStatus twiStatus = NOT_ENABLED;
    TwiMode twiMode = TWI_MODE_IDLE;
    uint8_t length = 0;
    uint8_t position = 0;
    bool eventRegistered = false;

public:
    ~AvrTwiManager() override = default;

    /** prepare TWI for use */
    void initTwi();

    /** set frequency is based on the scaling info from the AVR TWI manuals. */
    void setFrequency(unsigned long frequency) { TWBR = ((F_CPU / frequency) - 16) / 2; }

    /** send data over the twi bus */
    bool sendData(uint8_t addr, const uint8_t* data, uint8_t len, bool stop);
    /** receive data from the twi bus */
    bool receiveData(uint8_t addr, uint8_t* data, uint8_t len);

    /** check if the bus is ready */
    bool isReady(uint8_t addr);

    //
    // from base event
    //
    void exec() override;

    uint32_t timeOfNextCheck() override;

    void sendAddress(uint8_t addr);

private:
    bool waitWithTimeout(unsigned long timeout = 10000UL);
    bool waitForCompletion(TwiStatus expectedStatus, unsigned long timeout = 1000000UL);
    void prepareNextPhase();
    void sendByteOnWire();
    void readByteFromWire();
    bool startTwi(uint8_t addr, TwiStatus status, TwiMode mode, uint8_t len);
};

void AvrTwiManager::initTwi() {
    twiStatus = READY_FOR_USE;
    twiMode = TWI_MODE_IDLE;

    // pull up the TWI lines
    digitalWrite(SDA, 1);
    digitalWrite(SCL, 1);

    TWSR = 0;
    setFrequency(DEF_TWI_FREQ);
    if(!eventRegistered) {
        eventRegistered = true;
        taskManager.registerEvent(this);
    }
}

void AvrTwiManager::exec() {
    switch(twiStatus) {
        case I2C_ADDRESS_SENT:
            prepareNextPhase();
            break;
        case I2C_SEND_DATA:
            sendByteOnWire();
            break;
        case I2C_RECV_DATA:
            readByteFromWire();
             break;
        default:
            break;
    }
}

inline bool stateRequiresTrigger(AvrTwiManager::TwiStatus twiStatus) {
    return (twiStatus == AvrTwiManager::I2C_ADDRESS_SENT || twiStatus == AvrTwiManager::I2C_SEND_DATA
            || twiStatus == AvrTwiManager::I2C_RECV_DATA);
}

uint32_t AvrTwiManager::timeOfNextCheck() {
    if(twiStatus > 0x7f) {
        return secondsToMicros(10);
    }
    else {
        if(stateRequiresTrigger(twiStatus) && (TWCR & _BV(TWINT))) {
            setTriggered(true);
        }
        return 10;
    }
}

void AvrTwiManager::sendAddress(uint8_t addr) {
    serlogF(SER_IOA_DEBUG, "sendaddr");

    // send a start event
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    if(!waitWithTimeout()) return;

    serlogF(SER_IOA_INFO, "started");

    // check if we started, if not we have a hardware level problem
    uint8_t statusReg = (TWSR & TWSR_STATUS_MASK);
    if(statusReg != TWSR_VAL_START && statusReg != TWSR_VAL_REPEATED_START) {
        twiStatus = I2C_START_NACK;
        serlogF(SER_IOA_DEBUG, "TWI not acked");
        return;
    }

    // now send the address on the bus and set the mode to waiting for address account for read bit here too.
    uint8_t readBit = twiMode == TWI_MODE_READ ? 1 : 0;
    TWDR = (addr << 1) | readBit;
    TWCR = _BV(TWINT) | _BV(TWEN);
    serlogF(SER_IOA_DEBUG, "done");

    // and we are done!
    twiStatus = I2C_ADDRESS_SENT;
}

bool AvrTwiManager::waitWithTimeout(unsigned long timeout) {
    if(twiStatus == I2C_HW_ERROR) {
        serlogF(SER_ERROR, "I2C HW fault on entry!");
        return false;
    }
    long then = micros();
    while(!(TWCR & _BV(TWINT))) {
        if ((micros() - then) > 10000) {
            serlogF4(SER_ERROR, "I2C ERROR! timeout (now, then, tm). ", micros(), then, 10000);
            twiStatus = I2C_HW_ERROR;
            return false;
        }
    }
    return true;
}

void avrTwiStop(unsigned long timeout = 100000UL) {
    serlogF(SER_IOA_DEBUG, "stopping");
    long then = micros();
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);

    // wait for stop
    while (TWCR & (1 << TWSTO)) {
        if((micros() - then) > timeout) {
            serlogF4(SER_IOA_INFO, "I2C stop failed");
            break;
        }
    }
    serlogF(SER_IOA_DEBUG, "stopped");
}

void AvrTwiManager::prepareNextPhase() {
    serlogF4(SER_IOA_DEBUG, "nxtp");

    auto statusBits = (TWSR & TWSR_STATUS_MASK);
    if(statusBits != TWSR_VAL_SLW_W_ACK && statusBits != TWSR_VAL_SLW_R_ACK) {
        twiStatus = I2C_START_NACK;
        return;
    }

    if(twiMode == TWI_MODE_IS_READY) {
        // if we are checking for device ready then just stop here.
        avrTwiStop();
        twiStatus = I2C_OPERATION_SUCCESS;
    }
    else if(twiMode == TWI_MODE_READ) {
        twiStatus = I2C_RECV_DATA;
        readByteFromWire();
    }
    else if(twiMode == TWI_MODE_WRITE || twiMode == TWI_MODE_WRITE_NO_STOP) {
        serlogF(SER_IOA_DEBUG, "Send data");
        twiStatus = I2C_SEND_DATA;
        sendByteOnWire();
    }
}

void AvrTwiManager::sendByteOnWire() {
    if(position < length) {
        serlogF(SER_IOA_DEBUG, "Send data", position, buffer[position]);

        TWDR = buffer[position];
        TWCR = _BV(TWINT) | _BV(TWEN);
        ++position;
    }
    else {
        if(twiMode == TWI_MODE_WRITE) {
            avrTwiStop();
        }
        twiStatus = I2C_OPERATION_SUCCESS;
    }
}

void AvrTwiManager::readByteFromWire() {
    if(position < length) {
        bool more = ((TWSR & TWSR_STATUS_MASK) == TWSR_VAL_DATA_R_ACK);
        buffer[position++] = TWDR;
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        if(!more) {
            twiStatus = (position == length) ? I2C_OPERATION_SUCCESS : I2C_PARTIAL_READ;
        }
    }
    else {
        twiStatus = I2C_OPERATION_SUCCESS;
    }
}

bool AvrTwiManager::waitForCompletion(TwiStatus expectedStatus, unsigned long timeout) {
    serlogF3(SER_IOA_DEBUG,  "wait comp", twiStatus, expectedStatus);
    unsigned long then = micros();
    while(twiStatus != expectedStatus && twiStatus < 0x80) {
        if((micros() - then) > timeout) {
            serlogF(SER_ERROR, "ERROR timeout while waiting");
            twiStatus = I2C_HW_ERROR;
            return false;
        }
        taskManager.yieldForMicros(10);
    }

    bool err = twiStatus == I2C_START_NACK || twiStatus == I2C_HW_ERROR || twiStatus == I2C_PARTIAL_READ;

    if(expectedStatus == READY_FOR_USE) {
        serlogF(SER_IOA_DEBUG, "rfu");
        twiStatus = READY_FOR_USE;
    }
    serlogF(SER_IOA_DEBUG, "end w-comp");

    return !err;
}

bool AvrTwiManager::receiveData(uint8_t addr, uint8_t *data, uint8_t len) {
    if(!data || len > TWI_BUFFER_LENGTH) return false;
    waitForCompletion(READY_FOR_USE);
    startTwi(addr, I2C_SEND_ADDRESS, TWI_MODE_READ, len);
    bool done = waitForCompletion(I2C_OPERATION_SUCCESS);
    if(done) {
        memcpy(data, buffer, len);
        return true;
    }
    return false;

}

bool AvrTwiManager::isReady(uint8_t addr) {
    waitForCompletion(READY_FOR_USE);
    startTwi(addr, I2C_SEND_ADDRESS, TWI_MODE_IS_READY, 0);
    return waitForCompletion(I2C_OPERATION_SUCCESS);
    return false;
}

bool AvrTwiManager::sendData(uint8_t addr, const uint8_t *data, uint8_t len, bool stop) {
    if(!data || len > TWI_BUFFER_LENGTH) return false;
    serlogF(SER_IOA_DEBUG, "send1");
    waitForCompletion(READY_FOR_USE);
    memcpy(buffer, data, len);
    startTwi(addr, I2C_SEND_ADDRESS, twiMode = stop ? TWI_MODE_WRITE : TWI_MODE_WRITE_NO_STOP, len);
    serlogF(SER_IOA_DEBUG, "send2");
    return waitForCompletion(I2C_OPERATION_SUCCESS);
}

bool AvrTwiManager::startTwi(uint8_t addr, AvrTwiManager::TwiStatus status, AvrTwiManager::TwiMode mode, uint8_t len) {
    twiStatus = status;
    twiMode = mode;
    position = 0;
    length = len;
    sendAddress(addr);
    return twiStatus == I2C_ADDRESS_SENT;
}

AvrTwiManager IoaTwi;
WireType defaultWireTypePtr = &IoaTwi;

void ioaWireBegin() {
    IoaTwi.initTwi();
}

bool ioaWireReady(WireType wire, int address) {
    return IoaTwi.isReady(address);
}

void ioaWireSetSpeed(WireType wireType, long frequency) {
    IoaTwi.setFrequency(frequency);
}

bool ioaWireRead(WireType pI2c, int addr, uint8_t* buffer, size_t len) {
    return IoaTwi.receiveData(addr, buffer, len);
}

bool ioaWireWriteWithRetry(WireType pI2c, int address, const uint8_t* buffer, size_t len, int retriesAllowed, bool sendStop) {
    bool ready = retriesAllowed == 0;
    while(retriesAllowed != 0 && !ready) {
        ready = IoaTwi.isReady(retriesAllowed);
        if(!ready) taskManager.yieldForMicros(50);
        retriesAllowed--;
    }
    if(!ready) return false;

    return IoaTwi.sendData(address, buffer, len, sendStop);
}

#endif
