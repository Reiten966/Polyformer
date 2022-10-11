
/*
  DESCRIPTION
  ====================
  Simple example of the Bounce library that switches a LED when
  a state change (from HIGH to LOW) is triggered (for example when a button is pressed).

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


// INSTANTIATE A Bounce OBJECT
Bounce bounce = Bounce();

// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;

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
    // IF THE CHANGED VALUE IS LOW
    if ( deboucedInput == LOW ) {
      ledState = !ledState; // SET ledState TO THE OPPOSITE OF ledState
      digitalWrite(LED_PIN,ledState); // WRITE THE NEW ledState
    }
  }



}
