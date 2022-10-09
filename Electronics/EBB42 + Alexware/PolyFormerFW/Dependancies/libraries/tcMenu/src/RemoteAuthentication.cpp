/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "RemoteAuthentication.h"
#include <IoLogging.h>

void EepromAuthenticatorManager::initialise(EepromAbstraction* eeprom, EepromPosition start, uint16_t magicKey) {
    this->romStart = start;
    this->eeprom = eeprom;
    this->magicKey = magicKey;
    if(eeprom->read16(romStart) != magicKey) {
        // we need a clean start, the magic key has not been written at position 0
        resetAllKeys();
    }
}

bool EepromAuthenticatorManager::addAdditionalUUIDKey(const char* connectionName, const char* uuid) {
    if(eeprom == NULL) {
        serlogF(SER_ERROR, "EEPROM Auth not initialised!!");
        return false;
    }

    // find a space for the key
    int insertAt = findSlotFor(connectionName);
    if(insertAt == -1) {
        serlogF2(SER_ERROR, "Add Key failure ", connectionName);
        return false; // no spaces left
    } 

    // temp space to store the strings and ensure zero terminated.
    char buffer[UUID_KEY_SIZE];
    
    // buffer and then write out the name
    strncpy(buffer, connectionName, sizeof(buffer));
    buffer[CLIENT_DESC_SIZE-1] = 0;
    eeprom->writeArrayToRom(eepromOffset(insertAt), reinterpret_cast<const uint8_t*>(buffer), CLIENT_DESC_SIZE);
    
    // buffer and then write out the uuid
    strncpy(buffer, uuid, sizeof(buffer));
    buffer[UUID_KEY_SIZE-1] = 0;
    eeprom->writeArrayToRom(eepromOffset(insertAt) + CLIENT_DESC_SIZE, reinterpret_cast<const uint8_t*>(buffer), UUID_KEY_SIZE);

    serlogF2(SER_TCMENU_INFO, "Add Key success ", connectionName);
    return true;
}

bool EepromAuthenticatorManager::isAuthenticated(const char* connectionName, const char* authResponse) {
    if(eeprom == NULL) {
        serlogF(SER_ERROR, "EEPROM Auth not initialised!!");
        return false;
    }
    char buffer[UUID_KEY_SIZE];
    int i = findSlotFor(connectionName);
    if(i != -1) {
        eeprom->readIntoMemArray(reinterpret_cast<uint8_t*>(buffer), eepromOffset(i) + CLIENT_DESC_SIZE, sizeof(buffer));
        buffer[UUID_KEY_SIZE-1] = 0;
        serlogF3(SER_TCMENU_DEBUG, "uuid rom, mem ", buffer, authResponse)
        if(strcmp(buffer, authResponse) == 0) {
            serlogF2(SER_TCMENU_INFO, "Authenticated ", connectionName);
            return true;
        }
        else {
            serlogF2(SER_TCMENU_INFO, "Invalid Key ", connectionName);
        }
    }
    serlogF2(SER_TCMENU_INFO, "Not found ", connectionName);
    return false;
}

void EepromAuthenticatorManager::copyKeyNameToBuffer(int idx, char* buffer, int bufSize) {
    if(idx < 0 || idx >= numberOfEntries) {
        buffer[0]=0;
        return;
    }

    eeprom->readIntoMemArray(reinterpret_cast<uint8_t*>(buffer), eepromOffset(idx), min(bufSize, CLIENT_DESC_SIZE));
    buffer[bufSize-1]=0;
}

void EepromAuthenticatorManager::resetAllKeys() {
    serlogF2(SER_WARNING, "Resetting auth store: ", numberOfEntries);
    eeprom->write16(romStart, magicKey);
    for(int i=0; i<numberOfEntries;i++) {
        // we just zero the name and UUID first character, to clear it.
        eeprom->write8(eepromOffset(i), 0);
        eeprom->write8(eepromOffset(i) + CLIENT_DESC_SIZE, 0);
    }
	changePin("1234");
    serlogF(SER_WARNING, "Finished reset of auth store. Pin is now 1234");
}

int EepromAuthenticatorManager::findSlotFor(const char* name) {
    int emptySlot = -1;
    for(int i=0;i<numberOfEntries;i++) {
        uint8_t val = eeprom->read8(eepromOffset(i));
        if(emptySlot == -1 && val == 0) {
            // if there's an empty slot and we haven't got one, then record it.
            emptySlot = i;
        }
        else if(val != 0) {
            // however if there's an existing slot for the same name this takes priority
            char buffer[CLIENT_DESC_SIZE];
            eeprom->readIntoMemArray(reinterpret_cast<uint8_t*>(buffer), eepromOffset(i), CLIENT_DESC_SIZE);
            buffer[CLIENT_DESC_SIZE-1]=0;
            if(strcmp(buffer, name)==0) {
                // existing name found, so we give this index back.
                return i;
            }
        }
    }
    return emptySlot;
}

bool EepromAuthenticatorManager::doesPinMatch(const char* pinAttempt) {
	char eepromPin[MAX_PIN_LENGTH];
	eeprom->readIntoMemArray(reinterpret_cast<uint8_t*>(eepromPin), eepromOffset(numberOfEntries), MAX_PIN_LENGTH);
	eepromPin[MAX_PIN_LENGTH - 1] = 0;
	return strncmp(eepromPin, pinAttempt, MAX_PIN_LENGTH) == 0;
}

void EepromAuthenticatorManager::copyPinToBuffer(char* buffer, int size) {
	eeprom->readIntoMemArray((uint8_t*)buffer, eepromOffset(numberOfEntries), size);
	buffer[size - 1] = 0;
}

void EepromAuthenticatorManager::changePin(const char* newPin) {
	eeprom->writeArrayToRom(eepromOffset(numberOfEntries), (uint8_t*)newPin, MAX_PIN_LENGTH);
}

bool ReadOnlyAuthenticationManager::isAuthenticated(const char* connectionName, const char* authResponse) { 
	if (authBlocksPgm == NULL) return false;

    for(int i=0;i<numberOfEntries;i++) {
        if(strcmp_P(connectionName, authBlocksPgm[i].name) == 0) {                
            bool keyMatch = strcmp_P(authResponse, authBlocksPgm[i].uuid) == 0;
            serlogF3(SER_TCMENU_INFO, "AuthBlock found (name, match) ", connectionName, keyMatch);
            return keyMatch;
        }
    }
    serlogF2(SER_TCMENU_INFO, "AuthBlock not found for ", connectionName);
    return false;
}
