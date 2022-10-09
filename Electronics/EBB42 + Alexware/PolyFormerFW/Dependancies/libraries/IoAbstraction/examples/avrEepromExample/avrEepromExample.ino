/**
 * This sketch is part of IOAbstraction, it shows how to use the EEPROM abstraction,
 * for which you can choose from NoEeprom, AvrEeprom and I2cAt24C based eeproms.
 * This example shows both AvrEeprom and wrapping the standard EEPROM, but could 
 * equally be replaced by any of the others.
 *
 * This allows any libraries or code you write to work easily across 8 and 32 bit
 * machines by allowing you to decide what type of eeprom you have at compile / runtime.
 *
 * Note that running this sketch WILL WRITE INTO THE SELECTED ROM at the location
 * starting at romStart.
 *
 * It writes a byte, int, double and string to the eeprom and reads them back.
 */

// you always needs this include.
#include <EepromAbstraction.h>
#include <ArduinoEEPROMAbstraction.h>
#include <TaskManagerIO.h>
#include <Wire.h>

const unsigned int romStart = 400;

AvrEeprom anEeprom;
ArduinoEEPROMAbstraction eepromWrapper(&EEPROM);

const char strData[15] = { "Hello EEPROM"};

void setup() {
	Serial.begin(115200);
	while(!Serial);

	Serial.println("Eeprom example starting");

	// now write the values to the rom. 8, 16 and 32 bit
	anEeprom.write8(romStart, (byte)42);
	anEeprom.write16(romStart + 1, 0xface);
	anEeprom.write32(romStart + 3, 0xf00dd00d);
	
	// lastly write an array to the rom.
	anEeprom.writeArrayToRom(romStart + 7, (const unsigned char*)strData, sizeof strData);

	Serial.println("Eeprom example written initial values, starting on EEPROM proxy example");

    // here I show another way to wrap the EEPROM class into an IO abstraction, prefer local instantiation, it's small.

    eepromWrapper.write8(romStart + 30, 99);
    eepromWrapper.write16(romStart + 31, 0xf00d);
    eepromWrapper.write32(romStart + 33, 0xfade0ff);
    eepromWrapper.writeArrayToRom(romStart + 37, (const unsigned char*)strData, sizeof strData);
    //and if your device needs a commit operation, do it here. For example:
    //EEPROM.commit();

	Serial.println("All values written out");
}

void loop() {

    //
    // First we read back using the AVR eeprom directly.
    //

	Serial.print("Reading back byte: ");
	Serial.println(anEeprom.read8(romStart));

	Serial.print("Reading back word: 0x");
	Serial.println(anEeprom.read16(romStart + 1), HEX);

	Serial.print("Reading back long: 0x");
	Serial.println(anEeprom.read32(romStart + 3), HEX);

	Serial.print("Rom Array: ");
	char readBuffer[15];
	anEeprom.readIntoMemArray((unsigned char*)readBuffer, romStart + 7, sizeof readBuffer);
	Serial.println(readBuffer);

    //
    // Now we read back using the EEPROM class wrapper
    //

    Serial.println("Now reading back using ArduinoEEPROMAbstraction");

	Serial.print("Reading back byte: ");
	Serial.println(eepromWrapper.read8(romStart + 30));

	Serial.print("Reading back word: 0x");
	Serial.println(eepromWrapper.read16(romStart + 31), HEX);

	Serial.print("Reading back long: 0x");
	Serial.println(eepromWrapper.read32(romStart + 33), HEX);

    Serial.print("Rom array: ");
	eepromWrapper.readIntoMemArray((unsigned char*)readBuffer, romStart + 37, sizeof readBuffer);
	Serial.println(readBuffer);

	delay(10000);
}

