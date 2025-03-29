
// Blynk definitions
#define BLYNK_TEMPLATE_ID "TMPL6fmNwFU9z"
#define BLYNK_TEMPLATE_NAME "ESP8266 Blynk"
#define BLYNK_AUTH_TOKEN "684U9Nh21rRDqdpXiAuUhzk-hFWa9bhc"

#include <Arduino.h>
#include "utils.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial  // Định nghĩa debug output

// WiFi credentials - Wokwi
char ssid[] = "Dell";      
char pass[] = "12345678";  

// Pin definitions
#define gPIN 15
#define yPIN 2
#define rPIN 5
#define dhtPIN 16     // Digital pin connected to the DHT sensor
#define dhtTYPE DHT11 // DHT 22 (AM2302)
#define OLED_SDA 13
#define OLED_SCL 12

// Blynk virtual pins
#define VPIN_TEMPERATURE V0
#define VPIN_HUMIDITY V1
#define VPIN_SWITCH V2
#define VPIN_OPERATION_TIME V3

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(dhtPIN, dhtTYPE);

// Variables
float fHumidity = 0.0;
float fTemperature = 0.0;
bool switchState = false;
unsigned long operationStartTime = 0;
unsigned long currentOperationTime = 0;
bool blynkConnected = false;
unsigned long lastBlynkConnectionAttempt = 0;

// Khai báo prototype cho các hàm
void updateOLED();
bool WelcomeDisplayTimeout(uint msSleep = 5000);
void ThreeLedBlink();
void updateDHT();
void updateOperationTime();
void connectToBlynk();

// Đọc trạng thái switch từ Blynk
BLYNK_WRITE(VPIN_SWITCH) {
  switchState = param.asInt();
  
  if (switchState) {
    digitalWrite(gPIN, HIGH);
    digitalWrite(yPIN, LOW);
    digitalWrite(rPIN, LOW);
  } else {
    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    digitalWrite(rPIN, LOW);
  }
  
  // Hiển thị trạng thái trên OLED
  updateOLED();
}

// Kết nối đến Blynk
void connectToBlynk() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  if (millis() - lastBlynkConnectionAttempt < 30000 && lastBlynkConnectionAttempt > 0) {
    return; // Tránh kết nối lại quá nhanh
  }
  
  lastBlynkConnectionAttempt = millis();
  
  Serial.println("Connecting to Blynk...");
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "WiFi Connected");
  oled.drawUTF8(0, 28, "Connecting...");
  oled.drawUTF8(0, 42, "to Blynk");
  oled.sendBuffer();
  
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  if (Blynk.connect(10000)) {  // 10 giây timeout
    Serial.println("Connected to Blynk!");
    blynkConnected = true;
    
    oled.clearBuffer();
    oled.drawUTF8(0, 14, "IOT");
    oled.drawUTF8(0, 28, "Connected");
    oled.drawUTF8(0, 42, "to Blynk");
    oled.sendBuffer();
    delay(2000);
  } else {
    Serial.println("Failed to connect to Blynk");
    blynkConnected = false;
    
    oled.clearBuffer();
    oled.drawUTF8(0, 14, "IOT");
    oled.drawUTF8(0, 28, "Blynk Connect");
    oled.drawUTF8(0, 42, "Failed");
    oled.sendBuffer();
    delay(2000);
  }
}

bool WelcomeDisplayTimeout(uint msSleep) {
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;    
  return bDone;
}

void updateOLED() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);
  
  String s = StringFormat("Nhiet do: %.2f °C", fTemperature);
  oled.drawUTF8(0, 14, s.c_str());  
  
  s = StringFormat("Do am: %.2f %%", fHumidity);
  oled.drawUTF8(0, 28, s.c_str());
  
  s = StringFormat("Switch: %s", switchState ? "ON" : "OFF");
  oled.drawUTF8(0, 42, s.c_str());
  
  s = StringFormat("Time: %lu s", currentOperationTime / 1000);
  oled.drawUTF8(0, 56, s.c_str());
  
  oled.sendBuffer();
}

