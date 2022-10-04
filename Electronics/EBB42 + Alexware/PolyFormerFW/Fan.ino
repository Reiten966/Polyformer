#include <GyverNTC.h>
GyverNTC therm(thermistorPin, NOMINAL_RESISTANCE, B_VALUE, NOMINAL_TEMPERATURE, REFERENCE_RESISTANCE);  // pin, R thermistor, B thermistor, base temperature, R resistor

bool fan1 = false; //Keep track of the fan state

//#define ANALOG_RESOLUTION 65535 //16 Bit resolution, this can be higher than your MCU supports, and will adjust accordingly
#define meltzoneFanTemp 40 //Temperature the fan will kick in at
double targetTemperatureC = 0;
double celsius;


void thermistorSetup() {
};

void fanSetup() {
  analogReadResolution(10); //Set Resolution of ADC to 12 bit, this will pad any lower resolution values, giving us future flexability
  pinMode(meltzoneFanPin, OUTPUT);
};

void fanLoop() {
  if (error == 0) {
    //    if (therm.getTemp() >= meltzoneFanTemp || targetTemperatureC >= meltzoneFanTemp )
    //    {
    analogWrite(meltzoneFanPin, menuFan.getAsFloatingPointValue() * 2.55);
    fan1 = true;
    //    } else {
    //      digitalWrite(meltzoneFanPin, LOW);
    //      fan1 = false;
    //    }
  } else {
    digitalWrite(meltzoneFanPin, HIGH); //Error detected, run fan at full power.
    fan1 = true;
  }
};
