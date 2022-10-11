
/*
  DESCRIPTION
  ====================
  Simple example of the Bounce library that switches the state of a LED
  when the debounced input goes from LOW to HIGH. 

  Also if the debounced input was previously held LOW for more than 1000 milliseconds,
  the LED will blink.

  Set BOUNCE_PIN to the pin attached to the input (a button for example).
  Set LED_PIN to the pin attached to a LED.

*/

// WE WILL attach() THE Bounce INSTANCE TO THE FOLLOWING PIN IN setup()
#define BOUNCE_PIN 2

// DEFINE THE PIN FOR THE LED :
// 1) SOME BOARDS HAVE A DEFAULT LED (LED_BUILTIN)
#define LED_PIN LED_BUILTIN
// 2) OTHERWISE SET YOUR OWN PIN
// #define LED_PIN 13

// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>


// INSTANTIATE A Bounce OBJECT.
Bounce bounce = Bounce();
 
// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;

// SET A VARIABLE TO STORE THE BLINKING STATE
bool blinkLed = false;
// SET A VARIABLE TO STORE THE LAST TIME THE LED BLINKED
unsigned long blinkLedLastTime;

void setup() {

  // BOUNCE SETUP

  // SELECT ONE OF THE FOLLOWING :
  // 1) IF YOUR INPUT HAS AN INTERNAL PULL-UP
  bounce.attach( BOUNCE_PIN ,  INPUT_PULLUP ); // USE INTERNAL PULL-UP
  // 2) IF YOUR INPUT USES AN EXTERNAL PULL-UP
  //bounce.attach( BOUNCE_PIN, INPUT ); // USE EXTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  bounce.interval(5); // interval in ms

  // LED SETUP
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);

}

void loop() {
  // Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
  bounce.update();

  // <Bounce>.changed() RETURNS true IF THE STATE CHANGED (FROM HIGH TO LOW OR LOW TO HIGH)
  if ( bounce.changed() ) {
    // THE STATE OF THE INPUT CHANGED
    // GET THE STATE
    int deboucedInput = bounce.read();
    // IF THE CHANGED VALUE IS HIGH
    if ( deboucedInput == HIGH ) {
      ledState = !ledState; // SET ledState TO THE OPPOSITE OF ledState
      digitalWrite(LED_PIN,ledState); // WRITE THE NEW ledState
      // IF THE DURATION OF THE PREVIOUS STATE (LOW IN THIS CASE) WAS HELD LONGER THAN 1000 MILLISECONDS:
      if ( bounce.previousDuration() > 1000 ) blinkLed = true;
      else blinkLed = false;
    } 
  }

  // IF WE HAVE TO BLINK THE LED
  if ( blinkLed ) {
    if ( millis() - blinkLedLastTime > 500 ) {
      blinkLedLastTime = millis();
      ledState = !ledState; // SET ledState TO THE OPPOSITE OF ledState
      digitalWrite(LED_PIN,ledState); // WRITE THE NEW ledState
    }
  }
    
}
