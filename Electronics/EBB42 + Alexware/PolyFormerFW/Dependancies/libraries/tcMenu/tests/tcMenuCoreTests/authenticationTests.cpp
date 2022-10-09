#ifndef _BASE_REMOTE_TESTS_H
#define _BASE_REMOTE_TESTS_H

#include <AUnit.h>
#include <MockEepromAbstraction.h>
#include "RemoteAuthentication.h"
#include "fixtures_extern.h"
using namespace aunit;

void printAllAuthenticatorEntries(EepromAuthenticatorManager& authenticator, const char* why) {
	Serial.print(why);
	Serial.print(". entries : ");
	for (int i = 0; i < authenticator.getNumberOfEntries(); i++) {
		char sz[16];
		authenticator.copyKeyNameToBuffer(i, sz, sizeof(sz));
		if (sz[0] != 0) {
			Serial.print(sz);
			Serial.print('(');
			Serial.print(i);
			Serial.print(") ");
		}
	}
	Serial.println();
}

test(authenticationTest) {
	EepromAuthenticatorManager authenticator;
	authenticator.initialise(&eeprom, 10);

    assertEqual(eeprom.read16(10), uint16_t(0x9B32));

    // we should be in an out the box state, nothing should authenticate.
    assertFalse(authenticator.isAuthenticated("uuid1", uuid1));
    assertFalse(authenticator.isAuthenticated("uuid2", uuid2));
    assertFalse(authenticator.isAuthenticated("uuid3", uuid3));

    // add two keys and ensure we can authenticate with them
    authenticator.addAdditionalUUIDKey("uuid1", uuid1);
    authenticator.addAdditionalUUIDKey("uuid2", uuid2);
    printAllAuthenticatorEntries(authenticator, "After add");

	// now check what we've added
    assertTrue(authenticator.isAuthenticated("uuid1", uuid1));
    assertTrue(authenticator.isAuthenticated("uuid2", uuid2));
    assertFalse(authenticator.isAuthenticated("uuid3", uuid1));

    // re-add uuid1 as a different ID and check the old key doesn't work
    authenticator.addAdditionalUUIDKey("uuid1", uuid3);
    printAllAuthenticatorEntries(authenticator, "After replace");
    assertFalse(authenticator.isAuthenticated("uuid1", uuid1));
    assertTrue(authenticator.isAuthenticated("uuid1", uuid3));

    // re-initialise, should be from eeprom without clearing it
    authenticator.initialise(&eeprom, 10);
    printAllAuthenticatorEntries(authenticator ,"After load");

    // check the state was loaded back properly
    assertTrue(authenticator.isAuthenticated("uuid1", uuid3));
    assertTrue(authenticator.isAuthenticated("uuid2", uuid2));
    assertFalse(authenticator.isAuthenticated("uuid3", uuid2));

	// test clearing down everything, basically wipe eeprom
    authenticator.resetAllKeys();

	// everything should now be gone.
    assertFalse(authenticator.isAuthenticated("uuid1", uuid1));
    assertFalse(authenticator.isAuthenticated("uuid2", uuid1));
    assertFalse(authenticator.isAuthenticated("uuid3", uuid1));
}

test(testNoAuthenicatorMode) {
	NoAuthenticationManager noAuth;

	// does nothing but returns true to fulfil interface.
	assertTrue(noAuth.addAdditionalUUIDKey("uuid22", uuid2));
		
	// does nothing but returns true in all cases.
	assertTrue(noAuth.isAuthenticated("uuid1", uuid1));
	assertTrue(noAuth.isAuthenticated("anything", uuid2));
	assertTrue(noAuth.isAuthenticated("...", uuid3));
}

const AuthBlock authBlocks[] PROGMEM = {
	{ "uuid1", "07cd8bc6-734d-43da-84e7-6084990becfc" },  // UUID1
	{ "uuid2", "07cd8bc6-734d-43da-84e7-6084990becfd" }   // UUID2
};

const char pgmPassword[] PROGMEM = "1234";

test(testProgmemAuthenicatorMode) {
	ReadOnlyAuthenticationManager roAuth(authBlocks, 2, pgmPassword);

	// check the ones we know should work
	assertTrue(roAuth.isAuthenticated("uuid1", uuid1));
	assertTrue(roAuth.isAuthenticated("uuid2", uuid2));

	// now try combinations of ones that should not.
	assertFalse(roAuth.isAuthenticated("anything", uuid1));
	assertFalse(roAuth.isAuthenticated("uuid2", uuid1));
	assertFalse(roAuth.isAuthenticated("uuid1", uuid2));
	
	// it is not possible to add keys to this manager.
	assertFalse(roAuth.addAdditionalUUIDKey("anyKey", uuid3));
}

#endif