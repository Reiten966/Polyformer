/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */
#include <EepromAbstraction.h>

#ifdef __AVR__

uint8_t AvrEeprom::read8(EepromPosition position) {
	return eeprom_read_byte((uint8_t*)position);
}

void AvrEeprom::write8(EepromPosition position, uint8_t val) {
	if(read8(position) != val) {
		eeprom_write_byte((uint8_t*)position, val);
	}
}

uint16_t AvrEeprom::read16(EepromPosition position) {
	return eeprom_read_word((uint16_t*)position);
}

void AvrEeprom::write16(EepromPosition position, uint16_t val) {
	if(read16(position) != val) {
		eeprom_write_word((uint16_t*)position, val);
	}
}

uint32_t AvrEeprom::read32(EepromPosition position) {
	return eeprom_read_dword((uint32_t*)position);
}

void AvrEeprom::write32(EepromPosition position, uint32_t val) {
	if(read32(position) != val) {
		eeprom_write_dword((uint32_t*)position, val);
	}
}

void AvrEeprom::readIntoMemArray(uint8_t* memDest, EepromPosition romSrc, uint8_t len) {
	eeprom_read_block(memDest, (uint8_t*)romSrc, len);
}

void AvrEeprom::writeArrayToRom(EepromPosition romDest, const uint8_t* memSrc, uint8_t len) {
	bool changed = false;
	for(uint8_t i = 0;i < len; ++i) {
		changed = changed || (read8(romDest + i) != memSrc[i]);
	}

	if(changed) {
		eeprom_write_block(memSrc, (uint8_t*)romDest, len);
	}
}

#endif // AVR ONLY
