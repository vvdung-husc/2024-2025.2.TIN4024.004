// Blynk authentication
#define BLYNK_TEMPLATE_ID "TMPL6uZv-JegF"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "BPRnO3B4iFlsVNrFRKtygQwcjFFhXNS1"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

// Khai báo chân LED
#define gPIN 15  // LED xanh
#define yPIN 2   // LED vàng
#define rPIN 5   // LED đỏ

// Khai báo chân cảm biến DHT11
#define dhtPIN 4    // Đổi từ GPIO16 sang GPIO4 (D2 trên NodeMCU)
#define dhtTYPE DHT11

// Khai báo chân OLED
#define OLED_SDA 13
#define OLED_SCL 12

// Thông tin WiFi
#define WIFI_SSID "3567"
#define WIFI_PASS "12345678"

// Khởi tạo đối tượng
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(dhtPIN, dhtTYPE);
BlynkTimer timer;

// Biến lưu trạng thái
bool yellowBlink = false;
unsigned long startTime;
float fHumidity = 0.0;
float fTemperature = 0.0;

// Xử lý nút trên Blynk (V0 bật/tắt LED vàng)
BLYNK_WRITE(V3) {
  yellowBlink = param.asInt();
}

// Khai báo trước các hàm
void ThreeLedBlink();
void updateDHT();
void sendUptimeToBlynk();
void checkWiFiConnection();

void setup() {
  Serial.begin(115200);

  // Cấu hình chân đầu ra
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  // Khởi động OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "HN Thành CNPM");
  oled.sendBuffer();

  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  checkWiFiConnection();

  // Khởi động Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  dht.begin();
  startTime = millis();

  // Thiết lập timer
  timer.setInterval(1000L, ThreeLedBlink);
  timer.setInterval(2000L, updateDHT);
  timer.setInterval(5000L, sendUptimeToBlynk);
}

// Hàm kiểm tra kết nối WiFi
void checkWiFiConnection() {
  Serial.print("Đang kết nối WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi đã kết nối!");
  } else {
    Serial.println("\n❌ Không thể kết nối WiFi!");
  }
}

// Điều khiển LED nhấp nháy
void ThreeLedBlink() {
  static int currentLed = 0;
  static unsigned long lastTimer = 0;

  if (yellowBlink) {
    if (millis() - lastTimer >= 500) {
      digitalWrite(gPIN, LOW);
      digitalWrite(rPIN, LOW);
      digitalWrite(yPIN, !digitalRead(yPIN)); // Nhấp nháy LED vàng
      lastTimer = millis();
    }
  } else {
    static const int ledPin[3] = {gPIN, yPIN, rPIN};
    if (millis() - lastTimer >= 1000) {
      digitalWrite(ledPin[(currentLed + 2) % 3], LOW);
      digitalWrite(ledPin[currentLed], HIGH);
      currentLed = (currentLed + 1) % 3;
      lastTimer = millis();
    }
  }
}

// Cập nhật dữ liệu từ cảm biến DHT11
void updateDHT() {
  //float h = dht.readHumidity();
  //float t = dht.readTemperature();
  float h = random(0,1001) / 10.0;
  float t = random(-400, 801) / 10.0; 

  if (isnan(h) || isnan(t)) {
    Serial.println("❌ Lỗi đọc dữ liệu từ cảm biến DHT!");
    return;
  }

  fTemperature = t;
  fHumidity = h;

  Serial.printf("🌡️ Nhiet do: %.1f°C | 💧 Do am: %.1f%%\n", fTemperature, fHumidity);

  // Hiển thị lên OLED
  oled.clearBuffer();
  oled.setCursor(0, 20);
  oled.printf("Nhiet do: %.1f°C", fTemperature);
  oled.setCursor(0, 40);
  oled.printf("Do am: %.1f%%", fHumidity);
  oled.sendBuffer();

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V1, fTemperature);
  Blynk.virtualWrite(V2, fHumidity);
}

// Gửi thời gian chạy lên Blynk
void sendUptimeToBlynk() {
  Blynk.virtualWrite(V0, (int)((millis() - startTime) / 1000));
}


// Loop chính
void loop() {
  Blynk.run();
  timer.run();
}
