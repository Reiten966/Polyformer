/*
  NTC Thermistor

  Reads a temperature from the NTC 3950 thermistor and displays
  it in the default Serial.

  The Arduino Uno or any other Arduino board that uses Atmega328 as
  the Microcontroller has ADC resolution of 10 bits. Hence the values on each
  analog channel can vary from 0 to 1023.
  The STM32 board has ADC resolution of 12 bits. Hence the values on each analog
  channel can vary from 0 to 4095.

  If you use the STM32 board, you must calibrate your thermistor,
  since the analogRead(*) function return 0..4095
  versus 0..1023 for Arduino.

  ---------------------------
  |       analogRead()      |
  ---------------------------
  |  Arduion   |   STM32    |
  |-------------------------|
  |  0...1023  |  0...4095  |
  ---------------------------

  To calibrate use next constructor:
  Thermistor* thermistor = new NTC_Thermistor(
    ..., ANALOG_RESOLUTION
  );

  Where,
  ANALOG_RESOLUTION - board ADC resolution (default, 1023).

  https://github.com/YuriiSalimov/NTC_Thermistor

  Created by Yurii Salimov, May, 2019.
  Released into the public domain.
*/
#include <Thermistor.h>
#include <NTC_Thermistor.h>

#define SENSOR_PIN             PA6
#define REFERENCE_RESISTANCE   8000
#define NOMINAL_RESISTANCE     100000
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3950
#define STM32_ANALOG_RESOLUTION 4095

Thermistor* thermistor;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);

  thermistor = new NTC_Thermistor(
    SENSOR_PIN,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE,
    STM32_ANALOG_RESOLUTION // <- for a thermistor calibration
  );
}

// the loop function runs over and over again forever
void loop() {
  // Reads temperature
  const double celsius = thermistor->readCelsius();
  const double kelvin = thermistor->readKelvin();
  const double fahrenheit = thermistor->readFahrenheit();

  // Output of information
  Serial.print("Temperature: ");
  Serial.print(celsius);
  Serial.print(" C, ");
  Serial.print(kelvin);
  Serial.print(" K, ");
  Serial.print(fahrenheit);
  Serial.println(" F");

  delay(500); // optionally, only to delay the output of information in the example.
}
