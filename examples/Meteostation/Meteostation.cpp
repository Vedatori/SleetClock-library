#include <Arduino.h>
#include <WiFiManager.h>
#include "SleetClock.h"
#include "DarkskyParser.h"
#include <Preferences.h>

SleetClock sleetClock;
DarkskyParser dsParser;
Preferences preferences;

const char * deviceName = "SleetClock";
String apiKey;
String latitude;
String longitude;
const int darkSkyApiKeyLength = 32;
const int latitudeLongitudeLength = 9;

const int tz = 2* 3600;
const char *ntpServer1 = "0.pool.ntp.org";
const char *ntpServer2 = "1.pool.ntp.org";
const char *ntpServer3 = "2.pool.ntp.org";

WiFiManager wm; // global wm instance
WiFiManagerParameter darkSkyApiKey; // global param ( for non blocking w params )
WiFiManagerParameter coordinatesLatitude; 
WiFiManagerParameter coordinatesLongtitude; 

String getParam(String name){
    //read parameter from server, for customhmtl input
    String value;
    if(wm.server->hasArg(name)) {
        value = wm.server->arg(name);
    }
    return value;
}

void saveParamCallback() {
    Serial.println("[CALLBACK] saveParamCallback fired");
    apiKey = getParam("darkSkyKeyId");
    preferences.putString("apiKey", apiKey);
    Serial.println("PARAM darkSkyApiKey = " + apiKey);
    latitude = getParam("coordinateLatitude");
    preferences.putString("latitude", latitude);
    Serial.println("PARAM coordinatesLatitude = " + latitude);
    longitude = getParam("coordinateLongtitude");
    preferences.putString("longitude", longitude);
    Serial.println("PARAM coordinatesLongtitude = " + longitude);
}

void printInfo(void *arg) {
    struct tm timeInfo;

    TickType_t xLastWakeTime;
    const TickType_t xPeriod = 100 / portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount ();

    for(;;) {
        //Get sensors data
        sleetClock.updateState();
        
        //time_t currentTime = time(NULL);{}
        getLocalTime(&timeInfo);
        if(sleetClock.state.cursor < 0) {
            sleetClock.state.cursor = 0;
        }
        else if(sleetClock.state.cursor >= DS_NUMBER_OF_HOURLY_DATA) {
            sleetClock.state.cursor = DS_NUMBER_OF_HOURLY_DATA - 1;
        }
        if(sleetClock.state.cursor == 0) {
            sleetClock.drawTimeTemps(timeInfo, sleetClock.state.inTemp, dsParser.weatherData[sleetClock.state.cursor].temperature);
        }
        else {
            sleetClock.drawForecast(timeInfo, sleetClock.state.cursor, dsParser.weatherData[sleetClock.state.cursor].temperature);
            if((millis() - sleetClock.state.cursorChangeTime) > 5000) {
                sleetClock.state.cursor = 0;
            }
        }
        sleetClock.showWeatherOnLeds((Weather)dsParser.weatherData[sleetClock.state.cursor].weather);
        
        static int resetCounter = 0;
        if(digitalRead(sleetClock.button0)) {
            ++resetCounter;
            if(resetCounter > 50) {
                wm.resetSettings();
                ESP.restart();
            }
        }

        sleetClock.showWeatherOnLeds((Weather)dsParser.weatherData[sleetClock.state.cursor].weather);
        int16_t displayBrightness = sleetClock.state.potentiometer - sleetClock.state.illuminance;
        if(displayBrightness < 0)
            displayBrightness = 0;
        else if(displayBrightness > 4095 )
            displayBrightness = 4095;
        sleetClock.analogWrite(sleetClock.displayBacklight, displayBrightness);

        vTaskDelayUntil( &xLastWakeTime, xPeriod ); //Wait for the next cycle.
    }
}

void updateLcdBacklight(void *arg) {
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = 1000 / portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount();
    for(;;) {
        int16_t displayBrightness = sleetClock.state.potentiometer - sleetClock.state.illuminance;
        if(displayBrightness < 0)
            displayBrightness = 0;
        else if(displayBrightness > 4095 )
            displayBrightness = 4095;
        sleetClock.analogWrite(sleetClock.displayBacklight, displayBrightness);
        vTaskDelayUntil( &xLastWakeTime, xPeriod ); //Wait for the next cycle.
    }
}

void setup() {
    Serial.begin(115200);
    sleetClock.init();
    sleetClock.drawConnecting(deviceName);

    preferences.begin("darkSkyProps", false);
    apiKey = preferences.getString("apiKey", "646f7e8e4fb6b4a169d193a8cc67ee2f");
    latitude = preferences.getString("latitude", "49.195084");  //Brno main square
    longitude = preferences.getString("latitude", "16.608140"); //Brno main square

    new (&darkSkyApiKey) WiFiManagerParameter("darkSkyKeyId", "Dark Sky API key", "", darkSkyApiKeyLength, "placeholder=\"GUID\"");
    new (&coordinatesLatitude) WiFiManagerParameter("coordinateLatitude", "Coordinate Latitude", "", latitudeLongitudeLength, "placeholder=\"49.195084\"");
    new (&coordinatesLongtitude) WiFiManagerParameter("coordinateLongtitude", "Coordinate Longtitude", "", latitudeLongitudeLength, "placeholder=\"16.608140\"");
    
    wm.addParameter(&darkSkyApiKey);
    wm.addParameter(&coordinatesLatitude);
    wm.addParameter(&coordinatesLongtitude);
    wm.setSaveParamsCallback(saveParamCallback);
    wm.setHostname(deviceName);

    const char* menu[] = {"wifi","param","sep","exit"}; 
    wm.setMenu(menu,4);
    
    bool res;
    res = wm.autoConnect(deviceName);

    if(!res) {
        Serial.println("Failed to connect or hit timeout");
        ESP.restart();
    } 
    else {
        Serial.println("Connected to wi-fi");
    }
    dsParser.begin(apiKey.c_str(), latitude.c_str(), longitude.c_str());
    configTime(tz, 0, ntpServer1, ntpServer2, ntpServer3);
    preferences.end();

    xTaskCreate(printInfo, "printInfo", 2048, NULL, 2, NULL);
    xTaskCreate(updateLcdBacklight, "updateLcdBacklight", 2048, NULL, 1, NULL);
}

void loop() {
    dsParser.getData();
    delay(300000);
}