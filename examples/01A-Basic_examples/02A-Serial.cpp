#include <Arduino.h>    // Přidání knihovny s Arduino framework

void setup() {
    // Tato funkce se provede jednou při startu

    Serial.begin(115200);   // Začátek sériové komunikace s PC s rychlostí X baud/s
    Serial.println("Hello world!"); // Výpis textu na sériovou linku & nový řádek
}

void loop() {
    // Tato funkce se provádí periodicky neustále dokola

    Serial.print("It is ");
    Serial.print(millis()); // Výpis času od startu ESP v milisekundách
    Serial.println(" milliseconds since start.");

    /*
    Serial.print("It is ");
    Serial.print(millis()/1000);
    Serial.println(" seconds since start.");
    */

    delay(500); // Počkání X milisekund
}