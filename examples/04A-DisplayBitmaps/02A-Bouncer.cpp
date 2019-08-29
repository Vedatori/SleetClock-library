#include <Arduino.h>
#include "SleetClock.h"

SleetClock sleetClock;


const uint8_t radius = 5;
uint8_t x = radius;
uint8_t y = radius;
bool directionHorizontal = true;
bool directionVertical = true;

void setup() {
    Serial.begin(115200);
    sleetClock.init();
    sleetClock.drawLogo();
    delay(1000);
}

void loop() {
    uint8_t randomNumber = 1;
    sleetClock.display.clearBuffer();
    sleetClock.display.drawDisc(x, y, radius);
    sleetClock.display.sendBuffer();
    
    if(directionHorizontal && x + radius + randomNumber > 84)
        directionHorizontal = false;      
    else if(!directionHorizontal && x - radius - randomNumber < 0)
        directionHorizontal = true;
        
    if(directionHorizontal){
        x = x + randomNumber;
    }
    else{
        x = x - randomNumber;
    }
    
    if(directionVertical && y + radius + randomNumber > 48)
        directionVertical = false;      
    else if(!directionVertical && y - radius - randomNumber < 0)
        directionVertical = true;
    
    if(directionVertical){
        y = y + randomNumber;
    }
    else{
        y = y - randomNumber;
    }

}


