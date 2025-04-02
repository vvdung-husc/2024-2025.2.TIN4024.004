#include <Arduino.h>

// Chân kết nối cảm biến PIR
#define PIR_PIN 2

void setup()
{
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
}

void loop()
{
  // Đọc tín hiệu từ cảm biến
  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH)
  {
    Serial.println("Phát hiện chuyển động!");
  }
  else
  {
    Serial.println("Không có chuyển động.");
  }
  delay(1000);
}
