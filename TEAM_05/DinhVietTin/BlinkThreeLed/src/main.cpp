#include <Arduino.h>

#define LED_RED 23
#define LED_YELLOW 21
#define LED_GREEN 19
void setup()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop()
{
  digitalWrite(LED_RED, HIGH);
  delay(7000);
  digitalWrite(LED_RED, LOW);

  digitalWrite(LED_GREEN, HIGH);
  delay(10000);
  digitalWrite(LED_GREEN, LOW);

  digitalWrite(LED_YELLOW, HIGH);
  delay(2000);
  digitalWrite(LED_YELLOW, LOW);
}