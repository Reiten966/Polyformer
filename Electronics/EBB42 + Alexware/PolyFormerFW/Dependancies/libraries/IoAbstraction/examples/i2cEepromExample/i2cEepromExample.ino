/**
 * This sketch is part of IOAbstraction, it shows how to use the EEPROM abstraction,
 * for which you can choose from NoEeprom, AvrEeprom and I2cAt24C based eeproms.
 * This example chooses AvrEeprom, but could equally be replaced by any of the others.
 *
 * This allows any libraries or code you write to work easily across 8 and 32 bit
 * machines by allowing you to decide what type of eeprom you have at compile / runtime.
 *
 * Note that running this sketch WILL WRITE INTO THE SELECTED ROM at the location
 * starting at romStart.
 *
 * It writes a byte, int, double and string to the eeprom and reads them back.
 */

// We have a direct dependency on Wire and Arduino ships it as a library for every board
// therefore to ensure compilation we include it here.
#include <Wire.h>

// you always needs this include.
#include <EepromAbstractionWire.h>
#include <TaskManagerIO.h>

const unsigned int romStart = 800;

// When you want to use the AVR built in EEPROM support (only available on AVR)
// comment / uncomment to select
I2cAt24Eeprom anEeprom(0x50, PAGESIZE_AT24C128);

const char strData[100] = { "This is a quite long string that should need to be handled in many parts with wait states"};

void setup() {
	Serial.begin(115200);
	while(!Serial);

	// if you are using the i2c eeprom, you must include this line below, not needed otherwise.
	Wire.begin();

	Serial.println("Eeprom example starting");

	// clear the ROM first..
	for(int i=romStart;i<(romStart+100);i++) anEeprom.write8(i, 0);
    Serial.println(anEeprom.hasErrorOccurred() ? "Write failure" : "Write success");

	// now write the values to the rom. 8, 16 and 32 bit
	anEeprom.write8(romStart, (byte)42);
	anEeprom.write16(romStart + 1, 0xface);
	anEeprom.write32(romStart + 3, 0xf00dface);
    Serial.println(anEeprom.hasErrorOccurred() ? "Write failure" : "Write success");

    // lastly write an array to the rom.
	anEeprom.writeArrayToRom(romStart + 7, (const unsigned char*)strData, sizeof strData);

    Serial.println(anEeprom.hasErrorOccurred() ? "Write failure" : "Write success");

	Serial.println("Eeprom example written initial values");
	
	// we can check if there are any errors writing by calling hasErrorOccurred, for AVR there is never an error.
	// but for i2c variants there may well be.
}

void loop() {

	Serial.print("Reading back byte: ");
	Serial.println(anEeprom.read8(romStart));

	Serial.print("Reading back word: 0x");
	Serial.println(anEeprom.read16(romStart + 1), HEX);

	Serial.print("Reading back long: 0x");
	Serial.println(anEeprom.read32(romStart + 3), HEX);

	// finally we'll do hard comparisons against the array, as it's hard to check by hand.
	char readBuffer[100];
	anEeprom.readIntoMemArray((unsigned char*)readBuffer, romStart + 7, sizeof readBuffer);
	Serial.print("Rom Array: ");
	Serial.println(readBuffer);
	Serial.print("String is same: ");
	Serial.println(strcmp(readBuffer, strData)==0 ? "YES":"NO");


	// we can check if there are any errors writing by calling hasErrorOccurred, for AVR there is never an error.
	// but for i2c variants there may well be.
	Serial.println(anEeprom.hasErrorOccurred() ? "Read error" : "Successfully");

	delay(10000);
}

