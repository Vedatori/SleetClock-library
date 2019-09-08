#include "SleetClock.h"
#include <Wire.h>
#include "Adafruit-PWM-Servo-Driver-Library/Adafruit_PWMServoDriver.h"
#include "IndieBlocks/src/ESP32_encoder.h"
#include "OneWire/OneWire.h"
#include "Arduino-Temperature-Control-Library/DallasTemperature.h"
#include <time.h>


SleetClock::SleetClock() {}

float SleetClock::tempExchange;
DallasTemperature SleetClock::dallasTemp;

void SleetClock::init() {
    pwm = Adafruit_PWMServoDriver();
    pwm.begin();
    pwm.setPWMFreq(1000);
    Wire.setClock(400000);
    
    allOff();

    display.begin();
    
    encoder.risingSensitive = true;
    encoder.init();
    touchBar.init();    // must be after display.begin

    pinMode(encoderSwitch, INPUT);
    pinMode(button0, INPUT);
    pinMode(button1, INPUT);

    dallasTemp = DallasTemperature(&oneWireDS);
    dallasTemp.begin();
    dallasTemp.requestTemperaturesByIndex(0);
    tempExchange = dallasTemp.getTempCByIndex(0);

    previousWeather = INITIAL;
    xTaskCreate(updateTemp, "printInfo", 2048, NULL, 1, NULL);
}

void SleetClock::analogWrite(uint8_t pcaPin, uint16_t value) {
    if(pcaPinsPwmState[pcaPin] == value)
        return;
    if(value > 4095)
        value = 4095;
    pwm.setPWM(pcaPin, 0, value);
    pcaPinsPwmState[pcaPin] = value;
}

void SleetClock::allOff() {
    for(uint8_t pin = 0; pin < 16; ++pin) {
        analogWrite(pin, 0);
    }
}

void SleetClock::updateState() {
    int32_t prevCursor = state.cursor;
    state.cursor += encoder.getDiff()/2 + touchBar.getDiff();
    if(state.cursor != prevCursor)
        state.cursorChangeTime = millis();

    state.illuminance = /*0.9*state.illuminance + 0.1*/analogRead(photoresistor);
    state.potentiometer = analogRead(potentiometer);
    state.inTemp = tempExchange;
}

void SleetClock::showOnDisplay() {
    
}

void SleetClock::drawBitmap(const unsigned char* bitmap) {
    display.clearBuffer(); 
    display.drawXBM(0, 0, displayWidth, displayHeight, bitmap);
    display.sendBuffer();
}

void SleetClock::drawConnecting(const char * ssid) {
    display.clearBuffer();

    display.setFont(u8g2_font_tenthinnerguys_tf);
    display.drawStr(12, 10, "SleetClock");

    display.setFont(u8g2_font_haxrcorp4089_tr);
    display.drawStr(0, 22, "Connecting to WiFi...");
    display.drawStr(0, 34, " If unsuccessful find");
    display.drawStr(0, 46, ssid);

    display.sendBuffer();
}

void SleetClock::drawTimeTemps(struct tm timeNow, float inTemp, float outTemp) {
    display.clearBuffer();
    char outString[16];

    display.setFont(u8g2_font_profont29_mf);
    sprintf(outString, "%02d:%02d", timeNow.tm_hour, timeNow.tm_min);
    display.drawStr(3, 19, outString);

    display.setFont(u8g2_font_5x8_mn);
    sprintf(outString, "%02d", timeNow.tm_sec);
    display.drawStr(36, 30, outString);
    display.drawEllipse(40, 27, 7, 6, U8G2_DRAW_ALL);

    display.setFont(u8g2_font_tenthinnerguys_tf );
    display.drawStr(11, 35, "In");
    display.drawStr(56, 35, "Out");

    sprintf(outString, "%.1f°C", inTemp);
    display.drawUTF8(0, 48, outString);

    sprintf(outString, "%.1f°C", outTemp);
    display.drawUTF8(46, 48, outString);

    display.sendBuffer();
}

void SleetClock::drawForecast(struct tm timeNow, int8_t hoursOffset, float outTemp) {
    timeNow.tm_hour += hoursOffset;
    if(timeNow.tm_hour > 23)
        timeNow.tm_hour -= 24;
    else if(timeNow.tm_hour < 0)
        timeNow.tm_hour += 24;

    display.clearBuffer();
    char outString[8];

    display.setFont(u8g2_font_profont29_mf);
    sprintf(outString, "%02d:%02d", timeNow.tm_hour, timeNow.tm_min);
    display.drawStr(3, 19, outString);

    display.setFont(u8g2_font_tenthinnerguys_tf );
    sprintf(outString, "In %d", hoursOffset);
    display.drawStr(6, 35, outString);
    display.drawStr(56, 35, "Out");

    if(hoursOffset == 1 || hoursOffset == -1)
        display.drawStr(0, 48, "hour");
    else
        display.drawStr(0, 48, "hours");
    

    sprintf(outString, "%.1f°C", outTemp);
    display.drawUTF8(46, 48, outString);

    display.sendBuffer();
}

