#define BLYNK_TEMPLATE_ID "TMPL6uis_oez2"
#define BLYNK_TEMPLATE_NAME "ESP32ThuyCanh"
#define BLYNK_AUTH_TOKEN "1-5VyH7lLgamJ3DEU_RPwXfcs_i7AG0R"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Thông tin WiFi
char ssid[] = "Wokwi-GUEST";        // ← Đổi tên WiFi
char pass[] = "";    // ← Đổi mật khẩu WiFi

// Cảm biến DHT22
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Chân cảm biến và thiết bị
#define LIGHT_PIN 34
#define PH_PIN 35

#define LED_LIGHT_WARN 2    // Cảnh báo ánh sáng yếu (LED đỏ)
#define LED_PH_WARN 4       // Cảnh báo pH bất thường (LED đỏ)

#define LED_GROW_LIGHT 16   // Đèn trồng cây (điều khiển qua V6)
#define PUMP_PIN 17         // Bơm nước (điều khiển qua V7)

void setup() {
  Serial.begin(115200);

  pinMode(LED_LIGHT_WARN, OUTPUT);
  pinMode(LED_PH_WARN, OUTPUT);
  pinMode(LED_GROW_LIGHT, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  Blynk.run();

  // Đọc cảm biến
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  int light = analogRead(LIGHT_PIN);      // 0 - 4095
  int rawPH = analogRead(PH_PIN);         // 0 - 4095
  float phValue = map(rawPH, 0, 4095, 0, 14); // giả lập

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, humid);
  Blynk.virtualWrite(V2, light);
  Blynk.virtualWrite(V3, phValue);

  // CẢNH BÁO ÁNH SÁNG (V4)
  if (light < 1500) {
    digitalWrite(LED_LIGHT_WARN, HIGH);  // LED đỏ thật
    Blynk.setProperty(V4, "color", "#FF0000");
  } else {
    digitalWrite(LED_LIGHT_WARN, LOW);
    Blynk.setProperty(V4, "color", "#00FF00");
  }
  Blynk.virtualWrite(V4, 255); // luôn bật LED ảo

  // CẢNH BÁO pH (V5)
  if (phValue < 5.5 || phValue > 6.5) {
    digitalWrite(LED_PH_WARN, HIGH);
    Blynk.setProperty(V5, "color", "#FF0000");
  } else {
    digitalWrite(LED_PH_WARN, LOW);
    Blynk.setProperty(V5, "color", "#00FF00");
  }
  Blynk.virtualWrite(V5, 255);

  delay(2000);
}

// SWITCH ĐÈN TRỒNG CÂY
BLYNK_WRITE(V6) {
  int state = param.asInt();
  digitalWrite(LED_GROW_LIGHT, state);
}

// SWITCH BƠM NƯỚC
BLYNK_WRITE(V7) {
  int state = param.asInt();
  digitalWrite(PUMP_PIN, state);
}
