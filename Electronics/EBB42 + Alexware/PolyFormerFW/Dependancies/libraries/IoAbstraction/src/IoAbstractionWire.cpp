/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <IoAbstractionWire.h>

PCF8574IoAbstraction::PCF8574IoAbstraction(uint8_t addr, uint8_t interruptPin, WireType wireImplementation, bool mode16Bit, bool invertedLogic) : lastRead{}, toWrite{} {
	this->wireImpl = wireImplementation;
	this->address = addr;
	this->interruptPin = interruptPin;
    flags = 0;
    bitWrite(flags, NEEDS_WRITE_FLAG, true);
    bitWrite(flags, PCF8575_16BIT_FLAG, mode16Bit);
    bitWrite(flags, INVERTED_LOGIC, invertedLogic);
}

void PCF8574IoAbstraction::pinDirection(pinid_t pin, uint8_t mode) {
	bool invertedLogic = bitRead(flags, INVERTED_LOGIC);

	/*
	 When inverted logic is set to true, we do the inversion in the runLoop().
	 However, inputs need to be always set to HIGH in order the read to work.
	 So it is necessary to flip the value here.
	 */
	if (mode == INPUT || mode == INPUT_PULLUP) {
		overrideReadFlag();
		writeValue(pin, !invertedLogic ? HIGH : LOW);
	}
	else {
		writeValue(pin, LOW);
	}
    bitWrite(flags, NEEDS_WRITE_FLAG, true);
}

void PCF8574IoAbstraction::writeValue(pinid_t pin, uint8_t value) {
    int port = (pin > 7) ? 1 : 0;
	bitWrite(toWrite[port], (pin % 8), value);
    bitWrite(flags, NEEDS_WRITE_FLAG, true);
}

uint8_t PCF8574IoAbstraction::readValue(pinid_t pin) {
    int port = (pin > 7) ? 1 : 0;
    return (lastRead[port] & (1 << (pin % 8))) ? HIGH : LOW;
}

uint8_t PCF8574IoAbstraction::readPort(pinid_t pin) {
    int port = (pin > 7) ? 1 : 0;
    return lastRead[port];
}

void PCF8574IoAbstraction::writePort(pinid_t pin, uint8_t value) {
    int port = (pin > 7) ? 1 : 0;
    toWrite[port] = value;
    bitWrite(flags, NEEDS_WRITE_FLAG, true);
}

bool PCF8574IoAbstraction::runLoop(){
    bool writeOk = true;
    size_t bytesToTransfer = bitRead(flags, PCF8575_16BIT_FLAG) ? 2 : 1;
    bool invertedLogic = bitRead(flags, INVERTED_LOGIC);

    if (bitRead(flags, NEEDS_WRITE_FLAG)) {
        bitWrite(flags, NEEDS_WRITE_FLAG, false);

        uint8_t dataToWrite[2];
        dataToWrite[0] = invertedLogic ? ~toWrite[0] : toWrite[0];
        dataToWrite[1] = invertedLogic ? ~toWrite[1] : toWrite[1];

        writeOk = ioaWireWriteWithRetry(wireImpl, address, dataToWrite, bytesToTransfer);
    }

    if(bitRead(flags, PINS_CONFIGURED_READ_FLAG)) {
        writeOk = writeOk && ioaWireRead(wireImpl, address, lastRead, bytesToTransfer);

        if (invertedLogic) {
            lastRead[0] = ~lastRead[0];
            lastRead[1] = ~lastRead[1];
        }
    }
    return writeOk;
}

void PCF8574IoAbstraction::attachInterrupt(pinid_t /*pin*/, RawIntHandler intHandler, uint8_t /*mode*/) {
	// if there's an interrupt pin set
	if(interruptPin == 0xff) return;

	internalDigitalIo()->pinDirection(interruptPin, INPUT_PULLUP);
	internalDigitalIo()->attachInterrupt(interruptPin, intHandler, FALLING);
}

BasicIoAbstraction* ioFrom8574(uint8_t addr, pinid_t interruptPin, WireType wireImpl, bool invertedLogic) {
	return new PCF8574IoAbstraction(addr, interruptPin, wireImpl, invertedLogic);
}