void SleetClock::setAllWeatherLedsToZero() {
    analogWrite(ligthningLED, 0);
    analogWrite(flakesLED, 0);
    analogWrite(dropsLED, 0);
    analogWrite(cloudLED, 0);
    analogWrite(rgbR, 0);
    analogWrite(rgbG, 0);
    analogWrite(rgbB, 0);
}
void SleetClock::showWeatherOnLeds(Weather weather) {
    static unsigned animationStep = 0;
    if(previousWeather != weather) {
        previousWeather = weather;
        animationStep = 0;
    }
    switch (weather)
    {
        case CLEAR_DAY:
            showWeatherSegment(clearDayLeds);
            break;
        case CLEAR_NIGHT:
            showWeatherSegment(clearNightLeds);
            break;
        case CLOUDY:
            showWeatherSegment(cloudyLeds);
            break;
        case RAIN:
            if(animationStep < 15)
                showWeatherSegment(rainLeds[0]);
            else {    
                showWeatherSegment(rainLeds[1]);
                if(animationStep > 30)
                    animationStep = 0;
            }
            break;
        case HEAVY_RAIN:
            if(animationStep < 10)
                showWeatherSegment(heavyRainLeds[0]);
            else if(animationStep < 12)
                showWeatherSegment(heavyRainLeds[1]);
            else {
                showWeatherSegment(heavyRainLeds[2]);
                if(animationStep > 30)
                    animationStep = 0;
            }
            break;
        case SNOW:
            if(animationStep < 20)
                showWeatherSegment(snowLeds[0]);
            else {    
                showWeatherSegment(snowLeds[1]);
                if(animationStep > 30)
                    animationStep = 0;
            }
            break;
        case SLEET:
            if(animationStep < 10)
                showWeatherSegment(sleetLeds[0]);
            else if(animationStep < 20)
                showWeatherSegment(sleetLeds[1]);
            else {
                showWeatherSegment(sleetLeds[2]);
                if(animationStep > 30)
                    animationStep = 0;
            }
            break;
        case WIND:
            if(animationStep < 15)
                showWeatherSegment(windLeds[0]);
            else {    
                showWeatherSegment(windLeds[1]);
                if(animationStep > 30)
                    animationStep = 0;
            }
            break;
        case FOG:
            showWeatherSegment(fogLeds);
            break;
        case PARTLY_CLOUDY_DAY:
            showWeatherSegment(partlyCloudyDayLeds);
            break;
        case PARTLY_CLOUDY_NIGHT:
            showWeatherSegment(partlyCloudyNightLeds);
            break;
        case UNAVAILABLE:
            setAllWeatherLedsToZero();
            break;
        case OTHER:
            setAllWeatherLedsToZero();
            break;
        case INITIAL:
            setAllWeatherLedsToZero();
            break;
    }
    animationStep++;
}

void SleetClock::showWeatherSegment(const int16_t * weatherLedsSegment) {
    for(int i = 0; i < 7; ++i) {
        int16_t difference = weatherLedsSegment[i] - pcaPinsPwmState[i];
        bool falling = difference < 0;
        if(difference > weatherLedSteps[falling][i])
            analogWrite(i, pcaPinsPwmState[i] + weatherLedSteps[falling][i]);
        else if(difference < -weatherLedSteps[falling][i])
            analogWrite(i, pcaPinsPwmState[i] - weatherLedSteps[falling][i]);
        else
            analogWrite(i, pcaPinsPwmState[i] + difference);
    }
}

void SleetClock::setBuzzerOff() {
    setBuzzerLevel(0);
}

void SleetClock::setBuzzerOn() {
    setBuzzerLevel(1500);
}

void SleetClock::setBuzzerLevel(unsigned level) {    
    analogWrite(buzzer, level);
}

void SleetClock::setRedLevel(unsigned level) {
    analogWrite(rgbR, level);
}

void SleetClock::setGreenLevel(unsigned level) {
    analogWrite(rgbG, level);
}

void SleetClock::setBlueLevel(unsigned level) {
    analogWrite(rgbB, level);
}

void SleetClock::setLigthningLEDLevel(unsigned level) {
    analogWrite(ligthningLED, level);
}
void SleetClock::setFlakesLEDLevel(unsigned level) {
    analogWrite(flakesLED, level);
}

void SleetClock::setDropsLEDLevel(unsigned level) {
    analogWrite(dropsLED, level);
}

void SleetClock::setCloudLEDLevel(unsigned level) {
    analogWrite(cloudLED, level);
}
void SleetClock::updateTemp(void * args) {
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = 1000 / portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount();
    for(;;) {
        vTaskDelayUntil( &xLastWakeTime, xPeriod );
        dallasTemp.requestTemperaturesByIndex(0);
        float readTemp = dallasTemp.getTempCByIndex(0);
        if(readTemp > -55 && readTemp < 125)
        tempExchange = 0.9*tempExchange + 0.1*readTemp;    //IIR filter
    }
}