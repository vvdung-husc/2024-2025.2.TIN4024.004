#include <Arduino.h>
#include "utils.h"
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <UniversalTelegramBot.h>

// Thông tin Blynk
//Nguyễn Trần Viết Thắng
#define BLYNK_TEMPLATE_ID "TMPL61bYE-l65"
#define BLYNK_TEMPLATE_NAME "ESP8266BlynkTele"
#define BLYNK_AUTH_TOKEN "5KUjzDBJEUFuI-cDkeTE-A3ziMAvZMQE"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST"; // Tên mạng WiFi
char pass[] = "";            // Mật khẩu mạng WiFi

// Cấu hình phần cứng
#define gPIN 15  // LED xanh
#define yPIN 2   // LED vàng
#define rPIN 5   // LED đỏ
#define dhtPIN 16
#define dhtTYPE DHT11
#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo đối tượng
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
DHT dht(dhtPIN, dhtTYPE);


// Thông tin Telegram
//Nguyễn Trần Viết Thắng
const char* botToken = "7958454010:AAGYkVfTHc-FgAg-YqOMgIo5CR-DRgvMB-I";
const char* chatID = "-4717614518";

//Trần Văn Minh Nhật
#define BOTtoken "7467700827:AAG5Jr3HcVMIdNTKWF4_DBsQXK0BnncU0dM"  // your Bot Token (Get from Botfather)
#define GROUP_ID "-4725634280" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

bool buttonState = false;

void setup()
{
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  dht.begin();
  Wire.begin(OLED_SDA, OLED_SCL);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  client.setInsecure();

  // Kiểm tra kết nối WiFi
  Serial.print("Đang kết nối WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n✅ WiFi đã kết nối!");

  Blynk.virtualWrite(V3, buttonState);
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");
  oled.sendBuffer();

  bot.sendMessage(chatID, "ESP8266 IoT đã khởi động!");
}

void updateDHT()
{
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 10000)) return;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("⚠️ Không thể đọc cảm biến DHT!");
    return;
  }

  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);

  static ulong lastTimeTele = 0;
  if (!IsReady(lastTimeTele, 300000)) return;

  bot.sendMessage(chatID, StringFormat("🌡 Nhiệt độ: %.2f°C", t));
  bot.sendMessage(chatID, StringFormat("💧 Độ ẩm: %.2f%%", h));
}

void checkTelegramMessages()
{
  int messageCount = bot.getUpdates(bot.last_message_received + 1);
  if (messageCount == 0) return;
  
  while (messageCount)
  {
    for (int i = 0; i < messageCount; i++)
    {
      String text = bot.messages[i].text;
      Serial.println("📩 Tin nhắn từ Telegram: " + text);
      
      if (text == "/traffic_off")
      {
        digitalWrite(gPIN, LOW);
        digitalWrite(yPIN, LOW);
        digitalWrite(rPIN, LOW);
        bot.sendMessage(chatID, "🚦 Đèn giao thông đã TẮT!");
      }
      else if (text == "/traffic_on")
      {
        digitalWrite(gPIN, HIGH);
        bot.sendMessage(chatID, "🚦 Đèn giao thông đang BẬT!");
      }
      else
      {
        bot.sendMessage(chatID, "⚠️ Lệnh không hợp lệ! Hãy gửi /traffic_on hoặc /traffic_off.");
      }
    }
    messageCount = bot.getUpdates(bot.last_message_received + 1);
  }
}

void loop()
{
  Blynk.run();
  updateDHT();
  checkTelegramMessages();
}
