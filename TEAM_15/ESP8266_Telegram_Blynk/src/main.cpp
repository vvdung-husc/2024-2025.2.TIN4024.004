#define BLYNK_PRINT Serial
#include <Arduino.h>
#define BLYNK_TEMPLATE_ID "TMPL6oG3Nj8wA"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "HURYMOI5FgZO17csG7YVZ4HtJRk7Zm3z"
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";
#include "utils.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define dhtPIN 16
#define dhtTYPE DHT11

#define OLED_SDA 13
#define OLED_SCL 12

// Thông tin WiFi & Blynk

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
DHT dht(dhtPIN, dhtTYPE);

bool onlyYellowBlink = false; // Biến kiểm soát chế độ chớp đèn vàng
uint runtimeCounter = 1;      // Đếm thời gian chạy

BlynkTimer timer;

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
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");
  oled.sendBuffer();

  // Kết nối Blynk
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "128.199.144.129", 80);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

  // Cập nhật thời gian chạy mỗi giây
  timer.setInterval(1000, []()
                    { Blynk.virtualWrite(V0, runtimeCounter++); });
}

BLYNK_WRITE(V1)
{
  onlyYellowBlink = param.asInt(); // Đọc trạng thái từ Blynk
  Serial.println(onlyYellowBlink ? "Chế độ chỉ nháy đèn vàng" : "Chế độ bình thường");
}

void ThreeLedBlink()
{
  static ulong lastTimer = 0;
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (!IsReady(lastTimer, 1000))
    return;

  if (onlyYellowBlink)
  {
    digitalWrite(gPIN, LOW);
    digitalWrite(rPIN, LOW);
    digitalWrite(yPIN, !digitalRead(yPIN));
  }
  else
  {
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
  float h = random(-400, 801) / 10.0;
  float t = random(0, 101) / 10.0;
  // float h = dht.readHumidity();
  // float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (fTemperature != t)
  {
    fTemperature = t;
    Blynk.virtualWrite(V2, t);
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
  }

  if (fHumidity != h)
  {
    fHumidity = h;
    Blynk.virtualWrite(V3, h);
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
  }

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);
  oled.drawUTF8(0, 14, StringFormat("Nhiet do: %.2f °C", t).c_str());
  oled.drawUTF8(0, 42, StringFormat("Do am: %.2f %%", h).c_str());
  oled.sendBuffer();
}

void loop()
{
  Blynk.run();
  timer.run();

  if (!WelcomeDisplayTimeout())
    return;

  ThreeLedBlink();
  updateDHT();
}