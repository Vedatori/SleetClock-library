#include <Arduino.h>    // Přidání knihovny s Arduino framework

void setup() {
    // Tato funkce se provede jednou při startu
    pinMode(0, OUTPUT); // Nastavení pinu 0 jako výstupní
    pinMode(2, OUTPUT); // Nastavení pinu 2 jako výstupní
    digitalWrite(0, LOW);   // Nastavení výstupního napětí pinu 0 na 0V
}

void loop() {
    // Tato funkce se provádí periodicky neustále dokola
    digitalWrite(2, HIGH);  // Nastavení výstupního napětí pinu 2 na 3.3V
    delay(500); // Čekání X milisekund
    digitalWrite(2, LOW);   // Nastavení výstupního napětí pinu 2 na 0V
    delay(500);
}
