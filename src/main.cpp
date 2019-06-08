#include <Arduino.h>
#include <WiFiManager.h>
#include "SleetClock.h"

SleetClock sleetClock;
Adafruit_PCD8544 display = Adafruit_PCD8544(23, 19, 18, 5, 32);

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

void setup() {
  Serial.begin(115200);

  display.begin();
  display.setContrast(60);
  display.display(); 
  
  sleetClock.init();
  
  wm.resetSettings(); // wipe settings

  // add a custom input field
  int darkSkyApiKeyLenght = 32;
  int latitudeLongitudeLenght = 9;


  new (&darkSkyApiKey) WiFiManagerParameter("darkSkyKeyId", "Dark Sky API key", "", darkSkyApiKeyLenght,"placeholder=\"GUID\"");
  new (&coordinatesLatitude) WiFiManagerParameter("coordinateLatitude", "Coordinate Latitude", "", latitudeLongitudeLenght,"placeholder=\"37.8267\"");
  new (&coordinatesLongtitude) WiFiManagerParameter("coordinateLongtitude", "Coordinate Longtitude", "", latitudeLongitudeLenght,"placeholder=\"-122.4233\"");
  
  wm.addParameter(&darkSkyApiKey);
  wm.addParameter(&coordinatesLatitude);
  wm.addParameter(&coordinatesLongtitude);
  wm.setSaveParamsCallback(saveParamCallback);

  const char* menu[] = {"wifi","param","sep","exit"}; 
  wm.setMenu(menu,4);
  
  bool res;
  res = wm.autoConnect("Vedatori"); // password protected ap

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
    ESP.restart();
  } 
  else {
    Serial.println("Connected to wi-fi");
  }
  
}

void loop() {
  
  Serial.print("Encoder: ");
  Serial.print(sleetClock.encoder.getCount());
  sleetClock.dallasTemp.requestTemperaturesByIndex(0);
  Serial.print(" Temperature: ");
  Serial.println(sleetClock.dallasTemp.getTempCByIndex(0));
  
  delay(100);


}