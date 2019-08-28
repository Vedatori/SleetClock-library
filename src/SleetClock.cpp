#include "SleetClock.h"
#include <Wire.h>
#include "Adafruit-PWM-Servo-Driver-Library/Adafruit_PWMServoDriver.h"
#include "IndieBlocks/src/ESP32_encoder.h"
#include "OneWire/OneWire.h"
#include "Arduino-Temperature-Control-Library/DallasTemperature.h"
#include <time.h>


SleetClock::SleetClock() {}

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
    state.inTemp = dallasTemp.getTempCByIndex(0);

    previousWeather = INITIAL;
}

void SleetClock::analogWrite(uint8_t pcaPin, uint16_t value) {
    if(value > 4095)
        value = 4095;
    
    pwm.setPWM(pcaPin, 0, value);
}

void SleetClock::allOff() {
    for(uint8_t pin = 0; pin < 16; ++pin) {
        this->analogWrite(pin, 0);
    }
}

void SleetClock::updateState() {
    dallasTemp.requestTemperaturesByIndex(0);
    float readTemp = dallasTemp.getTempCByIndex(0);
    if(readTemp > -55 && readTemp < 125)
        state.inTemp = 0.99*state.inTemp + 0.01*readTemp;    //IIR filter

    int32_t prevCursor = state.cursor;
    state.cursor += encoder.getDiff()/2 + touchBar.getDiff();
    if(state.cursor != prevCursor)
        state.cursorChangeTime = millis();

    state.illuminance = /*0.9*state.illuminance + 0.1*/analogRead(photoresistor);
    state.potentiometer = analogRead(potentiometer);
}

void SleetClock::showOnDisplay() {
    
}

void SleetClock::drawBitmap(const unsigned char bitmap) {
    display.clearBuffer(); 
    display.drawXBM(0, 0, displayWidth, displayHeight, bitmap);
    display.sendBuffer();
}

void SleetClock::drawLogo() {
    drawBitmap(logoVedatori);
}

void SleetClock::drawTimeTemps(struct tm timeNow, float inTemp, float outTemp) {
    display.clearBuffer();
    char outString[10];

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

void SleetClock::setAllWeatherLedsToZero(){
    analogWrite(ligthningLED, 0);
    analogWrite(flakesLED, 0);
    analogWrite(dropsLED, 0);
    analogWrite(cloudLED, 0);
    analogWrite(rgbR, 0);
    analogWrite(rgbG, 0);
    analogWrite(rgbB, 0);
}
void SleetClock::showWeatherOnLeds(Weather weather){
    //Serial.print((int)weather);
    //Serial.print('\t');

    if(previousWeather != weather) {
        previousWeather = weather;
        animationStep = 0;
        setAllWeatherLedsToZero();
    }
    //Serial.print(animationStep);
    //Serial.println('\t');
    switch (weather)
    {
        case CLEAR_DAY:
            analogWrite(rgbR, 4095);
            analogWrite(rgbG, 1525);
            break;
        case CLEAR_NIGHT:
            analogWrite(rgbR, 4095);
            analogWrite(rgbG, 1523);
            analogWrite(rgbB, 2300);
            break;
        case CLOUDY:
            analogWrite(cloudLED, 500);
            break;
        case RAIN:
            analogWrite(cloudLED, 500);
            if(animationStep % 4 == 0){
                analogWrite(dropsLED, 2048);
            }
            else{
                analogWrite(dropsLED, 0);
            }
            break;
        case HEAVY_RAIN:
            analogWrite(cloudLED, 500);
            if(animationStep % 2 == 0){
                analogWrite(dropsLED, 2048);
                analogWrite(ligthningLED, 0);
            }
            else if(animationStep % 3 == 0){
                analogWrite(dropsLED, 0);
                analogWrite(ligthningLED, 4096);
            }
            else{
                analogWrite(dropsLED, 0);
                analogWrite(ligthningLED, 0);
            }
            break;
        case SNOW:
            analogWrite(cloudLED, 500);
            if(animationStep % 3 == 0){
                analogWrite(flakesLED, 300);
            }
            else{
                analogWrite(flakesLED, 0);
            }
            break;
        case SLEET:
            analogWrite(cloudLED, 500);
            if(animationStep % 8 == 4){
                analogWrite(flakesLED, 300);
                analogWrite(dropsLED, 0);
            }
            else if(animationStep % 4 == 0){
                analogWrite(flakesLED, 0);
                analogWrite(dropsLED, 2048);
            }
            else{
                analogWrite(flakesLED, 0);
                analogWrite(dropsLED, 0);
            }

            break;
        case WIND:
            if(animationStep > 10){
                animationStep = 0;
            }
            if(animationStep < 5){
                analogWrite(cloudLED, 100*(animationStep+1));
            }
            else{
                analogWrite(cloudLED, (10-animationStep+1)*100);
            }
            analogWrite(rgbR, 0);
            analogWrite(rgbG, 0);
            analogWrite(rgbB, 0);
            break;
        case FOG:
            analogWrite(cloudLED, 500);
            analogWrite(rgbR, 400);
            analogWrite(rgbG, 200);
            break;
        case PARTLY_CLOUDY_DAY:
            analogWrite(rgbR, 4095);
            analogWrite(rgbG, 1525);
            analogWrite(cloudLED, 200);
            break;
        case PARTLY_CLOUDY_NIGHT:
            analogWrite(rgbR, 4095);
            analogWrite(rgbG, 1523);
            analogWrite(rgbB, 2300);
            analogWrite(cloudLED, 200);
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

void SleetClock::setBuzzerOff(){
    setBuzzerLevel(0);
}

void SleetClock::setBuzzerOn(){
    setBuzzerLevel(1500);
}

void SleetClock::setBuzzerLevel(unsigned level){    
    analogWrite(buzzer, level);
}

void SleetClock::setRedLevel(unsigned level){
    analogWrite(rgbR, level);
}

void SleetClock::setGreenLevel(unsigned level){
    analogWrite(rgbG, level);
}

void SleetClock::setBlueLevel(unsigned level){
    analogWrite(rgbB, level);
}

void SleetClock::setLigthningLEDLevel(unsigned level){
    analogWrite(ligthningLED, level);
}
void SleetClock::setFlakesLEDLevel(unsigned level){
    analogWrite(flakesLED, level);
}

void SleetClock::setDropsLEDLevel(unsigned level){
    analogWrite(dropsLED, level);
}

void SleetClock::setCloudLEDLevel(unsigned level){
    analogWrite(cloudLED, level);
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