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
    int currentWeather[12], lastWeather[12];
    int currentPrecipProbability[12], lastPrecipProbability[12];
    struct tm timeInfo;
    time_t lastUpdate = 0;
    int lastHour = -1;
    float lastTemperature = 0, lastHumidity = 0;
    char s[9];
    sleetClock.display.clearBuffer();

    while (1) {
        time_t currentTime = time(NULL);
        sleetClock.display.clearBuffer();

        sleetClock.display.sendBuffer();
        delay(100);
    }
}

void setup() {
    Serial.begin(115200);
    sleetClock.init();

    sleetClock.drawLogo();
    delay(1000);  

    struct tm timeNow = {};
    timeNow.tm_hour = 10;
    timeNow.tm_min = 1;
    timeNow.tm_sec = 54;
    sleetClock.drawTimeTemps(timeNow, 26.1, 23.0);

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
    dsParser.getData();
    // set printing to dispaly to another rutine
    xTaskCreatePinnedToCore(printInfo, "printInfo", 2048, NULL, 1, NULL, 0);

}

void loop() {
  
    Serial.print("Encoder: ");
    Serial.print(sleetClock.encoder.getCount());
    sleetClock.dallasTemp.requestTemperaturesByIndex(0);
    Serial.print(" Temperature: ");
    Serial.print(sleetClock.dallasTemp.getTempCByIndex(0));
    Serial.print(" TouchCount: ");
    Serial.print(sleetClock.touchBar.getCount());

    Serial.println();
    
    delay(100);
}