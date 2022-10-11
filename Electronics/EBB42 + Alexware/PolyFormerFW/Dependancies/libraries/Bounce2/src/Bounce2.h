/*
  The MIT License (MIT)

  Copyright (c) 2013 thomasfredericks

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *
  Main code by Thomas O Fredericks (tof@t-o-f.info)
  Previous contributions by Eric Lowry, Jim Schimpf and Tom Harkaway
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef Bounce2_h
#define Bounce2_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Uncomment the following line for "LOCK-OUT" debounce method
//#define BOUNCE_LOCK_OUT

// Uncomment the following line for "BOUNCE_WITH_PROMPT_DETECTION" debounce method
//#define BOUNCE_WITH_PROMPT_DETECTION

#include <inttypes.h>

/**
    @example bounce_basic.ino
    Basic example of the Bounce class.
*/

/**
    @example button_basic.ino
    Basic example of the Button class.
*/



/**
     @brief  The Debouce class. Just the deboucing code separated from all harware.
*/
class Debouncer
{
 // Note : this is private as it migh change in the futur
private:
  static const uint8_t DEBOUNCED_STATE = 0b00000001; // Final returned calculated debounced state
  static const uint8_t UNSTABLE_STATE  = 0b00000010; // Actual last state value behind the scene
  static const uint8_t CHANGED_STATE   = 0b00000100; // The DEBOUNCED_STATE has changed since last update()

// Note : this is private as it migh change in the futur
private:
  inline void changeState();
  inline void setStateFlag(const uint8_t flag)       {state |= flag;}
  inline void unsetStateFlag(const uint8_t flag)     {state &= ~flag;}
  inline void toggleStateFlag(const uint8_t flag)    {state ^= flag;}
  inline bool getStateFlag(const uint8_t flag) const {return((state & flag) != 0);}

public:
	/*!
    @brief  Create an instance of the Debounce class.

    @endcode
*/
	Debouncer();

	    /**
    @brief  Sets the debounce interval in milliseconds.
            
    @param    interval_millis
    		The interval time in milliseconds.
     
     */
	void interval(uint16_t interval_millis);

	/*!
    @brief   Updates the pin's state. 

    Because Bounce does not use interrupts, you have to "update" the object before reading its value and it has to be done as often as possible (that means to include it in your loop()). Only call update() once per loop().

    @return True if the pin changed state.
*/

	bool update();

    /**
     @brief Returns the pin's state (HIGH or LOW).

     @return HIGH or LOW.
     */
	bool read() const;

    /**
    @brief Returns true if pin signal transitions from high to low.
    */
	bool fell() const;

    /**
    @brief Returns true if pin signal transitions from low to high.
    */
	bool rose() const;



public:
    /**
     @brief Returns true if the state changed on last update.

     @return True if the state changed on last update. Otherwise, returns false.
*/
  bool changed( ) const { return getStateFlag(CHANGED_STATE); }
  
  
  

      /**
     @brief Returns the duration in milliseconds of the current state. 

     Is reset to 0 once the pin rises ( rose() ) or falls ( fell() ).
    
      @return The duration in milliseconds (unsigned long) of the current state.
     */

  unsigned long currentDuration() const;


  /**
     @brief Returns the duration in milliseconds of the previous state. 

     Takes the values of duration() once the pin changes state.
    
      @return The duration in milliseconds (unsigned long) of the previous state. 
     */
  unsigned long previousDuration() const;
  
      /**
     @brief DEPRECATED (i.e. do not use). Renamed currentDuration().
	 
	 This member function is DEPRECATED will be removed next major version. DO NOT USE IT. USE currentDuration() INSTEAD.
	 It was renamed to avoid confusion with previousDuration().
	 
	  @return The duration in milliseconds (unsigned long) of the current state.
     */
    [[deprecated]]
    unsigned long duration() {
		return currentDuration();
	};

protected:
  void begin();
  virtual bool readCurrentState() =0;
  unsigned long previous_millis;
  uint16_t interval_millis;
  uint8_t state;
  unsigned long stateChangeLastTime;
  unsigned long durationOfPreviousState;

};


/**
@brief The Debouncer:Bounce class. Links the Deboucing class to a hardware pin.  This class is odly named, but it will be kept that so it stays compatible with previous code.
     
     */
class Bounce : public Debouncer
{


public:

/*!
    @brief  Create an instance of the Bounce class.

    @code

    // Create an instance of the Bounce class.
    Bounce() button;

    @endcode
*/
	Bounce();


/*!
    @brief  Attach to a pin and sets that pin's mode (INPUT, INPUT_PULLUP or OUTPUT).
            
    @param    pin
              The pin that is to be debounced.
    @param    mode
              A valid Arduino pin mode (INPUT, INPUT_PULLUP or OUTPUT).
*/
	void attach(int pin, int mode);

    /**
    Attach to a pin for advanced users. Only attach the pin this way once you have previously set it up. Otherwise use attach(int pin, int mode).
    */
	void attach(int pin);

  Bounce(uint8_t pin, unsigned long interval_millis ) : Bounce() {
    attach(pin);
    interval(interval_millis);
  }
 

 /**
  @brief Return pin that this Bounce is attached to
  
  @return integer identifier of the coupled pin
  */
  inline int getPin() const {
      return this->pin;
  };

  ////////////////
  // Deprecated //
  ////////////////

     /**
    @brief Deprecated (i.e. do not use). Included for partial compatibility for programs written with Bounce version 1
    */
	[[deprecated]]
	bool risingEdge() const { return rose(); }
     /**
    @brief Deprecated (i.e. do not use). Included for partial compatibility for programs written with Bounce version 1
    */
	[[deprecated]]
	bool fallingEdge() const { return fell(); }
     /**
    @brief Deprecated (i.e. do not use). Included for partial compatibility for programs written with Bounce version 1
    */


protected:


	uint8_t pin;

	virtual bool readCurrentState() { return digitalRead(pin); }
	virtual void setPinMode(int pin, int mode) {
#if defined(ARDUINO_ARCH_STM32F1)
		pinMode(pin, (WiringPinMode)mode);
#else
		pinMode(pin, mode);
#endif
	}



};

/**
     @brief The Debouncer:Bounce:Button class. The Button class matches an electrical state to a physical action.
     */
namespace Bounce2 {
   // code declarations

class Button : public Bounce{
protected:
    bool stateForPressed = 1; // 
  public:
	/*!
    @brief  Create an instance of the Button class. By default, the pressed state is matched to a HIGH electrical level.

    @code

    // Create an instance of the Button class.
    Button() button;

    @endcode
*/
   Button(){ }

    /*!
    @brief Set the electrical state (HIGH/LOW) that corresponds to a physical press. By default, the pressed state is matched to a HIGH electrical level.
            
    @param    state
              The electrical state (HIGH/LOW) that corresponds to a physical press.

*/
   void setPressedState(bool state){
    stateForPressed = state;
  }

  /*!
  @brief Get the electrical state (HIGH/LOW) that corresponds to a physical press. 
  */
  inline bool getPressedState() const {
    return stateForPressed;
  };

  /*!
  @brief Returns true if the button is currently physically pressed.
  */
  inline bool isPressed() const {
    return read() == getPressedState();
  };

    /*!
    @brief Returns true if the button was physically pressed          
*/
  inline bool pressed() const {
    return changed() && isPressed();
  };

        /*!
    @brief Returns true if the button was physically released          
*/
  inline bool released() const {
    return  changed() && !isPressed();
  };

};
};

#endif
