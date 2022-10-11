/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <inttypes.h>
#include "SwitchInput.h"

#define ONE_TURN_OF_ENCODER 32

SwitchInput switches;

void registerInterrupt(pinid_t pin);
void onSwitchesInterrupt(__attribute__((unused)) pinid_t pin);

KeyboardItem::KeyboardItem() : stateFlags(NOT_PRESSED), previousState(NOT_PRESSED), pin(-1), counter(0), acceleration(0),
                               repeatInterval(NO_REPEAT), notify{}, callbackOnRelease{} {}

KeyboardItem::KeyboardItem(pinid_t pin, KeyCallbackFn callback, uint8_t repeatInterval, bool keyLogicIsInverted) : notify{} {
    this->repeatInterval = repeatInterval;
    this->pin = pin;
	this->counter = 0;
    this->notify.callback = callback;
	previousState = NOT_PRESSED;
	stateFlags = NOT_PRESSED;
	callbackOnRelease = nullptr;
	acceleration = 0;
	bitWrite(stateFlags, KEY_LISTENER_MODE_BIT, 0);
	bitWrite(stateFlags, KEY_LOGIC_IS_INVERTED, keyLogicIsInverted);
}

KeyboardItem::KeyboardItem(pinid_t pin, SwitchListener* switchListener, uint8_t repeatInterval, bool keyLogicIsInverted) : notify{} {
	this->pin = pin;
	this->repeatInterval = repeatInterval;
    this->notify.listener = switchListener;
    this->counter = 0;
    previousState = NOT_PRESSED;
	stateFlags = NOT_PRESSED;
	callbackOnRelease = nullptr;
	acceleration = 0;
	bitWrite(stateFlags, KEY_LISTENER_MODE_BIT, 1);
	bitWrite(stateFlags, KEY_LOGIC_IS_INVERTED, keyLogicIsInverted);
}

KeyboardItem::KeyboardItem(const KeyboardItem& other) = default;
KeyboardItem& KeyboardItem::operator=(const KeyboardItem& other) = default;

void KeyboardItem::onRelease(KeyCallbackFn cb) {
	this->callbackOnRelease = cb;
}

void KeyboardItem::trigger(bool held) {
	if (!notify.callback) return;

	if (isUsingListener()) notify.listener->onPressed(pin, held);
	else notify.callback(pin, held);
}

void KeyboardItem::triggerRelease(bool held) {
	if (isUsingListener()) notify.listener->onReleased(pin, held);
	else if(callbackOnRelease) callbackOnRelease(pin, held);
}

void KeyboardItem::changeOnPressed(KeyCallbackFn pFunction) {
    bitWrite(stateFlags, KEY_LISTENER_MODE_BIT, 0);
    this->notify.callback = pFunction;
}

void KeyboardItem::changeListener(SwitchListener* listener) {
    bitWrite(stateFlags, KEY_LISTENER_MODE_BIT, 1);
    this->notify.listener = listener;
}


void KeyboardItem::checkAndTrigger(uint8_t buttonState){
	if (notify.callback == nullptr && callbackOnRelease == nullptr) return;

	if (buttonState == HIGH) {
		if (getState() == NOT_PRESSED) {
			setState(DEBOUNCING1);
		}
		else if (isDebouncing()) {
			setState(PRESSED);
			previousState = PRESSED;
			counter = 0; 
			acceleration = 1;
			trigger(false);
				
		}
		else if (getState() == PRESSED) {
			counter++;
			if (counter > HOLD_THRESHOLD) {
				setState(BUTTON_HELD);
				previousState = BUTTON_HELD;
				trigger(true);
				counter = 0;
				acceleration = 1;
			}
		}
		else if (getState() == BUTTON_HELD && repeatInterval != NO_REPEAT && notify.callback != nullptr) {
			counter = counter + (acceleration >> 2) + 1;
			if (counter > repeatInterval) {
				acceleration = min(255, acceleration + 1);
				trigger(true);
				counter = 0;
			}
		}
	}
	else if(getState() == DEBOUNCING1) {
		setState(DEBOUNCING2);
	}
	else {
		setState(NOT_PRESSED);
		if (previousState == PRESSED) {
			previousState = NOT_PRESSED;
			triggerRelease(false);
		} else if (previousState == BUTTON_HELD){
			previousState = NOT_PRESSED;
			triggerRelease(true);
		}
	}
}

