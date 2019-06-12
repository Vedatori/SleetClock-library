#include <Arduino.h>
#include <WiFiManager.h>
#include "SleetClock.h"
#include "JsonListener.h"
#include "DarkskyParser.h"


SleetClock sleetClock;

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
    Serial.println("PARAM darkSkyApiKey = " + getParam("darkSkyKeyId"));
    Serial.println("PARAM coordinatesLatitude = " + getParam("coordinateLatitude"));
    Serial.println("PARAM coordinatesLongtitude = " + getParam("coordinateLongtitude"));
}

DarkskyParser dsParser;

const char *apiKey = "646f7e8e4fb6b4a169d193a8cc67ee2f";
const char *latitude = "49.2002";
const char *longitude = "16.6078";
const int darkSkyApiKeyLength = 32;
const int latitudeLongitudeLength = 9;

const char *tz = "0";
const char *ntpServer1 = "0.pool.ntp.org";
const char *ntpServer2 = "1.pool.ntp.org";
const char *ntpServer3 = "2.pool.ntp.org";

/* weather definition */
enum {
    CLEAR_DAY = 0, CLEAR_NIGHT = 1, CLOUDY = 2,
    RAIN = 3, HEAVY_RAIN = 4,
    SNOW = 5, SLEET = 6, WIND = 7, FOG = 8,
    PARTLY_CLOUDY_DAY = 9, PARTLY_CLOUDY_NIGHT = 10,
    UNAVAILABLE = 11, OTHER = 12, INITIAL = 13,
};

void printInfo(void *arg) {
    int currentWeather[12], lastWeather[12];
    int currentPrecipProbability[12], lastPrecipProbability[12];
    struct tm timeInfo;
    time_t lastUpdate = 0;
    int lastHour = -1;
    float lastTemperature = 0, lastHumidity = 0;
    char s[9];

    while (1) {
        time_t currentTime = time(NULL);

        /* check if the data is valid */
        if ((dsParser.lastUpdate == 0) || ((currentTime - dsParser.lastUpdate) > 3600)) {
            /* no data or data is too old */
            for (int i = 0; i < 12; i++) {
                currentWeather[i] = UNAVAILABLE;
            }
        } else if (lastUpdate < dsParser.lastUpdate) {
            /* dark sky data has been updated */
            for (int i = 0, pos = dsParser.currentHour; i < 13; i++) {
                if (i == 1) { /* skip dsParser.weatherData[1]. it is forecast data of current hour so abandon it. */
                    continue;
                }
                /* set weather and precipitation probablity */
                
                currentPrecipProbability[pos % 12] = dsParser.weatherData[i].precipProbability;
                pos++;
            }
            lastUpdate = dsParser.lastUpdate;
        }

        /* draw weather icon and precipProbability */
        for (int i = 0; i < 12; i++) {
            if (currentWeather[i] != lastWeather[i]) {
                lastWeather[i] = currentWeather[i];
            }
            if (currentPrecipProbability[i] != lastPrecipProbability[i]) {
                if (currentWeather[i] < UNAVAILABLE) {
                lastPrecipProbability[i] = currentPrecipProbability[i];
                }
            }
        }

        /* write temperature */
        if (lastTemperature != dsParser.weatherData[0].temperature) {
            sprintf(s, "%5.1fC", dsParser.weatherData[0].temperature);
            
            Serial.println(s); 
            lastTemperature = dsParser.weatherData[0].temperature;
        }

        getLocalTime(&timeInfo);
        sprintf(s, "%02d:%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
        Serial.println(s);

        /* hour changed. update the last hour data. delete underline at previous hour */
        if (lastHour != timeInfo.tm_hour) {
            int t = (timeInfo.tm_hour + 11) % 12;
                
            currentPrecipProbability[t] = dsParser.weatherData[13].precipProbability;
            lastHour = timeInfo.tm_hour;
        }
        
        /* draw humidity */
        if (lastHumidity != dsParser.weatherData[0].humidity) {
            sprintf(s, "%5.1f%%", dsParser.weatherData[0].humidity);    
            Serial.println(s);
            lastHumidity = dsParser.weatherData[0].humidity;
        }

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

    /*xTaskCreatePinnedToCore(printInfo, "printInfo", 2048, NULL, 1, NULL, 0);
    dsParser.begin(apiKey, latitude, longitude);
    configTzTime(tz, ntpServer1, ntpServer2, ntpServer3);
    dsParser.getData();*/

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