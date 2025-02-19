#include <Arduino.h>
#include <TM1637Display.h>

// Chân GPIO kết nối với LED
const int LED_RED = 23;    // Đèn đỏ
const int LED_YELLOW = 22; // Đèn vàng
const int LED_GREEN = 21;  // Đèn xanh

// Chân kết nối TM1637
#define CLK 15
#define DIO 2
TM1637Display display(CLK, DIO);

void countdown(int seconds, int ledPin) {
  digitalWrite(ledPin, HIGH);
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i);
    delay(1000);
  }
  digitalWrite(ledPin, LOW);
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  display.setBrightness(7); // Độ sáng tối đa

  // Tất cả đèn tắt 5s khi khởi động
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  display.clear();
  delay(5000);
}

void loop() {
  countdown(3, LED_GREEN);   // Đèn xanh 3s
  countdown(5, LED_YELLOW);  // Đèn vàng 5s
  countdown(7, LED_RED);     // Đèn đỏ 7s
}
