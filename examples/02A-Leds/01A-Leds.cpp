#include <Arduino.h>
#include "SleetClock.h"


SleetClock sleetClock;

void renderOnDisplay(char* letter, unsigned value){
    sleetClock.display.clearBuffer();
    sleetClock.display.setFont(u8g2_font_ncenB14_tr);
    sleetClock.display.drawStr(0, 15, letter);
    char snum[5];
    itoa(value, snum, 10);
    printf("%s", snum);
    sleetClock.display.drawStr(0, 30, snum);
    sleetClock.display.sendBuffer();    
}

void writeToSerial(char* letter, unsigned value){
    Serial.print("LED ");
    Serial.print(letter);
    Serial.print(" ");
    Serial.println(value);
}

void setup() {
    Serial.begin(115200);
    sleetClock.init();
    sleetClock.drawLogo();
}

void loop() {
    unsigned level = analogRead(sleetClock.potentiometer); 
    unsigned remainder = sleetClock.encoder.getCount() % 3;
    char marking[] = "R";
    switch (remainder)
    {
    case 0:
        marking[0] = 'R'; 
        writeToSerial(marking, level);
        renderOnDisplay(marking, level);
        sleetClock.setRedLevel(level);
        break;
    case 1:
        marking[0] = 'G';
        writeToSerial(marking, level);
        renderOnDisplay(marking, level);
        sleetClock.setGreenLevel(level);
        break;
    case 2:
        marking[0] = 'B';
        writeToSerial(marking, level);
        renderOnDisplay(marking, level);
        sleetClock.setBlueLevel(level);
        break;
    default:
        break;
    }
    
    
    delay(100);
}

