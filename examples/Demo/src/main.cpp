#include <Arduino.h>
#include "SleetClock.h"

SleetClock sleetClock;

void setup() {
    Serial.begin(115200);
    sleetClock.init();
    sleetClock.analogWrite(sleetClock.flakesLED, 200);
    sleetClock.analogWrite(sleetClock.cloudLED, 200);
}

void loop() {
    Serial.print("Encoder: ");
    Serial.print(sleetClock.encoder.getCount());
    sleetClock.dallasTemp.requestTemperaturesByIndex(0);
    Serial.print(" Temperature: ");
    Serial.println(sleetClock.dallasTemp.getTempCByIndex(0));
    delay(100);
}