/**
  Created by Yurii Salimov, May, 2019.
  Released into the public domain.
*/
#include "AverageThermistor.h"

AverageThermistor::AverageThermistor(
  Thermistor* origin,
  const int readingsNumber,
  const int delayTimeInMillis
) {
  this->origin = origin;
  this->readingsNumber = validate(readingsNumber, DEFAULT_READINGS_NUMBER);
  this->delayTime = validate(delayTimeInMillis, DEFAULT_DELAY_TIME);
}

AverageThermistor::~AverageThermistor() {
  delete this->origin;
}

double AverageThermistor::readCelsius() {
  return average(&Thermistor::readCelsius);
}

double AverageThermistor::readKelvin() {
  return average(&Thermistor::readKelvin);
}

double AverageThermistor::readFahrenheit() {
  return average(&Thermistor::readFahrenheit);
}

inline double AverageThermistor::average(double (Thermistor::*read)()) {
  double sum = 0;
  for (int i = 0; i < this->readingsNumber; ++i) {
    sum += (this->origin->*read)();
    sleep();
  }
  return (sum / this->readingsNumber);
}

inline void AverageThermistor::sleep() {
  delay(this->delayTime);
}

template <typename A, typename B>
inline A AverageThermistor::validate(A data, B alternative) {
  return (data > 0) ? data : alternative;
}
