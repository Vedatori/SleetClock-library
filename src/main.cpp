#include <Arduino.h>
#include "SleetClock.h"

SleetClock sleetClock;
Adafruit_PCD8544 display = Adafruit_PCD8544(23, 19, 18, 5, 32);

void setup() {
  Serial.begin(115200);
  

  display.begin();
  display.setContrast(60);

  display.display(); // show splashscreen
  
  sleetClock.init();
  delay(200);
  
  
}

void loop() {
  
  Serial.print("Encoder: ");
  Serial.print(sleetClock.encoder.getCount());
  sleetClock.dallasTemp.requestTemperaturesByIndex(0);
  Serial.print(" Temperature: ");
  Serial.println(sleetClock.dallasTemp.getTempCByIndex(0));
  
  delay(100);


}