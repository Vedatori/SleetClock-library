#include <Arduino.h>
#include "SleetClock.h"


SleetClock sleetClock;

const int timerMillis = 60000;
const int timerLast = 10000;

void setup() {
    Serial.begin(115200);
    sleetClock.init();
    sleetClock.drawLogo();
}

void loop() {
    unsigned long currentMillis = millis();
    if(currentMillis > timerMillis && currentMillis < timerMillis + timerLast){
        sleetClock.setBuzzerOn();
    }
    else{
        sleetClock.setBuzzerOff();
    }
    
    delay(100);
}