BasicIoAbstraction* ioFrom8575(uint8_t addr, pinid_t interruptPin, WireType wireImpl, bool invertedLogic) {
    return new PCF8574IoAbstraction(addr, interruptPin, wireImpl, true, invertedLogic);
}

MCP23017IoAbstraction::MCP23017IoAbstraction(uint8_t address, Mcp23xInterruptMode intMode, pinid_t intPinA, pinid_t intPinB, WireType wireImpl) {
	this->wireImpl = wireImpl;
	this->address = address;
	this->intPinA = intPinA;
	this->intPinB = intPinB;
	this->intMode = intMode;
	this->toWrite = this->lastRead = 0;
	this->needsInit = true;
	this->portFlags = 0;
}

void MCP23017IoAbstraction::initDevice() {
	uint8_t controlReg = (readFromDevice(IOCON_ADDR) & 0xff);
	
	if(intPinB == 0xff && intPinA != 0xff) {
		bitSet(controlReg, IOCON_MIRROR_BIT);
	}
	else if(intPinA != 0xff) {
		bitClear(controlReg, IOCON_MIRROR_BIT);
	}

	bitClear(controlReg, IOCON_BANK_BIT);
	bitClear(controlReg, IOCON_SEQOP_BIT);

	uint16_t regToWrite = controlReg | (((uint16_t)controlReg) << 8U);
	writeToDevice(IOCON_ADDR, regToWrite);

	portFlags = 0;
	needsInit = false;
}

void MCP23017IoAbstraction::toggleBitInRegister(uint8_t regAddr, uint8_t theBit, bool value) {
	uint16_t reg = readFromDevice(regAddr);
	bitWrite(reg, theBit, value);

	// for debugging to see the commands being sent, uncomment below
	serlogF4(SER_IOA_DEBUG, "toggle(regAddr, bit, toggle): ", regAddr, theBit, value);
	serlogFHex(SER_IOA_DEBUG, "Value: ", reg);
	// end debugging code

	writeToDevice(regAddr, reg);
}

void MCP23017IoAbstraction::pinDirection(pinid_t pin, uint8_t mode) {
	if(needsInit) initDevice();

	toggleBitInRegister(IODIR_ADDR, pin, (mode == INPUT || mode == INPUT_PULLUP));
	toggleBitInRegister(GPPU_ADDR, pin, mode == INPUT_PULLUP);

	bitSet(portFlags, (pin < 8) ? READER_PORTA_BIT : READER_PORTB_BIT);
}

void MCP23017IoAbstraction::writeValue(pinid_t pin, uint8_t value) {
	if(needsInit) initDevice();

	bitWrite(toWrite, pin, value);
	bitSet(portFlags, (pin < 8) ? CHANGE_PORTA_BIT : CHANGE_PORTB_BIT);
}

uint8_t MCP23017IoAbstraction::readValue(pinid_t pin) {
	return bitRead(lastRead, pin);
}

uint8_t MCP23017IoAbstraction::readPort(pinid_t pin) {
	return (pin < 8) ? (lastRead & 0xff) : (lastRead >> 8);
}

void MCP23017IoAbstraction::writePort(pinid_t pin, uint8_t value) {
	if(pin < 8) {
		toWrite &= 0xff00; 
		toWrite |= value;
		bitSet(portFlags, CHANGE_PORTA_BIT);
	}
	else {
		toWrite &= 0x00ff; 
		toWrite |= ((uint16_t)value << 8);
		bitSet(portFlags, CHANGE_PORTB_BIT);
	}
}

bool MCP23017IoAbstraction::runLoop() {
	if(needsInit) initDevice();

	bool writeOk = true;

	bool flagA = bitRead(portFlags, CHANGE_PORTA_BIT);
	bool flagB = bitRead(portFlags, CHANGE_PORTB_BIT);
	if(flagA && flagB) // write on both ports
		writeOk = writeToDevice(OUTLAT_ADDR, toWrite);
	else if(flagA) 
		writeOk = writeToDevice8(OUTLAT_ADDR, toWrite);
	else if(flagB)
		writeOk = writeToDevice8(OUTLAT_ADDR + 1, toWrite >> 8);

	bitClear(portFlags, CHANGE_PORTA_BIT);
	bitClear(portFlags, CHANGE_PORTB_BIT);

	flagA = bitRead(portFlags, READER_PORTA_BIT);
	flagB = bitRead(portFlags, READER_PORTB_BIT);
	if(flagA && flagB)
		lastRead = readFromDevice(GPIO_ADDR);
	else if(flagA)
		lastRead = readFromDevice8(GPIO_ADDR);
	else if(flagB)
		lastRead = readFromDevice8(GPIO_ADDR + 1) << 8U;

	return writeOk;
}

