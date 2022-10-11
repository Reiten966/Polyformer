
// This example toggles the debug LED (pin 13) on or off
// when a button on pin 2 is pressed.

// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

#define BUTTON_PIN 2
#define LED_PIN 13

int ledState = LOW;


Bounce debouncer = Bounce(); // Instantiate a Bounce object

void setup() {
  
  debouncer.attach(BUTTON_PIN,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer.interval(25); // Use a debounce interval of 25 milliseconds
  
  
  pinMode(LED_PIN,OUTPUT); // Setup the LED
  digitalWrite(LED_PIN,ledState);
 
}

void loop() {

   debouncer.update(); // Update the Bounce instance
   
   if ( debouncer.fell() ) {  // Call code if button transitions from HIGH to LOW
     ledState = !ledState; // Toggle LED state
     digitalWrite(LED_PIN,ledState); // Apply new LED state
   }
}

