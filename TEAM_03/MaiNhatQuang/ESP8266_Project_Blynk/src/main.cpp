#define BLYNK_TEMPLATE_ID "TMPL6rGKAyzos"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project Blynk"
#define BLYNK_AUTH_TOKEN "JJNHS9mZkEvgIYO0yt64aeA4s8_3CGTX"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <U8g2lib.h>

// Chân LED (đỏ, vàng, xanh)
#define gPIN 15
#define yPIN 2
#define rPIN 5

//  Chân I2C OLED
#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//  Biến đếm thời gian
unsigned long runTime = 0;

//  Biến nhiệt độ & độ ẩm
float fTemperature = 0.0;
float fHumidity = 0.0;

//  Trạng thái đèn vàng nhấp nháy
bool yellowBlinkMode = false;

//  Kết nối Blynk
BlynkTimer timer;

//  Bắt sự kiện Switch từ Blynk (V1: bật/tắt nhấp nháy đèn vàng)
BLYNK_WRITE(V1) {
  yellowBlinkMode = param.asInt(); // 1 = bật, 0 = tắt
}

// Hiển thị màn hình chào mừng
bool WelcomeDisplayTimeout(unsigned int msSleep = 3000) {
  static unsigned long lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (millis() - lastTimer < msSleep) return false;
  bDone = true;
  return bDone;
}

// Hiển thị trên OLED
void updateOLED() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);

  oled.drawUTF8(0, 14, ("Nhiet do: " + String(fTemperature, 1) + "°C").c_str());
  oled.drawUTF8(0, 28, ("Do am: " + String(fHumidity, 1) + "%").c_str());
  oled.drawUTF8(0, 42, ("Thoi gian: " + String(runTime) + "s").c_str());

  oled.sendBuffer();
}

//  Điều khiển đèn giao thông
void TrafficLightControl() {
  static unsigned long lastTimer = 0;
  static int state = 0;
  static const unsigned long durations[] = {2000, 3000, 1000}; // Thời gian cho từng màu (ms)
  static const int ledPins[] = {rPIN, gPIN, yPIN};

  if (yellowBlinkMode) {
    // Nếu chế độ nhấp nháy đèn vàng được bật, chỉ nhấp nháy đèn vàng mỗi 500ms
    if (millis() - lastTimer > 500) {
      lastTimer = millis();
      digitalWrite(yPIN, !digitalRead(yPIN));
    }
    digitalWrite(rPIN, LOW);
    digitalWrite(gPIN, LOW);
    return;
  }

  // Nếu không ở chế độ nhấp nháy, điều khiển đèn theo chu trình
  if (millis() - lastTimer > durations[state]) {
    lastTimer = millis();
    digitalWrite(ledPins[state], LOW);
    state = (state + 1) % 3;
    digitalWrite(ledPins[state], HIGH);
  }
}

//  Gửi dữ liệu lên Blynk
void sendToBlynk() {
  Blynk.virtualWrite(V0, runTime);      // Thời gian chạy
  Blynk.virtualWrite(V2, fTemperature); // Nhiệt độ
  Blynk.virtualWrite(V3, fHumidity);    // Độ ẩm
}

//  Sinh dữ liệu nhiệt độ & độ ẩm ngẫu nhiên
float randomTemperature() {
  return random(-400, 800) / 10.0;
}

float randomHumidity() {
  return random(0, 1000) / 10.0;
}

// Cập nhật nhiệt độ & độ ẩm
void updateSensorData() {
  static unsigned long lastTimer = 0;
  if (millis() - lastTimer < 2000) return;
  lastTimer = millis();

  fTemperature = randomTemperature();
  fHumidity = randomHumidity();
  Serial.print("Nhiet do: ");
  Serial.print(fTemperature);
  Serial.println("°C");

  Serial.print("Do am: ");
  Serial.print(fHumidity);
  Serial.println("%");
}

//  Hiển thị thời gian chạy
void updateRunTime() {
  static unsigned long lastTimer = 0;
  if (millis() - lastTimer < 1000) return;
  lastTimer = millis();

  runTime++;
}

// SETUP
void setup() {
  Serial.begin(115200);

  // Cấu hình LED
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);
  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, HIGH); // Bắt đầu với đèn đỏ

  // Cấu hình OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Khoi dong...");
  oled.sendBuffer();

  // Kết nối WiFi & Blynk
  const char* ssid = "CNTT-MMT";       // Thay bằng SSID thực tế
  const char* password = "13572468";   // Thay bằng mật khẩu thực tế
  // const char* ssid = "Van Nghia 5G";       // Thay bằng SSID thực tế
  // const char* password = "0349270969";   // Thay bằng mật khẩu thực tế

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // Hẹn giờ gửi dữ liệu lên Blynk mỗi 2 giây
  timer.setInterval(2000L, sendToBlynk);
}

// LOOP
void loop() {
  Blynk.run();
  timer.run();

  if (!WelcomeDisplayTimeout()) return;

  TrafficLightControl();
  updateSensorData();
  updateRunTime();
  updateOLED();
}