
/* 
 DESCRIPTION
 ====================
 Simple example of the Bounce library that switches the debug LED when 
 either of 2 buttons are pressed.
 */
 
// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

#define BUTTON_PIN_1 2
#define BUTTON_PIN_2 3


#define LED_PIN 13

// Instantiate a Bounce object
Bounce debouncer1 = Bounce(); 

// Instantiate another Bounce object
Bounce debouncer2 = Bounce(); 

void setup() {

  // Setup the first button with an internal pull-up :
  pinMode(BUTTON_PIN_1,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncer1.attach(BUTTON_PIN_1);
  debouncer1.interval(5); // interval in ms
  
   // Setup the second button with an internal pull-up :
  pinMode(BUTTON_PIN_2,INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncer2.attach(BUTTON_PIN_2);
  debouncer2.interval(5); // interval in ms


  //Setup the LED :
  pinMode(LED_PIN,OUTPUT);

}

void loop() {
  // Update the Bounce instances :
  debouncer1.update();
  debouncer2.update();

  // Get the updated value :
  int value1 = debouncer1.read();
  int value2 = debouncer2.read();

  // Turn on the LED if either button is pressed :
  if ( value1 == LOW || value2 == LOW ) {
    digitalWrite(LED_PIN, HIGH );
  } 
  else {
    digitalWrite(LED_PIN, LOW );
  }

}


