#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6U-bgV0v4"
#define BLYNK_TEMPLATE_NAME "PP"
#define BLYNK_AUTH_TOKEN "XW-o3nfdrKUhSdxUDbpj38lycdu9GTXr"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED  23 // Chân kết nối nút bấm
#define pinBLED  21 // Chân kết nối đèn xanh
#define CLK 18  // Chân kết nối CLK của TM1637
#define DIO 19  // Chân kết nối DIO của TM1637
#define DHTPIN 16 // Chân kết nối cảm biến DHT22
#define DHTTYPE DHT22

// Biến toàn cục
ulong currentMiliseconds = 0;
bool blueButtonON = true;

// Khởi tạo màn hình TM1637 và cảm biến DHT22
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT22();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();

  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Serial.println("== START ==>");
}

void loop() {
  Blynk.run();
  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  readDHT22();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

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

  if (!blueButtonON) {
    display.clear();
  }
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (blueButtonON) {
    display.showNumberDec(value);
  }
}

void readDHT22() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON) {
    display.clear();
  }
}
