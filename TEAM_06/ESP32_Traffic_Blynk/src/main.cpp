#define BLYNK_TEMPLATE_ID "TMPL6W2ZuQ3Vs"
#define BLYNK_TEMPLATE_NAME "ESP32 Blynk LED"
#define BLYNK_AUTH_TOKEN "VlSDmhqoC5PRF5qza-WwZPx4BB7TSbzA"

#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Chân kết nối phần cứng
#define btnBLED  23
#define pinBLED  21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22
#define LDR_PIN 34  // Chân kết nối cảm biến ánh sáng

#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27

bool blueButtonON = true;
ulong currentMiliseconds = 0;
bool trafficLightMode = true;
int darkThreshold = 500;  // Mặc định ngưỡng tối là 500 lux

// Khởi tạo các đối tượng
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

// Biến đếm thời gian đèn giao thông
int trafficCountdown = 5;
int trafficState = 0; // 0: Xanh, 1: Vàng, 2: Đỏ

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT22();
void updateTrafficLight();
void checkLightSensor();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  display.setBrightness(0x0f);
  
  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");

  dht.begin();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  Serial.println("== START ==");
}

void loop() {
  Blynk.run();
  currentMiliseconds = millis();

  uptimeBlynk();
  updateBlueButton();
  readDHT22();
  checkLightSensor();
  updateTrafficLight();
}

// Hàm kiểm tra thời gian
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Hàm cập nhật trạng thái nút bấm
void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  if (!blueButtonON) display.clear();
}

// Hàm cập nhật thời gian chạy trên Blynk
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (blueButtonON) display.showNumberDec(value);
}

// Hàm đọc cảm biến DHT22
void readDHT22() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
  Serial.printf("Temperature: %.2f *C, Humidity: %.2f %%\n", temperature, humidity);
}

// Hàm kiểm tra cảm biến ánh sáng
void checkLightSensor() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  int ldrValue = analogRead(LDR_PIN);
  float illumination = (ldrValue / 4095.0) * 3300; // Quy đổi giá trị LDR thành lux giả định

  Serial.printf("Illumination: %.2f lux | Dark Threshold: %d lux\n", illumination, darkThreshold);

  if (illumination <= darkThreshold) {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(LED_YELLOW, LOW);
  } else {
    trafficLightMode = true;
  }
}

// Hàm cập nhật trạng thái đèn giao thông
void updateTrafficLight() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000) || !trafficLightMode) return;

  trafficCountdown--;

  if (trafficCountdown < 0) {
    trafficState = (trafficState + 1) % 3;
    if (trafficState == 0) trafficCountdown = 5; // Xanh 5s
    else if (trafficState == 1) trafficCountdown = 3; // Vàng 3s
    else if (trafficState == 2) trafficCountdown = 5; // Đỏ 5s
  }

  // Cập nhật đèn tương ứng
  digitalWrite(LED_GREEN, trafficState == 0 ? HIGH : LOW);
  digitalWrite(LED_YELLOW, trafficState == 1 ? HIGH : LOW);
  digitalWrite(LED_RED, trafficState == 2 ? HIGH : LOW);

  // Hiển thị số giây còn lại trên màn hình 7 đoạn
  display.showNumberDec(trafficCountdown);
}

// Xử lý sự kiện từ Blynk
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON) display.clear();
}

// Nhận giá trị **Dark Threshold** từ Blynk qua **V4**
BLYNK_WRITE(V4) {
  darkThreshold = param.asInt();  // Cập nhật giá trị ngưỡng tối từ Blynk
  Serial.printf("Updated Dark Threshold: %d lux\n", darkThreshold);
}
