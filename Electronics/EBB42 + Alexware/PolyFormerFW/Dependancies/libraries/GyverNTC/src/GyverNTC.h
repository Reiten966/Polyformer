/*
    Библиотека для работы с NTC термисторами по закону Стейнхарта-Харта
    Документация: 
    GitHub: https://github.com/GyverLibs/GyverNTC
    Возможности:
    - Установка параметров термистора
    - Получение температуры
    - Встроенное усреднение
    
    AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License

    Версии:
    v1.0 - релиз
    v1.1 - небольшая оптимизация, повышение точности
    v1.2 - оптимизация, поддержка 100к термисторов
*/

#ifndef _GyverNTC_h
#define _GyverNTC_h
#include <Arduino.h>
#define _T_SAMPLE_AVERAGE 20   // количество чтений для усреднения

class GyverNTC {
public:
    GyverNTC(uint8_t pin, uint32_t resistance, uint16_t beta, uint8_t tempBase = 25, uint32_t base = 10000) :
    _pin(pin), _beta(beta), _tempBase(tempBase), _baseDivRes((float)base / resistance) {}
    
    // прочитать температуру с пина
    float getTemp() {
        return computeTemp(analogRead(_pin));
    }
    
    // прочитать усреднённую температуру с пина
    float getTempAverage() {
        uint16_t aver = 0;
        for (uint8_t i = 0; i < _T_SAMPLE_AVERAGE; i++) aver += analogRead(_pin);
        return computeTemp((float)aver / _T_SAMPLE_AVERAGE);
    }
    
    // получить температуру из сигнала АЦП (10 бит, float)
    float computeTemp(float analog) {
        analog = _baseDivRes / (1023.0f / analog - 1.0);
        analog = (log(analog) / _beta) + 1.0 / (_tempBase + 273.15);
        return (1.0 / analog - 273.15);
    }
    
private:    
    const uint8_t _pin = 0;
    const uint16_t _beta = 0;
    const uint8_t _tempBase = 25;
    const float _baseDivRes;
};
#endif