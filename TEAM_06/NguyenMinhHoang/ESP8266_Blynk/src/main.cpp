
#define BLYNK_TEMPLATE_ID "TMPL6EPAXpI19"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project"
#define BLYNK_AUTH_TOKEN "Y-oMGzqsrxsk65a6ouk8UXJP0sGpJHHj"

#include <Arduino.h>
#include "utils.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi credentials
char ssid[] = "CNTT-MMT";          // SSID WiFi của bạn
char pass[] = "13572468";     // Mật khẩu WiFi của bạn

#define OLED_SDA 13
#define OLED_SCL 12
#define gPIN 15
#define yPIN 2
#define rPIN 5
#define dhtPIN 16
#define dhtTYPE DHT11

// OLED SH1106 Initialization
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

DHT dht(D0, dhtTYPE);

// Global variables
float fHumidity = 0.0;
float fTemperature = 0.0;
int displayMode = 0; // Modes: 0 - Welcome, 1 - Temp & Humidity, 2 - Uptime
bool isConnected = false;

void setup() {
  Serial.begin(115200);

  // Initialize GPIO pins
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  // Initialize DHT sensor
  dht.begin();

  // Initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);

  // Display Initial Message on OLED
  oled.drawUTF8(0, 14, "Dang ket noi WiFi...");
  oled.sendBuffer();

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Update OLED with WiFi connection success
  oled.clearBuffer();
  oled.drawUTF8(0, 14, "WiFi ket noi thanh cong!");
  oled.sendBuffer();
  delay(2000);

  // Connect to Blynk
  oled.clearBuffer();
  oled.drawUTF8(0, 28, "Dang ket noi Blynk...");
  oled.sendBuffer();
  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  oled.clearBuffer();
  oled.drawUTF8(0, 14, "Blynk ket noi thanh cong!");
  oled.sendBuffer();
  Serial.println("Blynk connected!");
  isConnected = true;
}

void ThreeLedBlink() {
  static ulong lastTimer = 0;
  static int currentLed = 0;
  const int ledPins[] = {gPIN, yPIN, rPIN};

  if (millis() - lastTimer < 1000) return; // Blink every 1 second
  lastTimer = millis();

  // Turn off the previous LED
  digitalWrite(ledPins[(currentLed + 2) % 3], LOW);
  // Turn on the current LED
  digitalWrite(ledPins[currentLed], HIGH);

  currentLed = (currentLed + 1) % 3;
}

void updateDHT() {
  static ulong lastTimer = 0;
  if (millis() - lastTimer < 2000) return; // Update every 2 seconds
  lastTimer = millis();

  // Simulate or read real DHT data
  float h = random(0, 1000) / 10.0;  // 0.0% - 100.0%
  float t = random(-400, 800) / 10.0;  // -40.0°C - 80.0°C
  // Uncomment for real sensor:
  // float h = dht.readHumidity();
  // float t = dht.readTemperature();

  fHumidity = h;
  fTemperature = t;

  Serial.print("Temperature: ");
  Serial.print(fTemperature);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(fHumidity);
  Serial.println(" %");

  // Send data to Blynk
  if (isConnected) {
    Blynk.virtualWrite(V1, fTemperature); // Send temperature
    Blynk.virtualWrite(V2, fHumidity);   // Send humidity
  }
}

void updateDisplay() {
  static ulong lastDisplaySwitch = 0;
  char buffer[32];

  if (millis() - lastDisplaySwitch < 5000) return; // Rotate every 5 seconds
  lastDisplaySwitch = millis();

  oled.clearBuffer();

  switch (displayMode) {
    case 0: // Welcome Screen
      oled.setFont(u8g2_font_unifont_t_vietnamese1);
      oled.drawUTF8(0, 14, "Trường ĐHKH Khoa CNTT");
      oled.drawUTF8(0, 28, "Lập trình IoT Nhom 4");
      oled.drawUTF8(0, 42, "Team 06");
      break;

    case 1: // Temperature and Humidity
      oled.setFont(u8g2_font_unifont_t_vietnamese2);
      sprintf(buffer, "Nhiet do: %.2f °C", fTemperature);
      oled.drawUTF8(0, 14, buffer);
      sprintf(buffer, "Do am: %.2f %%", fHumidity);
      oled.drawUTF8(0, 42, buffer);
      break;

    case 2: // Uptime
      ulong uptime = millis() / 1000;
      int hours = uptime / 3600;
      int minutes = (uptime % 3600) / 60;
      int seconds = uptime % 60;

      oled.setFont(u8g2_font_6x10_tf);
      sprintf(buffer, "Uptime: %02d:%02d:%02d", hours, minutes, seconds);
      oled.drawUTF8(0, 28, buffer);
      break;
  }

  oled.sendBuffer();
  displayMode = (displayMode + 1) % 3; // Cycle through modes
}

void loop() {
  if (!isConnected) return;
  Blynk.run();
  ThreeLedBlink();
  updateDHT();
  updateDisplay();
}