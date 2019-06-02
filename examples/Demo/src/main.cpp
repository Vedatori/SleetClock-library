#include <Arduino.h>
#include "SleetClock.h"

SleetClock sleetClock;

void setup() {
    Serial.begin(115200);
    sleetClock = SleetClock();
    sleetClock.analogWrite(sleetClock.flakesLED, 200);
    sleetClock.analogWrite(sleetClock.cloudLED, 200);
}

void loop() {
    Serial.println(sleetClock.encoder.getCount());
    delay(100);
}