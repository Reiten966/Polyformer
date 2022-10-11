// GND --- термистор --- A0 --- 10к --- 5V
#include <GyverNTC.h>
GyverNTC therm(0, 10000, 3435);	// пин, сопротивление при 25 градусах (R термистора = R резистора!), бета-коэффициент
// GyverNTC therm(0, 10000, 3435, 25, 10000);	// пин, R термистора, B термистора, базовая температура, R резистора
// серый 4300
// проводной 3950

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("Temperature ");
  Serial.print(therm.getTempAverage());
  Serial.println(" *C");
  delay(500);
}
