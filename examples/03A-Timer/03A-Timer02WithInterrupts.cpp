#include <Arduino.h>
#include "SleetClock.h"

SleetClock sleetClock;

int timerMillis = 60000;
const int timerLast = 10000;
unsigned long startTimerMillis = 0;

bool pressHasBeenDetected = false;
uint8_t timeInSeconds = 0;

void IRAM_ATTR handleEncoderPress() {
    if(pressHasBeenDetected == false){
        pressHasBeenDetected = true;
    }
}
void IRAM_ATTR handleButton0Press() {
    if(pressHasBeenDetected == false){
        timeInSeconds++;        
    }
}
void IRAM_ATTR handleButton1Press() {
    if(pressHasBeenDetected == false){
        timeInSeconds--;        
    }
}

void setup() {
    Serial.begin(115200);
    sleetClock.init();
    sleetClock.drawLogo();
    attachInterrupt(digitalPinToInterrupt(sleetClock.encoderSwitch), handleEncoderPress, HIGH);
    attachInterrupt(digitalPinToInterrupt(sleetClock.button0), handleButton0Press, HIGH);
    attachInterrupt(digitalPinToInterrupt(sleetClock.button1), handleButton1Press, HIGH);
    while(pressHasBeenDetected == false){
        Serial.println(timeInSeconds);        
        sleep(1);
    }
    timerMillis = timeInSeconds * 1000;
    startTimerMillis = millis();
    Serial.println("Countdown start");
}

void loop() {
    unsigned long currentMillis = millis() - startTimerMillis;
    if(currentMillis > timerMillis && currentMillis < timerMillis + timerLast){
        sleetClock.setBuzzerOn();
    }
    else{
        sleetClock.setBuzzerOff();
    }
    unsigned remainingTime = (timerMillis - currentMillis)/1000;
    Serial.println(remainingTime);
    delay(100);
}