SwitchInput::SwitchInput() : encoder{}, keys(MAX_KEYS) {
	this->ioDevice = nullptr;
	this->swFlags = 0;
    this->lastSyncStatus = true;
}

void SwitchInput::initialiseInterrupt(IoAbstractionRef device, bool usePullUpSwitching) {
	this->init(device, SWITCHES_NO_POLLING, usePullUpSwitching);
}

void SwitchInput::initialise(IoAbstractionRef device, bool usePullUpSwitching) {
	this->init(device, SWITCHES_POLL_KEYS_ONLY, usePullUpSwitching);
}

void SwitchInput::init(IoAbstractionRef device, SwitchInterruptMode mode, bool defaultIsPullUp) {
	this->ioDevice = device;

	// set up the flags
	this->swFlags = 0;
	bitWrite(swFlags, SW_FLAG_PULLUP_LOGIC, defaultIsPullUp);
	bitWrite(swFlags, SW_FLAG_INTERRUPT_DRIVEN, (mode == SWITCHES_NO_POLLING));
	bitWrite(swFlags, SW_FLAG_ENCODER_IS_POLLING, (mode == SWITCHES_POLL_EVERYTHING));

	if(mode == SWITCHES_POLL_KEYS_ONLY) {
		serlogF(SER_IOA_INFO, "Switches polling for keys");
		taskManager.scheduleFixedRate(SWITCH_POLL_INTERVAL, [] {
			switches.runLoop();
		});
	} else if(mode == SWITCHES_POLL_EVERYTHING) {
        serlogF(SER_IOA_INFO, "Switches polling for everything");
		taskManager.scheduleFixedRate(SWITCH_POLL_INTERVAL / 4, [] {
            static uint8_t counter = 0;
            if(++counter % 4 == 3) {
                switches.runLoop();
            }
			onSwitchesInterrupt(-1);
		});
	}

	serlogF4(SER_IOA_INFO, "Switches initialized (pull-up, int, encPoll)", bitRead(swFlags, SW_FLAG_PULLUP_LOGIC), bitRead(swFlags, SW_FLAG_INTERRUPT_DRIVEN),
			   bitRead(swFlags, SW_FLAG_ENCODER_IS_POLLING));
}

bool SwitchInput::addSwitch(pinid_t pin, KeyCallbackFn callback,uint8_t repeat, bool invertLogic) {
	internalAddSwitch(pin, invertLogic);
    return keys.add(KeyboardItem(pin, callback, repeat, invertLogic));
}

bool SwitchInput::addSwitchListener(pinid_t pin, SwitchListener* listener, uint8_t repeat, bool invertLogic) {
	internalAddSwitch(pin, invertLogic);
    return keys.add(KeyboardItem(pin, listener, repeat, invertLogic));
}

bool SwitchInput::internalAddSwitch(pinid_t pin, bool invertLogic) {
	if (ioDevice == nullptr) initialise(internalDigitalIo(), true);

	ioDevicePinMode(ioDevice, pin, isPullupLogic(invertLogic) ? INPUT_PULLUP : INPUT);

    if (isInterruptDriven()) {
		registerInterrupt(pin);
	}

    return true;
}

