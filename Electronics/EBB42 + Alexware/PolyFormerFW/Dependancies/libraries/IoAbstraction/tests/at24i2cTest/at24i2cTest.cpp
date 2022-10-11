#include <Arduino.h>
#include <IoAbstraction.h>
#include <EepromAbstractionWire.h>

/**
 * This test will clear all data on the rom in question, DO NOT USE UNLESS YOU WANT ALL CONTENTS CLEARED DOWN AND RESET
 */

const uint8_t i2cAddr = 0x50;
const At24EepromType eepromType = PAGESIZE_AT24C128;

const char smallerTestString[] = "Test string that exceeds page size";
const char longerTestString[] = "This is a test string that exceeds page size on larger EEPROMs with big pages";

uint8_t pageSize;
size_t eepromSize;

void fail(const char * why, int where) {
    serdebugF3("Failed: ", why, where);
    while(1);
}

void setup() {
    Wire.begin();
    Serial.begin(115200);
    while(!Serial);

    pageSize = at24PageFromRomSize(eepromType);
    eepromSize = at24ActualSizeFromRomSize(eepromType);

    serdebugF3("Starting wire AT24 test with ", pageSize, eepromSize);

    int loopsPerformed = 0;

    I2cAt24Eeprom eeprom(i2cAddr, eepromType);
    for(int i=0; i<eepromSize; i++) {
        eeprom.write8(i, 0);
        if(eeprom.read8(i) != 0) fail("READ err (expected 0) at ", i);
        if(eeprom.hasErrorOccurred()) fail("ERR during clear", i);
        loopsPerformed++;
    }

    serdebugF2("8 Bit finished OK, loops = ", loopsPerformed);

    for(int i=0; i<(eepromSize - 2); i+=2) {
        eeprom.write16(i, i);
        if(eeprom.read16(i) != i) fail("READ err (expected value) at ", i);
        if(eeprom.hasErrorOccurred()) fail("ERR during 16 bit", i);
        loopsPerformed++;
    }

    serdebugF2("16 Bit finished OK", loopsPerformed);

    eeprom.write32(eepromSize - 10, 0xf00dbeef);
    auto ret = eeprom.read32(eepromSize - 10);
    if(ret != 0xf00dbeef) {
        if(eeprom.hasErrorOccurred()) fail("ERR during 32 bit", 0);
    }

    serdebugF("32 Bit finished OK");


    const char* dataToWrite = (pageSize < 16) ? smallerTestString : longerTestString;
    size_t sizeToWrite = (pageSize < 16) ? sizeof smallerTestString : sizeof longerTestString;
    auto where = eepromSize - (sizeToWrite + 20);
    eeprom.writeArrayToRom(where, reinterpret_cast<const uint8_t *>(dataToWrite), sizeToWrite);
    if(eeprom.hasErrorOccurred()) fail("ERR during array write", 0);

    uint8_t readBack[100];
    eeprom.readIntoMemArray(readBack, where, sizeToWrite);
    if(eeprom.hasErrorOccurred()) fail("ERR during array read", 0);

    serdebugF2("Array read back = ", (const char*)readBack);

    if(strncmp((const char*)readBack, dataToWrite, sizeToWrite) != 0) {
        fail("ARRAY FAIL", 0);
    }

    serdebugF("Array test finished OK");

    eeprom.read8(eepromSize * 2);
    if(!eeprom.hasErrorOccurred()) {
        fail("Oversize not working", 0);
    }

    serdebugF("Oversize finished OK");
}

void loop() {
    taskManager.runLoop();
}
