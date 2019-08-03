#ifndef _SLEET_CLOCK_H
#define _SLEET_CLOCK_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <ESP32/ESP32_encoder.h>
#include <ESP32/ESP32_touchBar.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

/* weather definition */
enum Weather{
    CLEAR_DAY = 0, CLEAR_NIGHT = 1, CLOUDY = 2,
    RAIN = 3, HEAVY_RAIN = 4,
    SNOW = 5, SLEET = 6, WIND = 7, FOG = 8,
    PARTLY_CLOUDY_DAY = 9, PARTLY_CLOUDY_NIGHT = 10,
    UNAVAILABLE = 11, OTHER = 12, INITIAL = 13,
};

class SleetClock {
    Adafruit_PWMServoDriver pwm;
    Weather previousWeather = INITIAL;
    unsigned animationStep;
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
    static const uint8_t displayDC = 18;
    static const uint8_t displayCE = 5;
    static const uint8_t displayRST = 32;

    static const uint8_t displayWidth = 84;
    static const uint8_t displayHeight = 84; 

    static const unsigned char logoVedatori[];   // Obtained using GIMP2 -> Export as .xbm with default settings

    ESP32_encoder encoder = ESP32_encoder(encoderA, encoderB);
    ESP32_touchBar touchBar = ESP32_touchBar();
    OneWire oneWireDS = OneWire(tempDHT);
    DallasTemperature dallasTemp;
    U8G2_PCD8544_84X48_F_4W_SW_SPI display = U8G2_PCD8544_84X48_F_4W_SW_SPI(U8G2_R0, displayCLK, displayDIN, displayCE, displayDC, displayRST);  // Nokia 5110 Display
    SleetClock();
    void init();
    void analogWrite(uint8_t pcaPin, uint16_t value);   //set PCA9685 connected devices power PWM in range 0(off)-4095(on)
    void allOff();  //turn off all external devices
    void updateState();
    void showOnDisplay();
    void drawLogo();
    void drawTimeTemps(struct tm timeNow, float inTemp, float outTemp);
    void drawForecast(struct tm timeNow, int8_t hoursOffset, float outTemp);
    void showWeatherOnLeds(Weather weather);
    void setAllWeatherLedsToZero();

    struct stateVector {
        float inTemp = 0.0;     //[°C]
        float outTemp = 0.0;    //[°C]
        int32_t cursor = 0;
        uint32_t cursorChangeTime = 0;   //[ms]
        int16_t illuminance = 0;   //12-bit reading
    } state;
};

#endif  //_SLEET_CLOCK_H