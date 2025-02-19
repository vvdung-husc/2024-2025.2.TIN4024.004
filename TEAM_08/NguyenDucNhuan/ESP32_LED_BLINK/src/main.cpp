#include <Arduino.h>

#define RED_LED 23
#define YELLOW_LED 22
#define GREEN_LED 21

unsigned long previousMillis = 0;
const long intervalRed = 5000;
const long intervalYellow = 2000;
const long intervalGreen = 5000;
int state = 0;

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    Serial.begin(115200);
}

void loop() {
    unsigned long currentMillis = millis();
    
    if (state == 0 && currentMillis - previousMillis >= intervalRed) {
        previousMillis = currentMillis;
        digitalWrite(RED_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        state = 1;
    } else if (state == 1 && currentMillis - previousMillis >= intervalYellow) {
        previousMillis = currentMillis;
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        state = 2;
    } else if (state == 2 && currentMillis - previousMillis >= intervalGreen) {
        previousMillis = currentMillis;
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        state = 0;
    }
}