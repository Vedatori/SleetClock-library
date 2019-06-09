#include "SleetClock.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <ESP32/ESP32_encoder.h>
#include <OneWire.h>
#include <DallasTemperature.h>



SleetClock::SleetClock() {}

void SleetClock::init() {
    pwm = Adafruit_PWMServoDriver();
    pwm.begin();
    pwm.setPWMFreq(1000);
    Wire.setClock(400000);
    
    this->allOff();
    
    encoder.risingSensitive = false;
    encoder.init();

    dallasTemp = DallasTemperature(&oneWireDS);
    dallasTemp.begin();
    
    display.begin();
}

void SleetClock::analogWrite(uint8_t pcaPin, uint16_t value) {
    if(value > 4095)
        value = 4095;
    if(pcaPin == displayBacklight || pcaPin == buzzer)
        pwm.setPWM(pcaPin, 0, 4095 - value);
    else
        pwm.setPWM(pcaPin, 0, value);
}

void SleetClock::allOff() {
    for(uint8_t pin = 0; pin < 16; ++pin) {
        this->analogWrite(pin, 0);
    }
}

void SleetClock::showOnDisplay() {
    
}