#include <Arduino.h>
#include "utils.h"

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <U8g2lib.h>


#include <Esp8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define dhtPIN 16     // Digital pin connected to the DHT sensor
#define dhtTYPE DHT11 // DHT 22 (AM2302)

#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

DHT dht(D0, dhtTYPE);

// WiFi credentials
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";



// #define BOTtoken 


// #define GROUP_ID "" // thường là một số âm



WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

bool buttonState = false;

bool WelcomeDisplayTimeout(uint msSleep = 5000)
{
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone)
    return true;
  if (!IsReady(lastTimer, msSleep))
    return false;
  bDone = true;
  return bDone;
}

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

  Wire.begin(OLED_SDA, OLED_SCL); // SDA, SCL
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  client.setInsecure();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("❌ WiFi connection failed!");
  }
  else
  {
    Serial.println("✅ WiFi connected!");
  }
  Blynk.virtualWrite(V3, buttonState);
  oled.begin();
  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");

  oled.sendBuffer();
  randomSeed(analogRead(0));

  bot.sendMessage(GROUP_ID, "IoT Developer started up");
}

void ThreeLedBlink()
{
  static ulong lastTimer = 0;
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (!IsReady(lastTimer, 1000))
    return;
  int prevLed = (currentLed + 2) % 3;
  digitalWrite(ledPin[prevLed], LOW);
  digitalWrite(ledPin[currentLed], HIGH);
  currentLed = (currentLed + 1) % 3;
}

String getTemperatureEffect(float temperature)
{
  int category = (temperature < 10) ? 0 : (temperature < 15) ? 1
                                      : (temperature < 25)   ? 2
                                      : (temperature < 30)   ? 3
                                      : (temperature < 35)   ? 4
                                      : (temperature < 40)   ? 5
                                                             : 6;

  String message = "Nhiệt độ " + String(temperature, 2) + "°C: ";

  switch (category)
  {
  case 0:
    return message + "Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch.";
  case 1:
    return message + "Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp.";
  case 2:
    return message + "Nhiệt độ dễ chịu, không ảnh hưởng nhiều đến sức khỏe.";
  case 3:
    return message + "Nhiệt độ lý tưởng, ít ảnh hưởng đến sức khỏe.";
  case 4:
    return message + "Cơ thể bắt đầu có dấu hiệu mất nước, mệt mỏi.";
  case 5:
    return message + "Nguy cơ sốc nhiệt, chuột rút, say nắng.";
  case 6:
    return message + "Cực kỳ nguy hiểm, có thể gây suy nội tạng, đột quỵ nhiệt!";
  default:
    return message + "Không xác định.";
  }
}

String getHumidityEffect(float humidity)
{
  int category = (humidity < 30) ? 0 : (humidity < 40) ? 1
                                   : (humidity <= 60)  ? 2
                                   : (humidity <= 70)  ? 3
                                   : (humidity <= 80)  ? 4
                                                       : 5;

  String message = "Độ ẩm " + String(humidity, 2) + "%: ";

  switch (category)
  {
  case 0:
    return message + "Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh về hô hấp (viêm họng, khô mũi).";
  case 1:
    return message + "Hơi khô, có thể gây khó chịu với một số người.";
  case 2:
    return message + "Mức lý tưởng, ít ảnh hưởng đến sức khỏe.";
  case 3:
    return message + "Bắt đầu khó chịu, có nguy cơ nấm mốc, vi khuẩn phát triển.";
  case 4:
    return message + "Cảm giác oi bức, khó thở, cơ thể khó thoát mồ hôi, tăng nguy cơ sốc nhiệt.";
  case 5:
    return message + "Cực kỳ nguy hiểm, môi trường quá ẩm có thể gây bệnh nghiêm trọng về hô hấp!";
  default:
    return message + "Không xác định.";
  }
}

float fHumidity = 0.0;
float fTemperature = 0.0;

void updateDHT()
{
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 10000))
    return;

  // float h = dht.readHumidity();
  // float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  float h = random(0, 10001) / 100.0;    // [0.00, 100.00]
  float t = random(-4000, 8001) / 100.0; // [-40.00, 80.00]

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  bool bDraw = false;

  if (fTemperature != t)
  {
    bDraw = true;
    fTemperature = t;
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
  }

  if (fHumidity != h)
  {
    bDraw = true;
    fHumidity = h;
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
  }
  if (bDraw)
  {
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);

    String s = StringFormat("Nhiet do: %.2f °C", t);
    oled.drawUTF8(0, 14, s.c_str());

    s = StringFormat("Do am: %.2f %%", h);
    oled.drawUTF8(0, 42, s.c_str());

    oled.sendBuffer();
  }
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  static ulong lastTimeTele = 0;
  if (!IsReady(lastTimeTele, 300000))
    return;
  bot.sendMessage(GROUP_ID, getTemperatureEffect(t));
  bot.sendMessage(GROUP_ID, getHumidityEffect(h));
}

void BlinkingYellowLight()
{
  static bool yellowState = false;
  static unsigned long previousBlinkMillis = 0;
  if (IsReady(previousBlinkMillis, 500))
  {
    yellowState = !yellowState;
    digitalWrite(yPIN, yellowState);
  }

  digitalWrite(gPIN, LOW);
  digitalWrite(rPIN, LOW);
}
void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return; // Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
}
BLYNK_WRITE(V3)
{
  buttonState = param.asInt();
}

bool isTrafficOn = true;
void checkTelegramMessages()
{
  int messageCount = bot.getUpdates(bot.last_message_received + 1);
  if (messageCount == 0)
    return;
  while (messageCount)
  {
    for (int i = 0; i < messageCount; i++)
    {
      String text = bot.messages[i].text;

      Serial.println("Tin nhắn từ Telegram: " + text);

      if (text == "/traffic_off")
      {
        digitalWrite(gPIN, LOW);
        digitalWrite(yPIN, LOW);
        digitalWrite(rPIN, LOW);
        bot.sendMessage(GROUP_ID, "🚦 Đèn giao thông đã TẮT!");
        isTrafficOn = false;
      }
      else if (text == "/traffic_on")
      {
        bot.sendMessage(GROUP_ID, "🚦 Đèn giao thông đang BẬT!");
        isTrafficOn = true;
      }
      else
      {
        bot.sendMessage(GROUP_ID, "⚠️ Lệnh không hợp lệ! Hãy gửi /traffic_on hoặc /traffic_off.");
      }
    }
    messageCount = bot.getUpdates(bot.last_message_received + 1);
  }
}

void loop()
{
  if (!WelcomeDisplayTimeout())
    return;
  if (isTrafficOn)
    ThreeLedBlink();
  updateDHT();
  uptimeBlynk();
  if (buttonState)
  {
    BlinkingYellowLight();
  }
  checkTelegramMessages();
}