[![Foo](https://img.shields.io/badge/Version-1.2-brightgreen.svg?style=flat-square)](#versions)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD$%E2%82%AC%20%D0%9D%D0%B0%20%D0%BF%D0%B8%D0%B2%D0%BE-%D1%81%20%D1%80%D1%8B%D0%B1%D0%BA%D0%BE%D0%B9-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)

# GyverNTC
Библиотека для работы с NTC термисторами по закону Стейнхарта-Харта
- Установка параметров термистора
- Получение температуры
- Встроенное усреднение

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

## Содержание
- [Установка](#install)
- [Инициализация](#init)
- [Использование](#usage)
- [Пример](#example)
- [Версии](#versions)
- [Баги и обратная связь](#feedback)

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **GyverNTC** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/GyverNTC/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)

<a id="init"></a>
## Инициализация
![scheme](https://github.com/GyverLibs/GyverNTC/blob/main/docs/conn2.png)
```cpp
// подключение: GND --- термистор --- A0 --- 10к --- 5V
GyverNTC therm(0, 10000, 3435);	            // пин, сопротивление при 25 градусах (R термистора = R резистора!), бета-коэффициент
GyverNTC therm(0, 10000, 3435, 25, 10000);  // пин, R термистора, B термистора, базовая температура, R резистора
```

<a id="usage"></a>
## Использование
```cpp
float getTemp();                // прочитать температуру с пина
float getTempAverage();         // прочитать усреднённую температуру с пина
float computeTemp(int analog);  // получить температуру из 10 бит сигнала АЦП
```

<a id="example"></a>
## Пример
Остальные примеры смотри в **examples**!
```cpp
#include <GyverNTC.h>
GyverNTC therm(0, 10000, 3435);

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("Temperature ");
  Serial.print(therm.getTempAverage());
  Serial.println(" *C");
  delay(500);
}
```

<a id="versions"></a>
## Версии
- v1.0
- v1.1 - небольшая оптимизация, повышение точности
- v1.2 - оптимизация, поддержка 100к термисторов

<a id="feedback"></a>
## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!
