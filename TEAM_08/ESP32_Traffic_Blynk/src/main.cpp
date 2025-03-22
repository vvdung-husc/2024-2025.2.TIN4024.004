
//Lê Văn Bùi
#define BLYNK_TEMPLATE_ID "TMPL6EcyWZBb6"
#define BLYNK_TEMPLATE_NAME "Bui Le"
#define BLYNK_AUTH_TOKEN "waDCU5hEJF7fR0uJRETW6RcF-GCudH0s"

/* Lê Quang Vũ
#define BLYNK_TEMPLATE_ID "TMPL6-MCfoVVn"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
#define BLYNK_AUTH_TOKEN "cKdXp_mMqaZvmoGmAiMqRoSjqge9ZIDR"
*/

/*
//Nguyễn Đức Nhuận
#define BLYNK_TEMPLATE_ID "TMPL6R6i378Rq"
#define BLYNK_TEMPLATE_NAME "Traffic and DHT BanDemDoiMau"
#define BLYNK_AUTH_TOKEN "opLgjE7q8mUhNx5cFyasCUQTzuKBM3vW"
*/

//Lê Hữu Đông Quân
// #define BLYNK_TEMPLATE_ID "TMPL61-yqowgg"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "WXoOHSnkx8oZOHg_910tFxUleUMuhWz1"

//Đặng Ngọc Hoàng
// #define BLYNK_TEMPLATE_ID "TMPL6pL3Elp5X"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "LbculCQ3x3GFQe0UxCNHFvh2-DiKI_wK"


#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

//Pin
#define rLED  27
#define yLED  26
#define gLED  25

// Cảm biến ánh sáng (LDR)
#define LDR_PIN 32
int ldrThreshold = 1000;  // Mặc định 1000, có thể thay đổi từ Blynk


// Thời gian đèn
//1000 ms = 1 seconds
uint rTIME = 5000;   //5 seconds
uint yTIME = 3000;
uint gTIME = 7000;
uint blinkInterval = 500; // Thời gian nhấp nháy

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED  23
#define pinBLED  21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
ulong blinkStartTime = 0;
bool isBlinking = false;
bool yellowState = false;
bool blueButtonON = true;

int currentLED = rLED;
uint remainingTime = 0;

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();
void Check_Light_Sensor();
void Blink_Yellow_Light();
void updateBlueButton();
void uptimeBlynk();
void updateDHT();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  
  display.setBrightness(0x0f);
  dht.begin();

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);

  currentLED = rLED;
  nextTimeTotal += rTIME;
  remainingTime = rTIME / 1000;
  
  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("WiFi connected");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  
  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal/1000); Serial.println(" (ms)");
}

void loop() {  
  Blynk.run();
  currentMiliseconds = millis();
  Check_Light_Sensor();
  uptimeBlynk();
  updateBlueButton();
  updateDHT();

  if (isBlinking) {
    Blink_Yellow_Light();
  } else {
    NonBlocking_Traffic_Light();
    NonBlocking_Traffic_Light_TM1637();
  }
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void NonBlocking_Traffic_Light(){
  switch (currentLED) {
    case rLED: // Đèn đỏ: 5 giây
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        nextTimeTotal += gTIME;
        Serial.print("2. GREEN  => YELLOW "); Serial.print(nextTimeTotal/1000); Serial.println(" (ms)");
      } 
      break;

    case gLED: // Đèn xanh: 7 giây
      if (IsReady(ledTimeStart,gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        Serial.print("3. YELLOW => RED    "); Serial.print(nextTimeTotal/1000); Serial.println(" (ms)");        
      }
      break;

    case yLED: // Đèn vàng: 2 giây
      if (IsReady(ledTimeStart,yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal/1000); Serial.println(" (ms)");        
      }
      break;
  }  
}

void NonBlocking_Traffic_Light_TM1637(){
  if (!isBlinking && blueButtonON) {  // Chỉ hiển thị số khi nút xanh bật
    remainingTime = (ledTimeStart + 
                    (currentLED == rLED ? rTIME : 
                     (currentLED == gLED ? gTIME : yTIME))
                    - currentMiliseconds) / 1000;

    display.showNumberDec(remainingTime, true);
  } else {
    display.clear();  // Nếu Blue LED tắt, xóa số
  }
}

void Check_Light_Sensor() {
  int lightValue = analogRead(LDR_PIN);

  Blynk.virtualWrite(V4, ldrThreshold);  // Gửi giá trị cảm biến lên Blynk
  
  if (lightValue < ldrThreshold) {  // Sử dụng biến có thể thay đổi
      if (!isBlinking) {
          isBlinking = true;
          blinkStartTime = millis();
          digitalWrite(rLED, LOW);
          digitalWrite(gLED, LOW);
          digitalWrite(yLED, LOW);
          display.clear(); // Xoá số hiển thị khi vào chế độ nhấp nháy
          Serial.println("Low light detected - Blinking Yellow Mode");
      }
  } else {
      if (isBlinking) {
          isBlinking = false;
          digitalWrite(yLED, LOW);
          currentLED = rLED;
          ledTimeStart = millis();
          remainingTime = rTIME / 1000;
          display.showNumberDec(remainingTime);
          Serial.println("Light restored - Returning to normal mode");
      }
  }
}

// Hàm này nhận giá trị mới từ Blynk để cập nhật ngưỡng sáng
BLYNK_WRITE(V4) {
  ldrThreshold = param.asInt(); // Nhận giá trị ngưỡng từ Blynk
  Serial.print("🔄 Ngưỡng ánh sáng mới: ");
  Serial.println(ldrThreshold);
}


void Blink_Yellow_Light() {
  if (IsReady(blinkStartTime, blinkInterval)) {
    yellowState = !yellowState;
    digitalWrite(yLED, yellowState);
  }
  display.clear();
}

void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;  
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;  
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON) {  // Nếu bảng LED đang tắt -> BẬT
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  } else {  // Nếu bảng LED đang bật -> TẮT
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
  }  
}



void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (!isBlinking && currentLED == 0) { 
    display.showNumberDec(value);
  } 
}

void updateDHT() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("⚠️ Failed to read from DHT sensor! Retrying...");
    return;
  }

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}



BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  if (blueButtonON) {
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    
    // HIỂN THỊ LẠI SỐ GIÂY CỦA ĐÈN GIAO THÔNG
    if (!isBlinking) {  
      display.showNumberDec(remainingTime);
    }

  } else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);   
    display.clear();  // TẮT BẢNG LED
  }
}
