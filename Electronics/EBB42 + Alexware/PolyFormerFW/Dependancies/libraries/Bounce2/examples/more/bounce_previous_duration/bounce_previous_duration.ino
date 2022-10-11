
/*
  DESCRIPTION
  ====================
  When the debounced input goes from LOW to HIGH, the LED will turn on for the same time
  the debounced input was held LOW.

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

// SET A VARIABLE TO STORE THE INTERVAL FOR HOW LONG TO KEEP THE LED HIGH
unsigned long ledHighInterval;

// SET A VARIABLE TO STORE THE START TIME WHEN THE LED WAS TURNED HIGH
unsigned long ledHighLastTime;

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
  digitalWrite(LED_PIN, LOW);

}

void loop() {
  // Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
  bounce.update();

  // <Bounce>.rose() RETURNS true IF THE STATE JUST WENT FROM LOW TO HIGH
  if ( bounce.rose() ) {
    digitalWrite(LED_PIN, HIGH); // TURN ON THE LED
    ledHighLastTime = millis();
    ledHighInterval = bounce.previousDuration();
  }

  // IF WE HAVE TO BLINK THE LED
  if ( millis() - ledHighLastTime > ledHighInterval ) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }

}
