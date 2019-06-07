#ifndef _SLEET_CLOCK_H
#define _SLEET_CLOCK_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <ESP32/ESP32_encoder.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


class SleetClock {
    Adafruit_PWMServoDriver pwm;
public:
    static const uint8_t ligthningLED = 0;
    static const uint8_t flakesLED = 1;
    static const uint8_t dropsLED = 2;
    static const uint8_t cloudLED = 3;
    static const uint8_t rgbR = 4;
    static const uint8_t rgbG = 5;
    static const uint8_t rgbB = 6;
    static const uint8_t displayBacklight = 7;
    static const uint8_t buzzer = 8;
    static const uint8_t servo0 = 9;
    static const uint8_t servo1 = 10;
    static const uint8_t servo2 = 11;
    static const uint8_t servo3 = 12;
    static const uint8_t servo4 = 13;
    static const uint8_t servo5 = 14;
    static const uint8_t servo6 = 15;

    static const uint8_t encoderSwitch = 17;
    static const uint8_t tempDHT = 16;
    static const uint8_t encoderA = 25;
    static const uint8_t encoderB = 26;
    static const uint8_t potentiometer = 34;
    static const uint8_t photoresistor = 35;
    static const uint8_t button0 = 36;
    static const uint8_t button1 = 39;
    static const uint8_t displayCLK = 23;
    static const uint8_t displayDIN = 19;
    static const uint8_t displayDC = 28;
    static const uint8_t displayCE = 5;
    static const uint8_t displayRST = 32;

    ESP32_encoder encoder = ESP32_encoder(encoderA, encoderB);
    OneWire oneWireDS = OneWire(tempDHT);
    DallasTemperature dallasTemp;
    Adafruit_PCD8544 *display;
    SleetClock();
    void init();
    void analogWrite(uint8_t pcaPin, uint16_t value);   //set PCA9685 connected devices power PWM in range 0(off)-4095(on)
    void allOff();  //turn off all external devices
    void showOnDisplay();
};

#endif  //_SLEET_CLOCK_H