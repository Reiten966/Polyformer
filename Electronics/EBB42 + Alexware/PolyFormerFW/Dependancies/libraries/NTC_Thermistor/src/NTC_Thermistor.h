/**
  NTC_Thermistor - class describes a set of methods
  for working with a NTC thermistor and reading
  a temperature in Celsius, Fahrenheit and Kelvin.

  Instantiation, for example, to NTC 3950 thermistor:
  Thermistor* thermistor = new NTC_Thermistor(A1, 8000, 100000, 25, 3950);

  Read temperature:
    double celsius = thermistor->readCelsius();
    double kelvin = thermistor->readKelvin();
    double fahrenheit = thermistor->readFahrenheit();

  v.1.1.2:
  - updated conversion from celsius to fahrenheit;
  - added conversion from kelvin to fahrenheit;
  - optimized calls of private methods.

  v.1.1.3:
  - fixed bug in setReadingsNumber() method.

  v.1.1.4:
  - removed deprecated init() method;
  - replaced pinMode from INPUT_PULLUP to INPUT.

  v.2.0.0
  - implemented Thermistor interface;
  - removed methods for averaging result.

  v.2.0.2
  - optimized resistanceToKelvins(*) method;
  - optimized constructor;
  - updated documentation.

  https://github.com/YuriiSalimov/NTC_Thermistor

  Created by Yurii Salimov, February, 2018.
  Released into the public domain.
*/
#ifndef NTC_THERMISTOR_H
#define NTC_THERMISTOR_H

#include "Thermistor.h"

class NTC_Thermistor : public Thermistor {

  private:
    // Default analog resolution for Arduino board
    static const int DEFAULT_ADC_RESOLUTION = 1023;

    int pin; // an analog port.
    double referenceResistance;
    double nominalResistance;
    double nominalTemperature; // in Celsius.
    double bValue;
    int adcResolution;

  public:
    /**
      Constructor

      @param pin - an analog port number to be attached to the thermistor
      @param referenceResistance - reference resistance
      @param nominalResistance - nominal resistance at a nominal temperature
      @param nominalTemperature - nominal temperature in Celsius
      @param bValue - b-value of a thermistor
      @param adcResolution - ADC resolution (default 1023, for Arduion)
    */
    NTC_Thermistor(
      int pin,
      double referenceResistance,
      double nominalResistance,
      double nominalTemperatureCelsius,
      double bValue,
      int adcResolution = DEFAULT_ADC_RESOLUTION
    );

    /**
      Reads a temperature in Celsius from the thermistor.

      @return temperature in degree Celsius
    */
    double readCelsius() override;

    /**
      Reads a temperature in Kelvin from the thermistor.

      @return temperature in degree Kelvin
    */
    double readKelvin() override;

    /**
      Reads a temperature in Fahrenheit from the thermistor.

      @return temperature in degree Fahrenheit
    */
    double readFahrenheit() override;

  private:
    /**
      Resistance to Kelvin conversion:
      1/K = 1/K0 + ln(R/R0)/B;
      K = 1 / (1/K0 + ln(R/R0)/B);
      Where
      K0 - nominal temperature,
      R0 - nominal resistance at a nominal temperature,
      R - the input resistance,
      B - b-value of a thermistor.

      @param resistance - resistance value to convert
      @return temperature in degree Kelvin
    */
    inline double resistanceToKelvins(double resistance);

    /**
      Calculates a resistance of the thermistor:
      Converts a value of the thermistor sensor into a resistance.
      R = R0 / (ADC / V - 1);
      Where
      R0 - nominal resistance at a nominal temperature,
      ADC - analog port resolution (1023, for Arduino)
      V - current voltage (analog port value).

      @return resistance of the thermistor sensor.
    */
    inline double readResistance();

    /**
      Reads a voltage from the thermistor analog port.

      @return thermistor voltage in analog range (0...1023, for Arduino).
    */
    inline double readVoltage();

    /**
      Celsius to Kelvin conversion:
      K = C + 273.15

      @param celsius - temperature in degree Celsius to convert
      @return temperature in degree Kelvin
    */
    inline double celsiusToKelvins(double celsius);

    /**
      Kelvin to Celsius conversion:
      C = K - 273.15

      @param kelvins - temperature in degree Kelvin to convert
      @return temperature in degree Celsius
    */
    inline double kelvinsToCelsius(double kelvins);

    /**
      Celsius to Fahrenheit conversion:
      F = C * 1.8 + 32

      @param celsius - temperature in degree Celsius to convert
      @return temperature in degree Fahrenheit
    */
    inline double celsiusToFahrenheit(double celsius);

    /**
      Kelvin to Fahrenheit conversion:
      F = (K - 273.15) * 1.8 + 32

      @param kelvins - temperature in degree Kelvin to convert
      @return temperature in degree Fahrenheit
    */
    inline double kelvinsToFahrenheit(double kelvins);
};

#endif
