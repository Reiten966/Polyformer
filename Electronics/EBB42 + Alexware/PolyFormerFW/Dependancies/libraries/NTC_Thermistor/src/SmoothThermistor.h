/**
  SmoothThermistor - class-wrapper allows to smooth
  the temperature value of origin Thermistor instance.

  Instantiation:
  Thermistor* thermistor = new SmoothThermistor(
    THERMISTOR, SMOOTH_FACTOR
  );

  Where,
  THERMISTOR - origin Thermistor instance.
  SMOOTH_FACTOR - smoothing factor of a temperature value.

  Read temperature:
  double celsius = thermistor->readCelsius();
  double kelvin = thermistor->readKelvin();
  double fahrenheit = thermistor->readFahrenheit();

  v.2.0.0
  - created

  v.2.0.2
  - optimized smoothe(*) method;
  - added default constant for the smoothing factor;
  - added default value of constructor parameters;
  - updated documentation.

  v.2.0.3
  - replaced "define" constants with "static const"

  https://github.com/YuriiSalimov/NTC_Thermistor

  Created by Yurii Salimov, May, 2019.
  Released into the public domain.
*/
#ifndef SMOOTH_THERMISTOR_H
#define SMOOTH_THERMISTOR_H

#include "Thermistor.h"

class SmoothThermistor final : public Thermistor {

  private:
    // Minimum smoothing factor.
    static const int MIN_SMOOTHING_FACTOR = 2;

    Thermistor* origin;
    int smoothingFactor;
    double celsius = 0;
    double kelvin = 0;
    double fahrenheit = 0;

  public:
    /**
      Constructor

      @param origin - origin Thermistor instance (not NULL).
      @param factor - smoothing factor of a temperature value (default, 2)
    */
    SmoothThermistor(
      Thermistor* origin,
      int smoothingFactor = MIN_SMOOTHING_FACTOR
    );

    /**
      Destructor
      Deletes the origin Thermistor instance.
    */
    ~SmoothThermistor();

    /**
      Reads a temperature in Celsius from the thermistor.

      @return average temperature in degree Celsius
    */
    double readCelsius() override;

    /**
      Reads a temperature in Kelvin from the thermistor.

      @return smoothed temperature in degree Kelvin
    */
    double readKelvin() override;

    /**
      Reads a temperature in Fahrenheit from the thermistor.

      @return smoothed temperature in degree Fahrenheit
    */
    double readFahrenheit() override;

  private:
    /**
      Perform smoothing of the input value.

      @param input - the value to smooth
      @param data - the data for smoothing of the input value
      @return smoothed value or the input value
      if the input data is 0.
    */
    inline double smoothe(double input, double data);

    /**
      Sets the smoothing factor.
      If the input value is less than NTC_MIN_SMOOTHING_FACTOR,
      then sets NTC_MIN_SMOOTHING_FACTOR.

      @param smoothingFactor - new smoothing factor
    */
    inline void setSmoothingFactor(int smoothingFactor);
};

#endif
