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

    display.begin();
    
    encoder.risingSensitive = false;
    encoder.init();
    touchBar.init();    // must be after display.begin

    dallasTemp = DallasTemperature(&oneWireDS);
    dallasTemp.begin();
}

void SleetClock::analogWrite(uint8_t pcaPin, uint16_t value) {
    if(value > 4095)
        value = 4095;
    if(pcaPin == buzzer)
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

void SleetClock::drawLogo() {
    display.clearBuffer();
    display.drawXBM(0, 0, displayWidth, displayHeight, logoVedatori);
    display.sendBuffer();
}

const unsigned char SleetClock::logoVedatori[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x20, 0x00,
        0x80, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x3f, 0x00, 0xf8, 0x00, 0xe0,
        0x03, 0x00, 0x1f, 0x00, 0x00, 0x80, 0x7f, 0x00, 0xb6, 0x03, 0xf0, 0x0f,
        0xc0, 0x73, 0x00, 0x00, 0xe0, 0xff, 0x81, 0x87, 0x07, 0xfc, 0x3f, 0xf0,
        0xf1, 0x00, 0x00, 0xf8, 0xff, 0xc7, 0xff, 0x1f, 0xff, 0xff, 0xf8, 0xff,
        0x03, 0x00, 0x78, 0x9e, 0xe7, 0x37, 0xbf, 0x1f, 0xfe, 0xfc, 0xfb, 0x07,
        0x00, 0x78, 0x8c, 0xe7, 0x07, 0xbf, 0x1f, 0xf0, 0xfc, 0xf9, 0x07, 0x00,
        0xf8, 0xcc, 0xe7, 0xc7, 0xbf, 0x1f, 0xf1, 0xfc, 0xf1, 0x07, 0x00, 0xf8,
        0xcc, 0xe7, 0xe7, 0xbf, 0x9f, 0xe3, 0xfc, 0xf0, 0x07, 0x00, 0xf8, 0xe0,
        0xe7, 0x07, 0xbf, 0x9f, 0xe7, 0xfc, 0xe4, 0x07, 0x00, 0xf8, 0xe1, 0xe7,
        0x07, 0xbf, 0x9f, 0xe7, 0x7c, 0xe6, 0x07, 0x00, 0xf8, 0xe1, 0xe7, 0xe7,
        0xbf, 0x9f, 0xe7, 0x7c, 0xc0, 0x07, 0x00, 0xf8, 0xf3, 0xe7, 0xe7, 0xbf,
        0x9f, 0xe3, 0x3c, 0xc0, 0x07, 0x00, 0xf8, 0xf3, 0xe7, 0x07, 0xbe, 0x1f,
        0xf0, 0x1c, 0x8f, 0x07, 0x00, 0xf8, 0xff, 0xe7, 0x07, 0x3e, 0x1f, 0xf8,
        0x3c, 0x9f, 0x07, 0x00, 0xf0, 0xff, 0xc3, 0xff, 0x0f, 0xfe, 0x7f, 0xf8,
        0xff, 0x01, 0x00, 0xc6, 0xff, 0x18, 0xff, 0xc7, 0xf8, 0x1f, 0xe3, 0xff,
        0x00, 0x80, 0x8f, 0x7f, 0x7c, 0xfc, 0xf1, 0xf1, 0x87, 0x8f, 0x3f, 0x00,
        0xc0, 0x3f, 0x1e, 0xff, 0x71, 0x4c, 0xc7, 0xe3, 0x1f, 0x0f, 0x00, 0xf0,
        0xff, 0xc0, 0xff, 0x03, 0x1e, 0x1f, 0xf8, 0x7f, 0x00, 0x00, 0xfc, 0xff,
        0xe3, 0xc7, 0x8f, 0xff, 0x3f, 0xfe, 0xff, 0x01, 0x00, 0x7e, 0xe0, 0xf3,
        0x01, 0xdf, 0x0f, 0x7e, 0xfe, 0xfc, 0x03, 0x00, 0x3e, 0xe0, 0xf3, 0x00,
        0xde, 0x0f, 0x7c, 0xfe, 0xfc, 0x03, 0x00, 0xfe, 0xf8, 0xf3, 0x78, 0xde,
        0xcf, 0x7c, 0xfe, 0xfc, 0x03, 0x00, 0xfe, 0xf9, 0xf3, 0x7c, 0xde, 0xcf,
        0x7c, 0xfe, 0xfc, 0x03, 0x00, 0xfe, 0xf9, 0x73, 0x7c, 0xde, 0x0f, 0x7c,
        0xfe, 0xfc, 0x03, 0x00, 0xfe, 0xf9, 0xf3, 0x7c, 0xde, 0x0f, 0x7e, 0xfe,
        0xfc, 0x03, 0x00, 0xfe, 0xf9, 0xf3, 0x38, 0xde, 0x4f, 0x7c, 0xfe, 0xfc,
        0x03, 0x00, 0xfe, 0xf9, 0xf3, 0x01, 0xde, 0xcf, 0x7c, 0xfe, 0xfc, 0x03,
        0x00, 0xfe, 0xf9, 0xf3, 0x03, 0xdf, 0xcf, 0x78, 0xfe, 0xfc, 0x03, 0x00,
        0xfe, 0xfd, 0xf3, 0xff, 0xdf, 0xef, 0x7d, 0xfe, 0xff, 0x01, 0x00, 0xf8,
        0xff, 0xe1, 0xff, 0x07, 0xff, 0x3f, 0xfc, 0xff, 0x00, 0x00, 0xe0, 0x7f,
        0x80, 0xff, 0x03, 0xfe, 0x0f, 0xf0, 0x3f, 0x00, 0x00, 0xc0, 0x1f, 0x00,
        0xfe, 0x00, 0xf8, 0x03, 0xc0, 0x1f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x3c,
        0x00, 0xe0, 0x01, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };