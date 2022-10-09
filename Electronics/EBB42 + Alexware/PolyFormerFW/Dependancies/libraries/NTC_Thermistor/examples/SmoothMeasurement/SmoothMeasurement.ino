/*
  Smooth NTC Thermistor

  Reads a temperature from the NTC 3950 thermistor,
  smooth and displays it in the default Serial.

  https://github.com/YuriiSalimov/NTC_Thermistor

  Created by Yurii Salimov, May, 2019.
  Released into the public domain.
*/
#include <Thermistor.h>
#include <NTC_Thermistor.h>
#include <SmoothThermistor.h>

#define SENSOR_PIN             A1
#define REFERENCE_RESISTANCE   8000
#define NOMINAL_RESISTANCE     100000
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3950

/**
  Smoothing factor of a temperature value.
*/
#define SMOOTHING_FACTOR 5

Thermistor* thermistor = NULL;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);

  Thermistor* originThermistor = new NTC_Thermistor(
    SENSOR_PIN,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE
  );
  thermistor = new SmoothThermistor(originThermistor, SMOOTHING_FACTOR);

  /* OR
    thermistor = new SmoothThermistor(
      new NTC_Thermistor(
        SENSOR_PIN,
        REFERENCE_RESISTANCE,
        NOMINAL_RESISTANCE,
        NOMINAL_TEMPERATURE,
        B_VALUE
      ),
      SMOOTHING_FACTOR
    );
  */
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
