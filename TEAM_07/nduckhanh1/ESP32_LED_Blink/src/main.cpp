#include <Arduino.h>
#define LED_PIN 23
void setup() {
  // put your setup code here, to run once:
 pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
