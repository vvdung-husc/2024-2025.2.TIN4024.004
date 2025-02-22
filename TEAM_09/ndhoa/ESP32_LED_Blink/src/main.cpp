#include <Arduino.h>
#define LED_PIN 23

unsigned long previousMillis = 0;
const long interval = 1000; // Khoảng thời gian 1 giây

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello!");

  pinMode(LED_PIN, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  digitalWrite(LED_PIN, HIGH); // Bật đèn LED
  delay(1000);                 // Đợi 500ms
  digitalWrite(LED_PIN, LOW);  // Tắt đèn LED
  delay(1000);

  unsigned long currentMillis = millis(); // Lấy thời gian hiện tại

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis; // Cập nhật thời gian
    Serial.println("Non-blocking example");
  }
}