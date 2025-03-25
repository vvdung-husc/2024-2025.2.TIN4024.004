#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <secrets.h>

#define PIR_PIN 32

bool motionDetected = false;

void sendMessage(String message)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("❌ WiFi chưa kết nối, không thể gửi tin nhắn!");
    return;
  }

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + message;
  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0)
  {
    Serial.println("✅ Tin nhắn đã gửi: " + message);
  }
  else
  {
    Serial.println("❌ Gửi tin nhắn thất bại!");
  }
  http.end();
}

void setup()
{
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);

  // Kết nối WiFi trên Wokwi
  WiFi.begin("Wokwi-GUEST", "", 0, NULL, true);
  Serial.print("🔄 Đang kết nối WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi đã kết nối!");
  sendMessage("🚀 Thông báo phát hiện chuyển động!");
}

void loop()
{
  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH && !motionDetected)
  {
    Serial.println("🚨 Phát hiện chuyển động!");
    sendMessage("🚨 PIR phát hiện chuyển động!");
    motionDetected = true;
  }
  else if (pirState == LOW)
  {
    motionDetected = false;
  }
  delay(500);
}
