#include <Arduino.h>

// Chân GPIO kết nối với LED
const int LED_RED = 23;    // Đèn đỏ
const int LED_YELLOW = 22; // Đèn vàng
const int LED_GREEN = 21;  // Đèn xanh

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // Bật đèn xanh 3s
  digitalWrite(LED_GREEN, HIGH);
  delay(3000);
  digitalWrite(LED_GREEN, LOW);

  // Bật đèn vàng 5s
  digitalWrite(LED_YELLOW, HIGH);
  delay(5000);
  digitalWrite(LED_YELLOW, LOW);

  // Bật đèn đỏ 7s
  digitalWrite(LED_RED, HIGH);
  delay(7000);
  digitalWrite(LED_RED, LOW);
}
