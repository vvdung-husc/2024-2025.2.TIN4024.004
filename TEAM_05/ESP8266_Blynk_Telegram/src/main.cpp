// Đinh Viết Tín 
#define BLYNK_TEMPLATE_ID "TMPL6dnv6jqMP"
#define BLYNK_TEMPLATE_NAME "ESP8266 Blynk Telegram"
#define BLYNK_AUTH_TOKEN "PVSrkog9yqc_F7CtlpAuuMH7aJYxBA0Z"

#define BOT_TOKEN "7869624586:AAHbkuMDIIbX8wodUUvGXXh9X1AYr3sXjLU"
#define TELEGRAM_GROUP_ID "-4771876874"
#define TELEGRAM_USER_ID "1253151334"

#include <Arduino.h>
#include "utils.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define dhtPIN D0
#define dhtTYPE DHT11

#define OLED_SDA 13
#define OLED_SCL 12

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define BLYNK_VPIN_UPTIME V3
#define BLYNK_VPIN_TEMPERATURE V0
#define BLYNK_VPIN_HUMIDITY V2
#define BLYNK_VPIN_SWITCH V1

WiFiClientSecure secureClient;
UniversalTelegramBot telegramBot(BOT_TOKEN, secureClient);

bool yellowBlink = false;

bool trafficEnabled = true;

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

DHT dht(dhtPIN, dhtTYPE);

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

  Wire.begin(OLED_SDA, OLED_SCL);

  oled.begin();
  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Bngoc");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Nhóm Lập trình IoT");

  oled.sendBuffer();

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN); // Cấu hình Blynk với mã token
  Blynk.connect();

  secureClient.setInsecure(); // Bỏ qua chứng chỉ SSL để kết nối với Telegram
  telegramBot.sendMessage(TELEGRAM_GROUP_ID, "IoT Developer started up");
}

// Xử lý sự kiện khi nhấn công tắc trên Blynk
BLYNK_WRITE(BLYNK_VPIN_SWITCH)
{
  int switchState = param.asInt(); // Đọc trạng thái công tắc
  Serial.print("Switch state: ");
  Serial.println(switchState); // In trạng thái công tắc

  yellowBlink = (switchState == 1); // Bật/tắt đèn vàng nhấp nháy
  if (yellowBlink)
  {
    Serial.println("Yellow blink ON");
  }
  else
  {
    Serial.println("Yellow blink OFF");
    digitalWrite(yPIN, LOW); // Tắt đèn vàng khi không nhấp nháy
  }
}

void ThreeLedBlink()
{
  static ulong lastTimer = 0;
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (!IsReady(lastTimer, 1000))
    return;

  if (yellowBlink)
  {
    // Chế độ đèn vàng nhấp nháy
    Serial.println("Yellow LED blinking");
    digitalWrite(yPIN, !digitalRead(yPIN)); // Đèn vàng nhấp nháy
  }
  else
  {
    // Nhấp nháy ba đèn bình thường
    int prevLed = (currentLed + 2) % 3;
    digitalWrite(ledPin[prevLed], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
  }
}

float fHumidity = 0.0;
float fTemperature = 0.0;

void updateDHT()
{
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 2000))
    return;

  float h = random(0, 101) / 10.0;
  float t = random(-400, 801) / 10.0;
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
    Blynk.virtualWrite(BLYNK_VPIN_TEMPERATURE, t);
  }

  if (fHumidity != h)
  {
    bDraw = true;
    fHumidity = h;
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Blynk.virtualWrite(BLYNK_VPIN_HUMIDITY, h);
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
}

void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return;
  ulong value = millis() / 1000;
  Blynk.virtualWrite(V3, value);
  Blynk.virtualWrite(BLYNK_VPIN_UPTIME, value);
  lastTime = millis(); // Cập nhật lastTime sau khi gửi dữ liệu
}

void ProcessTelegramCommands()
{
  int msgCount = telegramBot.getUpdates(telegramBot.last_message_received + 1);
  if (msgCount == 0)
    return;

  while (msgCount)
  {
    for (int i = 0; i < msgCount; i++)
    {
      String command = telegramBot.messages[i].text;
      Serial.println("Received from Telegram: " + command);

      if (command == "/traffic_off")
      {
        digitalWrite(gPIN, LOW);
        digitalWrite(yPIN, LOW);
        digitalWrite(rPIN, LOW);
        telegramBot.sendMessage(TELEGRAM_GROUP_ID, "🚦 Traffic lights OFF!");
        trafficEnabled = false;
      }
      else if (command == "/traffic_on")
      {
        telegramBot.sendMessage(TELEGRAM_GROUP_ID, "🚦 Traffic lights ON!");
        trafficEnabled = true;
      }
    }
    msgCount = telegramBot.getUpdates(telegramBot.last_message_received + 1);
  }
}

void loop()
{
  if (!WelcomeDisplayTimeout())
    return;
  ThreeLedBlink();
  updateDHT();
  uptimeBlynk();
  ProcessTelegramCommands(); // Xử lý các lệnh từ Telegram
  if (Blynk.connected())
  {
    Blynk.run();
  }
  else
  {
    Serial.println("Blynk not connected");
  }
}