void SwitchInput::onRelease(pinid_t pin, KeyCallbackFn callbackOnRelease) {
	if (ioDevice == nullptr) initialise(internalDigitalIo(), true);

	auto keyItem = keys.getByKey(pin);
	if(keyItem) {
	    // already initialised, just add the release callback
	    keyItem->onRelease(callbackOnRelease);
	}
	else {
        internalAddSwitch(pin, false);
	    // not yet added, we will do a best efforts standard initialisation.
        KeyboardItem newItem(pin, (KeyCallbackFn) nullptr, NO_REPEAT, false);
        newItem.onRelease(callbackOnRelease);
        keys.add(newItem);
    }
}

void SwitchInput::replaceOnPressed(pinid_t pin, KeyCallbackFn callbackOnPressed) {
    auto keyItem = keys.getByKey(pin);
    if(keyItem) {
        keyItem->changeOnPressed(callbackOnPressed);
    }
}

void SwitchInput::replaceSwitchListener(pinid_t pin, SwitchListener* newListener) {
    auto keyItem = keys.getByKey(pin);
    if(keyItem) {
        keyItem->changeListener(newListener);
    }
}

bool SwitchInput::isSwitchPressed(pinid_t pin) {
    return keys.getByKey(pin)->isPressed();
}

void SwitchInput::pushSwitch(pinid_t pin, bool held) {
    keys.getByKey(pin)->trigger(held);
}

void SwitchInput::changeEncoderPrecision(uint8_t slot, uint16_t precision, uint16_t currentValue, bool rollover, int step) {
	if (slot < MAX_ROTARY_ENCODERS && encoder[slot] != nullptr) {
		encoder[slot]->changePrecision(precision, (int)currentValue, rollover, step);
	}
}

void SwitchInput::setEncoder(uint8_t slot, RotaryEncoder* enc) {
	if (slot < MAX_ROTARY_ENCODERS) {
		this->encoder[slot] = enc;
	}
}

bool SwitchInput::runLoop() {
	bool needAnotherGo = false;

	lastSyncStatus = ioDeviceSync(ioDevice);

	for (bsize_t i = 0; i < keys.count(); ++i) {
		// get the pins current state
		auto key = keys.itemAtIndex(i);
		uint8_t pinState = ioDeviceDigitalRead(ioDevice, key->getPin());
		if(isPullupLogic(key->isLogicInverted())) {
			pinState = !pinState;
		}
		// and pass to the key handler.
		key->checkAndTrigger(pinState);

		// we need to call into here again if we are debouncing or anything is pressed.
		needAnotherGo |= (key->isDebouncing() || key->isPressed());
	}

	return needAnotherGo;
}


/******ROTARY ENCODERS *****/


RotaryEncoder::RotaryEncoder(EncoderCallbackFn callback) : notify{} {
    this->notify.callback = callback;
	this->currentReading = 0;
	this->maximumValue = 0;
    this->flags = 0U;
    bitWrite(flags, LAST_SYNC_STATUS, 1);
    this->intent = CHANGE_VALUE;
}

RotaryEncoder::RotaryEncoder(EncoderListener* listener) : notify{} {
	this->notify.encoderListener = listener;
	this->currentReading = 0;
	this->maximumValue = 0;
    this->stepSize = 1;
    this->flags = 0U;
    bitWrite(flags, LAST_SYNC_STATUS, 1);
    bitWrite(flags, OO_LISTENER_CALLBACK, 1);
    this->intent = CHANGE_VALUE;
}

void RotaryEncoder::changePrecision(uint16_t maxValue, int currentValue, bool rolloverOnMax, int step) {
	this->maximumValue = maxValue;
	this->currentReading = currentValue;
    this->stepSize = step;
	bitWrite(flags, WRAP_AROUND_MODE, rolloverOnMax);
	intent = (maxValue == 0U && currentValue == 0) ? DIRECTION_ONLY : CHANGE_VALUE;
	runCallback((int)currentReading);
}

void RotaryEncoder::replaceCallback(EncoderCallbackFn callbackFn) {
    bitWrite(flags, OO_LISTENER_CALLBACK, false);
    this->notify.callback = callbackFn;
}

