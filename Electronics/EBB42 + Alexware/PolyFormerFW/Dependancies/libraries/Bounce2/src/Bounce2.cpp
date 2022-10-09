// Please read Bounce2.h for information about the liscence and authors


#include "Bounce2.h"

//////////////
// DEBOUNCE //
//////////////

Debouncer::Debouncer():previous_millis(0)
    , interval_millis(10)
    , state(0) {}

void Debouncer::interval(uint16_t interval_millis)
{
    this->interval_millis = interval_millis;
}

void Debouncer::begin() {
	 state = 0;
    if (readCurrentState()) {
        setStateFlag(DEBOUNCED_STATE | UNSTABLE_STATE);
    }

	#ifdef BOUNCE_LOCK_OUT
    previous_millis = 0;
#else
    previous_millis = millis();
#endif
}

bool Debouncer::update()
{

    unsetStateFlag(CHANGED_STATE);
#ifdef BOUNCE_LOCK_OUT
    
    // Ignore everything if we are locked out
    if (millis() - previous_millis >= interval_millis) {
        bool currentState = readCurrentState();
        if ( currentState != getStateFlag(DEBOUNCED_STATE) ) {
            previous_millis = millis();
            changeState();
        }
    }
    

#elif defined BOUNCE_WITH_PROMPT_DETECTION
    // Read the state of the switch port into a temporary variable.
    bool readState = readCurrentState();


    if ( readState != getStateFlag(DEBOUNCED_STATE) ) {
      // We have seen a change from the current button state.

      if ( millis() - previous_millis >= interval_millis ) {
	// We have passed the time threshold, so a new change of state is allowed.
	// set the STATE_CHANGED flag and the new DEBOUNCED_STATE.
	// This will be prompt as long as there has been greater than interval_misllis ms since last change of input.
	// Otherwise debounced state will not change again until bouncing is stable for the timeout period.
		 changeState();
      }
    }

    // If the readState is different from previous readState, reset the debounce timer - as input is still unstable
    // and we want to prevent new button state changes until the previous one has remained stable for the timeout.
    if ( readState != getStateFlag(UNSTABLE_STATE) ) {
	// Update Unstable Bit to macth readState
        toggleStateFlag(UNSTABLE_STATE);
        previous_millis = millis();
    }
    
    
#else
    // Read the state of the switch in a temporary variable.
    bool currentState = readCurrentState();
    

    // If the reading is different from last reading, reset the debounce counter
    if ( currentState != getStateFlag(UNSTABLE_STATE) ) {
        previous_millis = millis();
         toggleStateFlag(UNSTABLE_STATE);
    } else
        if ( millis() - previous_millis >= interval_millis ) {
            // We have passed the threshold time, so the input is now stable
            // If it is different from last state, set the STATE_CHANGED flag
            if (currentState != getStateFlag(DEBOUNCED_STATE) ) {
                previous_millis = millis();
                 

                 changeState();
            }
        }

    
#endif

		return  changed(); 

}

// WIP HELD
unsigned long Debouncer::previousDuration() const {
	return durationOfPreviousState;
}

unsigned long Debouncer::currentDuration() const {
	return (millis() - stateChangeLastTime);
}

inline void Debouncer::changeState() {
	toggleStateFlag(DEBOUNCED_STATE);
	setStateFlag(CHANGED_STATE) ;
	durationOfPreviousState = millis() - stateChangeLastTime;
	stateChangeLastTime = millis();
}

bool Debouncer::read() const
{
    return  getStateFlag(DEBOUNCED_STATE);
}

bool Debouncer::rose() const
{
    return getStateFlag(DEBOUNCED_STATE) && getStateFlag(CHANGED_STATE);
}

bool Debouncer::fell() const
{
    return  !getStateFlag(DEBOUNCED_STATE) && getStateFlag(CHANGED_STATE);
}

////////////
// BOUNCE //
////////////


Bounce::Bounce()
    : pin(0)
{}

void Bounce::attach(int pin) {
    this->pin = pin;
    
    // SET INITIAL STATE
    begin();
}

void Bounce::attach(int pin, int mode){
    setPinMode(pin, mode);
    this->attach(pin);
}