bool MCP23017IoAbstraction::writeToDevice(uint8_t reg, uint16_t command) {
	uint8_t data[3];
	data[0] = reg;
    data[1] = (uint8_t)command;
    data[2] = (uint8_t)(command>>8);
	return ioaWireWriteWithRetry(wireImpl, address, data, sizeof data);
}

bool MCP23017IoAbstraction::writeToDevice8(uint8_t reg, uint8_t command) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = (uint8_t)command;
    return ioaWireWriteWithRetry(wireImpl, address, data, sizeof data);
}

uint16_t MCP23017IoAbstraction::readFromDevice(uint8_t reg) {
	ioaWireWriteWithRetry(wireImpl, address, &reg, 1, 0, false);

	uint8_t data[2];
	ioaWireRead(wireImpl, address, data, sizeof data);
	// read will get port A first then port B.
	uint8_t portA = data[0];
	uint16_t portB = data[1] << 8U;
	return portA | portB;
}

uint8_t MCP23017IoAbstraction::readFromDevice8(uint8_t reg) {
    ioaWireWriteWithRetry(wireImpl, address, &reg, 1, 0);

	uint8_t buffer[2];
    if(ioaWireRead(wireImpl, address, buffer, 1)){
       return buffer[0];
    }
    return 0;
}

void MCP23017IoAbstraction::attachInterrupt(pinid_t pin, RawIntHandler intHandler, uint8_t mode) {
	// only if there's an interrupt pin set
	if(intPinA == 0xff) return;

	auto  inbuiltIo = internalDigitalIo();
	uint8_t pm = (intMode == ACTIVE_HIGH_OPEN || intMode == ACTIVE_LOW_OPEN) ? INPUT_PULLUP : INPUT;
	uint8_t im = (intMode == ACTIVE_HIGH || intMode == ACTIVE_HIGH_OPEN) ? RISING : FALLING;
	if(intPinB == 0xff) {
        inbuiltIo->pinDirection(intPinA, pm);
        inbuiltIo->attachInterrupt(intPinA, intHandler, im);
	}
	else {
        inbuiltIo->pinDirection(intPinA, pm);
        inbuiltIo->attachInterrupt(intPinA, intHandler, im);
        inbuiltIo->pinDirection(intPinB, pm);
        inbuiltIo->attachInterrupt(intPinB, intHandler, im);
    }

	toggleBitInRegister(GPINTENA_ADDR, pin, true);
	toggleBitInRegister(INTCON_ADDR, pin, mode != CHANGE);
	toggleBitInRegister(DEFVAL_ADDR, pin, mode == FALLING);
}

void MCP23017IoAbstraction::setInvertInputPin(pinid_t pin, bool shouldInvert) {
    toggleBitInRegister(IPOL_ADDR, pin, shouldInvert);
}

IoAbstractionRef ioFrom23017(pinid_t addr, WireType wireImpl) {
	return ioFrom23017IntPerPort(addr, NOT_ENABLED, 0xff, 0xff, wireImpl);
}

IoAbstractionRef ioFrom23017(uint8_t addr, Mcp23xInterruptMode intMode, pinid_t interruptPin, WireType wireImpl) {
	return ioFrom23017IntPerPort(addr, intMode, interruptPin, 0xff, wireImpl);
}

IoAbstractionRef ioFrom23017IntPerPort(pinid_t addr, Mcp23xInterruptMode intMode, pinid_t intPinA, pinid_t intPinB, WireType wireImpl) {
	return new MCP23017IoAbstraction(addr, intMode, intPinA, intPinB, wireImpl);
}