#include <Arduino.h>
#include "SleetClock.h"       

SleetClock sleetClock;

void setup() {
    Serial.begin(115200);
    sleetClock = SleetClock();

    sleetClock.analogWrite(sleetClock.flakesLED, 500);
    sleetClock.analogWrite(sleetClock.cloudLED, 500);
}

void loop() {
    Serial.println(analogRead(sleetClock.potentiometer));
    delay(500);
}