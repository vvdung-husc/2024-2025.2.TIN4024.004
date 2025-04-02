#define BLYNK_TEMPLATE_ID "TMPL6pEvA_RXf"
#define BLYNK_TEMPLATE_NAME "BLYNK"
#define BLYNK_AUTH_TOKEN "VYupZeVSLopvlKEBTu9_tKlX5pI3rSYr"

#include <Arduino.h>
#include "utils.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define dhtPIN D0     // Digital pin connected to the DHT sensor
#define dhtTYPE DHT11 // DHT 11

#define OLED_SDA 13
#define OLED_SCL 12

#define WIFI_SSID "CNTT-MMT"
#define WIFI_PASSWORD "13572468"
#define WIFI_CHANNEL 6

#define BLYNK_VPIN_UPTIME V3
#define BLYNK_VPIN_TEMPERATURE V0
#define BLYNK_VPIN_HUMIDITY V2
#define BLYNK_VPIN_SWITCH V1

// Biến lưu trạng thái của switch
bool yellowBlink = false;

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

DHT dht(dhtPIN, dhtTYPE);

bool WelcomeDisplayTimeout(uint msSleep = 5000) {
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;
  return bDone;
}

void setup() {
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL);  // SDA, SCL

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
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN); // Cấu hình Blynk với mã token
  Blynk.connect();
}

// Xử lý sự kiện khi nhấn công tắc trên Blynk
BLYNK_WRITE(BLYNK_VPIN_SWITCH) {
  int switchState = param.asInt();  // Đọc trạng thái công tắc
  Serial.print("Switch state: ");
  Serial.println(switchState); // In trạng thái công tắc
  
  yellowBlink = (switchState == 1); // Bật/tắt đèn vàng nhấp nháy
  if (yellowBlink) {
    Serial.println("Yellow blink ON");
  } else {
    Serial.println("Yellow blink OFF");
    digitalWrite(yPIN, LOW);  // Tắt đèn vàng khi không nhấp nháy
  }
}

void ThreeLedBlink() {
  static ulong lastTimer = 0;
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (!IsReady(lastTimer, 1000)) return;

  if (yellowBlink) {
    // Chế độ đèn vàng nhấp nháy
    Serial.println("Yellow LED blinking");
    digitalWrite(yPIN, !digitalRead(yPIN)); // Đèn vàng nhấp nháy
  } else {
    // Nhấp nháy ba đèn bình thường
    int prevLed = (currentLed + 2) % 3;
    digitalWrite(ledPin[prevLed], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
  }
}

float fHumidity = 0.0;
float fTemperature = 0.0;

void updateDHT() {
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 2000)) return;

  float h = random(0, 101) / 10.0; //dht.readHumidity();
  float t = random(-400, 801) / 10.0; //dht.readTemperature();  // or dht.readTemperature(true) for Fahrenheit
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  bool bDraw = false;

  if (fTemperature != t) {
    bDraw = true;
    fTemperature = t;
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    Blynk.virtualWrite(BLYNK_VPIN_TEMPERATURE, t);
  }

  if (fHumidity != h) {
    bDraw = true;
    fHumidity = h;
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Blynk.virtualWrite(BLYNK_VPIN_HUMIDITY, h);
  }

  if (bDraw) {
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);

    String s = StringFormat("Nhiet do: %.2f °C", t);
    oled.drawUTF8(0, 14, s.c_str());

    s = StringFormat("Do am: %.2f %%", h);
    oled.drawUTF8(0, 42, s.c_str());

    oled.sendBuffer();
  }
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = millis() / 1000;
  Blynk.virtualWrite(V3, value);
  Blynk.virtualWrite(BLYNK_VPIN_UPTIME, value);
  lastTime = millis(); // Cập nhật lastTime sau khi gửi dữ liệu
}

void loop() {
  if (!WelcomeDisplayTimeout()) return;
  ThreeLedBlink();
  updateDHT();
  uptimeBlynk();
  if (Blynk.connected()) {
    Blynk.run();
  } else {
    Serial.println("Blynk not connected");
  }
}
