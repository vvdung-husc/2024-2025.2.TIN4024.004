#include <Arduino.h>

const int redLed = 23;
const int yellowLed = 21;
const int greenLed = 22;

unsigned long previousMillis = 0;
unsigned long blinkPreviousMillis = 0;
int ledState = LOW;

void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  static int step = 0;
  static unsigned long interval = 0000; // Start with red LED timing
  static unsigned long blinkInterval = 1000; // Blink every 1s

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    step = (step + 1) % 3; // Cycle through 0, 1, 2

    if (step == 0) {
      interval = 5000; // Red LED for 5s
    } else if (step == 1) {
      interval = 3000; // Yellow LED for 3s
    } else if (step == 2) {
      interval = 5000; // Green LED for 5s
    }
  }

  if (currentMillis - blinkPreviousMillis >= blinkInterval) {
    blinkPreviousMillis = currentMillis;
    ledState = !ledState;
    if (step == 0) {
      digitalWrite(redLed, ledState);
      digitalWrite(yellowLed, LOW);
      digitalWrite(greenLed, LOW);
    } else if (step == 1) {
      digitalWrite(redLed, LOW);
      digitalWrite(yellowLed, ledState);
      digitalWrite(greenLed, LOW);
    } else if (step == 2) {
      digitalWrite(redLed, LOW);
      digitalWrite(yellowLed, LOW);
      digitalWrite(greenLed, ledState);
    }
  }
}

