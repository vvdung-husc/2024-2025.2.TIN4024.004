#include <Arduino.h>

// put function declarations here:
#define LED_PIN 23

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);//Bật đèn
  delay(1000);
  digitalWrite(LED_PIN, LOW);//Bật đèn
  delay(1000);
}

// put function definitions here:

