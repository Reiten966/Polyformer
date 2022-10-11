# BOUNCE 2

Debouncing library for Arduino and Wiring by Thomas Ouellet Fredericks with many contributions from the community : https://github.com/thomasfredericks/Bounce2/graphs/contributors

Basically, the mechanical part of buttons and switches vibrate slightly when closed or opened causing multiple undesired false states (similar to noise). This library filters out these undesired state changes. More about debouncing: 
* John Errington's Experiments with an Arduino : [Using digital inputs: Switch bounce and solutions to it](http://www.skillbank.co.uk/arduino/switchbounce.htm)
* Wikipedia article : http://en.wikipedia.org/wiki/Debounce#Contact_bounce

See the bottom of this page for a basic usage example and the "examples" folder for more.

The library is composed of three classes:
* Debouncer : The code that does the actual debouncing. Only advanced users should play with this class.
* Bounce : This is the general use library. It links the Debouncer to a hardware pin on your board.
* Button : A special version of Bounce for buttons that are pressed.

# INSTALLATION & DOWNLOAD

Install through your software's Library Manager or download the latest version [here](https://github.com/thomasfredericks/Bounce2/archive/master.zip) and put the "Bounce2" folder in your "libraries" folder. 

Please note that the original version of this library (Bounce 1) is included in the "extras" folder of the download but not supported anymore.

## BASIC USE

### INSTANTIATE

```cpp
#include <Bounce2.h>
Bounce b = Bounce(); // Instantiate a Bounce object
```

### SETUP

```cpp
b.attach ( <PIN> , <PIN MODE> );
b.interval( <INTERVAL IN MS> );
```
### LOOP

```cpp
b.update();
if ( b.changed() ) { 
  // THE STATE OF THE INPUT CHANGED
  int deboucedValue = b.read();
  // DO SOMETHING WITH THE VALUE
}
```


## BOUNCE EXAMPLE

```cpp
// This example toggles the debug LED (pin 13) on or off when a button on pin 2 is pressed.

// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

#define BUTTON_PIN 2
#define LED_PIN 13

int ledState = LOW;


Bounce b = Bounce(); // Instantiate a Bounce object

void setup() {
  
  b.attach(BUTTON_PIN,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  b.interval(25); // Use a debounce interval of 25 milliseconds
  
  
  pinMode(LED_PIN,OUTPUT); // Setup the LED
  digitalWrite(LED_PIN,ledState); // Turn off the LED
 
}

void loop() {

   b.update(); // Update the Bounce instance
   
   if ( b.fell() ) {  // Call code if button transitions from HIGH to LOW
     ledState = !ledState; // Toggle LED state
     digitalWrite(LED_PIN,ledState); // Apply new LED state
   }
}
```

## BUTTON EXAMPLE

```cpp
/* 
 DESCRIPTION
 ====================
 This is an example of the Bounce2::Button class. 
 When the user presses a physical button, it toggles a LED on or off.
 The Button class matches an electrical state to a physical action. 
 Use .setPressedState(LOW or HIGH) to set the detection state for when the button is pressed.

 INSTRUCTIONS
 ====================
 Set BUTTON_PIN to the pin attached to the button.
 Set LED_PIN to the pin attached to a LED.
 
 */
 
// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

// INSTANTIATE A Button OBJECT
Bounce2::Button button = Bounce2::Button();

// WE WILL attach() THE BUTTON TO THE FOLLOWING PIN IN setup()
#define BUTTON_PIN 39 

// DEFINE THE PIN FOR THE LED :
// 1) SOME BOARDS HAVE A DEFAULT LED (LED_BUILTIN)
//#define LED_PIN LED_BUILTIN
// 2) OTHERWISE SET YOUR OWN PIN
#define LED_PIN 13

// SET A VARIABLE TO STORE THE LED STATE
bool ledState = LOW;

void setup() {

  // BUTTON SETUP 
  
  // SELECT ONE OF THE FOLLOWING :
  // 1) IF YOUR BUTTON HAS AN INTERNAL PULL-UP
  // button.attach( BUTTON_PIN ,  INPUT_PULLUP ); // USE INTERNAL PULL-UP
  // 2) IF YOUR BUTTON USES AN EXTERNAL PULL-UP
  button.attach( BUTTON_PIN, INPUT ); // USE EXTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  button.interval(5); 

  // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
  button.setPressedState(LOW); 
  
  // LED SETUP
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,ledState);

}

void loop() {
  // UPDATE THE BUTTON
  // YOU MUST CALL THIS EVERY LOOP
  button.update();

  if ( button.pressed() ) {
    
    // TOGGLE THE LED STATE : 
    ledState = !ledState; // SET ledState TO THE OPPOSITE OF ledState
    digitalWrite(LED_PIN,ledState);

  }
}
```


# DOCUMENTATION

The complete class documentation can be found in the "docs" folder or [online here](http://thomasfredericks.github.io/Bounce2/).

## GITHUB PAGE (SOURCE CODE)

https://github.com/thomasfredericks/Bounce2

# HAVE A QUESTION?

Please post your questions [here](http://forum.arduino.cc/index.php?topic=266132.0).



# ALTERNATE DEBOUNCE ALGORITHMS FOR ADVANCED USERS AND SPECIFIC CASES


## STABLE INTERVAL

By default, the Bounce library uses a stable interval to process the debouncing. This is simpler to understand and can cancel unwanted noise.

![](https://raw.github.com/thomasfredericks/Bounce-Arduino-Wiring/master/extras/BouncySwitch_stable.png)

## LOCK-OUT INTERVAL

By defining "#define BOUNCE_LOCK_OUT" in "Bounce.h" (or in your code before including "Bounce.h") you can activate an alternative debouncing method. This method is a lot more responsive, but does not cancel noise.

```
#define BOUNCE_LOCK_OUT
```

![](https://raw.github.com/thomasfredericks/Bounce-Arduino-Wiring/master/extras/BouncySwitch_lockout.png)

## WITH PROMPT DETECTION

By defining "#define BOUNCE_WITH_PROMPT_DETECTION" in "Bounce.h" (or in your code before including "Bounce.h") you can activate an alternative debouncing method. Button state changes are available immediately so long as the previous state has been stable for the timeout period. Otherwise the state will be updated as soon as  the timeout period allows.

* Able to report acurate switch time normally with no delay.
* Use when accurate switch transition timing is important.

```
#define BOUNCE_WITH_PROMPT_DETECTION
```