void RotaryEncoder::replaceCallbackListener(EncoderListener* listener) {
    bitWrite(flags, OO_LISTENER_CALLBACK, true);
    this->notify.encoderListener = listener;
}

void RotaryEncoder::setUserIntention(EncoderUserIntention intention) {
    intent = intention;
    if(intention == DIRECTION_ONLY) {
        maximumValue = 0;
        currentReading = 0;
    }
}

// this abs accounts for some boards where abs is a double precision function
#define safeAbs(x) ((x) < 0 ? -(x) : (x))

void RotaryEncoder::increment(int8_t incVal) {
    if(maximumValue == 0 && intent == DIRECTION_ONLY) {
		// first check if we are in direction only mode (max = 0)
		 runCallback(incVal);
         return;
	}

    bool rollover = bitRead(flags, WRAP_AROUND_MODE) != 0;
    if(incVal >= 0) {
        if(rollover) {
			currentReading = (currentReading + incVal);
			if (currentReading > maximumValue) currentReading = currentReading - maximumValue - 1;
        } else {
			currentReading = min((uint16_t)(currentReading + incVal), maximumValue);
		}
	} else if(currentReading < abs(incVal)) {
		currentReading = rollover? maximumValue - safeAbs(incVal) + 1 : 0;
	} else if(currentReading != 0) {
		currentReading += incVal;
    }
    runCallback((int)currentReading);
}

HardwareRotaryEncoder::HardwareRotaryEncoder(pinid_t pinA, pinid_t pinB, EncoderCallbackFn callback, HWAccelerationMode accelerationMode, EncoderType encoderType) : RotaryEncoder(callback) {
	this->pinA = pinA;
	this->pinB = pinB;
	this->lastChange = micros();
    this->accelerationMode = accelerationMode;
	this->encoderType = encoderType;

	// set the pin directions to input with pull ups enabled
	ioDevicePinMode(switches.getIoAbstraction(), pinA, INPUT_PULLUP);
	ioDevicePinMode(switches.getIoAbstraction(), pinB, INPUT_PULLUP);

	// read back the initial values.
    bool lastSyncOK = ioDeviceSync(switches.getIoAbstraction());
	bitWrite(flags, LAST_SYNC_STATUS, lastSyncOK);
	this->aLast = ioDeviceDigitalRead(switches.getIoAbstraction(), pinA);
	this->cleanFromB = ioDeviceDigitalRead(switches.getIoAbstraction(), pinB);

	if(!switches.isEncoderPollingEnabled()) {
		registerInterrupt(pinA);
	}
}

void checkRunLoopAndRepeat() {
	// turn off interrupts until deboucing / repeat logic is complete.
	switches.setInterruptDebouncing(true);

	// instead of running constantly, we only run when there's a need to, eg something
	// is still in a debouncing state. Otherwise we wait for an interrupt.
	// switches.runLoop returns true when it needs to run again.
	if(switches.runLoop() && switches.isInterruptDriven()) {
		taskManager.scheduleOnce(20, [] {
			checkRunLoopAndRepeat();
		});
	}
	else {
		// back to normal now - interrupt only
		switches.setInterruptDebouncing(false);
	}
}

void onSwitchesInterrupt(__attribute__((unused)) pinid_t pin) {
	if(switches.isInterruptDriven() && !switches.isInterruptDebouncing()) {
		checkRunLoopAndRepeat();
	}

  for(int i = 0; i < MAX_ROTARY_ENCODERS; ++i) {
		if(switches.encoder[i]) {
			switches.encoder[i]->encoderChanged();
		}
	}
}

