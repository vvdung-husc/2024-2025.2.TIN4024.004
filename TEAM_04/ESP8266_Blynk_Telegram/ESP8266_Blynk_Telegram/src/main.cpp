//Nguyen Dinh Ngoc Bao
#define BLYNK_TEMPLATE_ID "TMPL6fjFNdL5O"
#define BLYNK_TEMPLATE_NAME "ESP8266Project"
#define BLYNK_AUTH_TOKEN "EqapOpB088ZJ2obRlqC0FFqewej-al3E"
#define BOT_TOKEN "7647186542:AAE51tLWwz_fQbT8ceTq5ZC2x4_lpd2VYug"
#define TELEGRAM_GROUP_ID "-4624526302" 
#define TELEGRAM_USER_ID "7647186542" 

//Nguyen Huu Phuoc


//Phan Ba Dan



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

#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 5

#define SENSOR_PIN 16
#define SENSOR_TYPE DHT11

#define OLED_SDA_PIN 13
#define OLED_SCL_PIN 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

DHT dht(SENSOR_PIN, SENSOR_TYPE);

char wifiSSID[] = "Workwi-guest";
char wifiPASS[] = "";

WiFiClientSecure secureClient;
UniversalTelegramBot telegramBot(BOT_TOKEN, secureClient);

bool switchState = false;

bool DisplayWelcomeTimeout(uint timeoutMS = 5000)
{
  static ulong lastTime = 0;
  static bool done = false;
  if (done)
    return true;
  if (!IsReady(lastTime, timeoutMS))
    return false;
  done = true;
  return done;
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  dht.begin();

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  Blynk.begin(BLYNK_AUTH_TOKEN, wifiSSID, wifiPASS);
  secureClient.setInsecure();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("❌ WiFi connection failed!");
  }
  else
  {
    Serial.println("✅ WiFi connected!");
  }
  Blynk.virtualWrite(V3, switchState);
  oled.begin();
  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT");

  oled.sendBuffer();
  randomSeed(analogRead(0));

  telegramBot.sendMessage(TELEGRAM_GROUP_ID, "IoT Developer started up");
}

void CycleTrafficLights()
{
  static ulong lastTime = 0;
  static int currentLight = 0;
  static const int lightPins[3] = {LED_GREEN, LED_YELLOW, LED_RED};

  if (!IsReady(lastTime, 1000))
    return;
  int prevLight = (currentLight + 2) % 3;
  digitalWrite(lightPins[prevLight], LOW);
  digitalWrite(lightPins[currentLight], HIGH);
  currentLight = (currentLight + 1) % 3;
}

float humidityVal = 0.0;
float temperatureVal = 0.0;

void ReadSensorData()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 10000))
    return;

    float humidity = random(0, 10001) / 100.0;
    float temperature = random(-4000, 8001) / 100.0;
    

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  bool shouldUpdate = false;

  if (temperatureVal != temperature)
  {
    shouldUpdate = true;
    temperatureVal = temperature;
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
  }

  if (humidityVal != humidity)
  {
    shouldUpdate = true;
    humidityVal = humidity;
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
  }
  if (shouldUpdate)
  {
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);

    String tempStr = StringFormat("Nhiet do: %.2f °C", temperature);
    oled.drawUTF8(0, 14, tempStr.c_str());

    String humStr = StringFormat("Do am: %.2f %%", humidity);
    oled.drawUTF8(0, 42, humStr.c_str());

    oled.sendBuffer();

  }
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
}

void BlinkYellow()
{
  static bool yellowBlink = false;
  static unsigned long lastBlinkTime = 0;
  if (IsReady(lastBlinkTime, 500))
  {
    yellowBlink = !yellowBlink;
    digitalWrite(LED_YELLOW, yellowBlink);
  }

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
}

void UpdateUptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return;
  ulong elapsedTime = lastTime / 1000;
  Blynk.virtualWrite(V0, elapsedTime);
}

BLYNK_WRITE(V3)
{
  switchState = param.asInt();
}

bool trafficEnabled = true;
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
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, LOW);
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
  if (!DisplayWelcomeTimeout())
    return;
  if (trafficEnabled)
    CycleTrafficLights();
  ReadSensorData();
  UpdateUptimeBlynk();
  if (switchState)
  {
    BlinkYellow();
  }
  ProcessTelegramCommands();
}