void ThreeLedBlink() {
  static ulong lastTimer = 0;
  static int currentLed = 0;  
  static const int ledPin[3] = {gPIN, yPIN, rPIN};
  
  // Nếu switch đang bật thì không nhấp nháy đèn
  if (switchState) return;
  
  if (!IsReady(lastTimer, 1000)) return;
  int prevLed = (currentLed + 2) % 3;
  digitalWrite(ledPin[prevLed], LOW);  
  digitalWrite(ledPin[currentLed], HIGH);  
  currentLed = (currentLed + 1) % 3;
}

void updateDHT() {
  static ulong lastTimer = 0;  
  if (!IsReady(lastTimer, 2000)) return;
  
  // Thay vì đọc từ cảm biến thật (có thể không hoạt động trong Wokwi), 
  // chúng ta sẽ sử dụng dữ liệu giả lập
  float h = random(0, 101) / 10.0; // 0.0 đến 10.0
  float t = random(150, 401) / 10.0; // 15.0 đến 40.0
  
  bool bDraw = false;
  if (fTemperature != t) {
    bDraw = true;
    fTemperature = t;
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    
    // Gửi nhiệt độ lên Blynk nếu đã kết nối
    if (blynkConnected) {
      Blynk.virtualWrite(VPIN_TEMPERATURE, t);
    }
  }
  
  if (fHumidity != h) {
    bDraw = true;
    fHumidity = h;
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");  
    
    // Gửi độ ẩm lên Blynk nếu đã kết nối
    if (blynkConnected) {
      Blynk.virtualWrite(VPIN_HUMIDITY, h);
    }
  }
  
  if (bDraw) {
    updateOLED();
  } 
}

void updateOperationTime() {
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 1000)) return;
  
  // Tính thời gian hoạt động từ khi khởi động
  currentOperationTime = millis() - operationStartTime;
  
  // Gửi thời gian hoạt động lên Blynk nếu đã kết nối
  if (blynkConnected) {
    Blynk.virtualWrite(VPIN_OPERATION_TIME, currentOperationTime / 1000);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nESP8266 DHT11 OLED Blynk Demo");
  
  // Khởi tạo các chân GPIO
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);
  
  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);
  
  // Khởi tạo DHT sensor
  dht.begin();
  
  // Khởi tạo màn hình OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();
  
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "IOT");  
  oled.drawUTF8(0, 28, "Connecting...");
  oled.drawUTF8(0, 42, "to WiFi");  
  oled.sendBuffer();
  
  // Kết nối WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi...");
  
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());
    
    // Kết nối đến Blynk
    connectToBlynk();
  } else {
    Serial.println("Failed to connect to WiFi");
    
    oled.clearBuffer();
    oled.drawUTF8(0, 14, "IOT");  
    oled.drawUTF8(0, 28, "WiFi");
    oled.drawUTF8(0, 42, "Failed");  
    oled.sendBuffer();
    delay(2000);
  }
  
  // Ghi nhận thời gian bắt đầu hoạt động
  operationStartTime = millis();
  
  // Hiển thị màn hình dữ liệu đầu tiên
  updateDHT();
}

void loop() {
  // Kiểm tra kết nối WiFi
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastWifiReconnectAttempt = 0;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastWifiReconnectAttempt > 30000) {  // Thử lại mỗi 30 giây
      lastWifiReconnectAttempt = currentMillis;
      Serial.println("WiFi disconnected. Reconnecting...");
      WiFi.disconnect();
      WiFi.begin(ssid, pass);
    }
    
    // Nếu WiFi đang kết nối lại, thì các chức năng khác vẫn chạy
  } else {
    // WiFi đã kết nối
    
    // Kiểm tra kết nối Blynk
    if (!blynkConnected) {
      connectToBlynk();
    } else {
      if (!Blynk.connected()) {
        Serial.println("Lost connection to Blynk");
        blynkConnected = false;
      } else {
        Blynk.run(); // Chạy Blynk
      }
    }
  }
  
  if (!WelcomeDisplayTimeout()) return;
  
  // Cập nhật nhiệt độ và độ ẩm
  updateDHT();
  
  // Cập nhật thời gian hoạt động
  updateOperationTime();
  
  // Nhấp nháy đèn LED (chỉ khi switch off)
  ThreeLedBlink();
}