int HardwareRotaryEncoder::amountFromChange(unsigned long change) {
	if(change > 250000 || maximumValue < ONE_TURN_OF_ENCODER) return stepSize;

    if(accelerationMode == HWACCEL_NONE) {
        return stepSize;
    }
    else if(accelerationMode == HWACCEL_REGULAR) {
        if(change > 120000) return stepSize + stepSize;
        else if (change > 70000) return stepSize << 2;
        else if (change > 30000) return stepSize << 3;
        else return stepSize << 4;
    }
    else { // slower, very slight acceleration..
        if(change > 100000) return stepSize + stepSize;
        else if (change > 30000) return stepSize + stepSize + stepSize;
        else return stepSize << 2;
    }

}

void HardwareRotaryEncoder::encoderChanged() {
	bool lastSyncStatus = ioDeviceSync(switches.getIoAbstraction());
    bitWrite(flags, LAST_SYNC_STATUS, lastSyncStatus);

	uint8_t a = ioDeviceDigitalRead(switches.getIoAbstraction(), pinA);
	uint8_t b = ioDeviceDigitalRead(switches.getIoAbstraction(), pinB);

	if(encoderType == QUARTER_CYCLE){
		if((a != aLast) || (b != cleanFromB)) {
			aLast = a;
			if((a != aLast) || (b != cleanFromB)) {
				cleanFromB = b;
				if((a || cleanFromB) || (a == 0 && b == 0)) {	
					unsigned long timeNow = micros();
					int amt = amountFromChange(timeNow - lastChange);
					increment((int8_t)(a != b ? -amt : amt));
					lastChange = timeNow;
				}
			}
		}		
	}
	else {
		if(a != aLast) {
			aLast = a;
			if(b != cleanFromB) {
				cleanFromB = b;
				if(a) {	
					unsigned long timeNow = micros();
					int amt = amountFromChange(timeNow - lastChange);
					increment((int8_t)(a != b ? -amt : amt));
					lastChange = timeNow;
				}
			}
		}	
	}
}

EncoderUpDownButtons::EncoderUpDownButtons(pinid_t pinUp, pinid_t pinDown, EncoderCallbackFn callback, uint8_t speed)
        : RotaryEncoder(callback), upPin(pinUp), downPin(pinDown) {
	switches.addSwitchListener(pinUp, this, speed);
	switches.addSwitchListener(pinDown, this, speed);
}

void EncoderUpDownButtons::onPressed(pinid_t pin, bool held) {
    if(pin == upPin) {
        int8_t dir = (switches.getEncoder()->getUserIntention() == SCROLL_THROUGH_ITEMS) ? -stepSize : stepSize;
        increment(dir);
    } else if(pin == downPin) {
        int8_t dir = (switches.getEncoder()->getUserIntention() == SCROLL_THROUGH_ITEMS) ? stepSize : -stepSize;
        increment(dir);
    }
}

void EncoderUpDownButtons::onReleased(pinid_t pin, bool held) {
    // ignored..
}

/******** ENCODER SETUP METHODS ***********/

void setupUpDownButtonEncoder(pinid_t pinUp, pinid_t pinDown, EncoderCallbackFn callback, int speed) {
	if (switches.getIoAbstraction() == nullptr) switches.init(internalDigitalIo(), SWITCHES_POLL_EVERYTHING, true);

	auto* enc = new EncoderUpDownButtons(pinUp, pinDown, callback, speed);
	switches.setEncoder(enc);
}

void registerInterrupt(pinid_t pin) {
	taskManager.setInterruptCallback(onSwitchesInterrupt);
	taskManager.addInterrupt(switches.getIoAbstraction(), pin, CHANGE);
}

void setupRotaryEncoderWithInterrupt(pinid_t pinA, pinid_t pinB, EncoderCallbackFn callback, HWAccelerationMode accelerationMode, EncoderType encoderType) {
	if (switches.getIoAbstraction() == nullptr) switches.init(internalDigitalIo(), SWITCHES_POLL_EVERYTHING, true);

	switches.setEncoder(new HardwareRotaryEncoder(pinA, pinB, callback, accelerationMode, encoderType));
}
