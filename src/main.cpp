#include <Arduino.h>
#include <WiFiManager.h>
#include "SleetClock.h"
#include "JsonListener.h"
#include "DarkskyParser.h"


SleetClock sleetClock;

DarkskyParser dsParser;

String apiKey = String("646f7e8e4fb6b4a169d193a8cc67ee2f");
String latitude = String("49.2002");
String longitude = String("16.6078");
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

void saveParamCallback(){
    Serial.println("[CALLBACK] saveParamCallback fired");
    apiKey = getParam("darkSkyKeyId");
    Serial.println("PARAM darkSkyApiKey = " + apiKey);
    latitude = getParam("coordinateLatitude");
    Serial.println("PARAM coordinatesLatitude = " + latitude);
    longitude = getParam("coordinateLongtitude");
    Serial.println("PARAM coordinatesLongtitude = " + longitude);
}

void printInfo(void *arg) {
    /*int currentWeather[12], lastWeather[12];
    int currentPrecipProbability[12], lastPrecipProbability[12];
    time_t lastUpdate = 0;
    int lastHour = -1;
    float lastTemperature = 0, lastHumidity = 0;*/
    struct tm timeInfo;

    while (1) {
        //Get sensors data
        sleetClock.updateState();

        //time_t currentTime = time(NULL);
        getLocalTime(&timeInfo);
        if(sleetClock.state.cursor < 0) {
            sleetClock.state.cursor = 0;
        }
        else if(sleetClock.state.cursor >= DS_NUMBER_OF_HOURLY_DATA) {
            sleetClock.state.cursor = DS_NUMBER_OF_HOURLY_DATA - 1;
        }
        if(sleetClock.state.cursor == 0)
            sleetClock.drawTimeTemps(timeInfo, sleetClock.state.inTemp, dsParser.weatherData[sleetClock.state.cursor].temperature);
        else {
            sleetClock.drawForecast(timeInfo, sleetClock.state.cursor, dsParser.weatherData[sleetClock.state.cursor].temperature);
            if((millis() - sleetClock.state.cursorChangeTime) > 5000) {
                sleetClock.state.cursor = 0;
            }
        }
        sleetClock.showWeatherOnLeds((Weather)dsParser.weatherData[sleetClock.state.cursor].weather);
        delay(200);
    }
}

void setup() {
    Serial.begin(115200);
    sleetClock.init();
    sleetClock.drawLogo();

    new (&darkSkyApiKey) WiFiManagerParameter("darkSkyKeyId", "Dark Sky API key", "", darkSkyApiKeyLength, "placeholder=\"GUID\"");
    new (&coordinatesLatitude) WiFiManagerParameter("coordinateLatitude", "Coordinate Latitude", "", latitudeLongitudeLength, "placeholder=\"37.8267\"");
    new (&coordinatesLongtitude) WiFiManagerParameter("coordinateLongtitude", "Coordinate Longtitude", "", latitudeLongitudeLength, "placeholder=\"-122.4233\"");
    
    wm.addParameter(&darkSkyApiKey);
    wm.addParameter(&coordinatesLatitude);
    wm.addParameter(&coordinatesLongtitude);
    wm.setSaveParamsCallback(saveParamCallback);

    const char* menu[] = {"wifi","param","sep","exit"}; 
    wm.setMenu(menu,4);
    
    bool res;
    res = wm.autoConnect("Vedatori");

    if(!res) {
        Serial.println("Failed to connect or hit timeout");
        ESP.restart();
    } 
    else {
        Serial.println("Connected to wi-fi");
    }
    dsParser.begin(apiKey.c_str(), latitude.c_str(), longitude.c_str());
    configTime(tz, 0, ntpServer1, ntpServer2, ntpServer3);

    // set printing to dispaly to another rutine
    /*for (int i = 0; i< DS_NUMBER_OF_HOURLY_DATA; i++){
        Serial.print(dsParser.weatherData[i].weather);
        Serial.print(" ");
        Serial.print(dsParser.weatherData[i].temperature);
        Serial.print(" ");
        Serial.println(dsParser.weatherData[i].precipProbability);
        for(int j = 0; j < 20;j++){
            sleetClock.showWeatherOnLeds((Weather)dsParser.weatherData[i].weather);
            delay(100);
        }
    }*/
    xTaskCreatePinnedToCore(printInfo, "printInfo", 2048, NULL, 1, NULL, 0);

}

void loop() {

    Serial.print("Cursor: ");
    Serial.print(sleetClock.state.cursor);

    Serial.println();
    dsParser.getData();
    
    delay(10000);
}