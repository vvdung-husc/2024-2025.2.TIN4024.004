#include <Arduino.h>
#include <TM1637Display.h>

#define BLYNK_TEMPLATE_ID "TMPL6DNfYnwLr"
#define BLYNK_TEMPLATE_NAME "TRAFFICBLYNK"
#define BLYNK_AUTH_TOKEN "6MKNAytVfwkKa5dPUCugliQqzol2Na3U" 

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// Định nghĩa GPIO
#define gLED  25
#define yLED  26
#define rLED  27

#define btnBLED  23
#define pinBLED  21

#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22

// Cảm biến ánh sáng (LDR)
#define LDR_PIN 33
int LDR_THRESHOLD = 1000; // Giá trị ngưỡng ánh sáng (có thể chỉnh từ Blynk)

// Thời gian đèn 
uint rTIME = 5000;
uint yTIME = 3000;
uint gTIME = 6000;
uint blinkInterval = 500; // đọ trễ của nút bấm

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

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
  
  Serial.print("Receiving signal "); 
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Wifi is connected");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Blynk.virtualWrite(V4, LDR_THRESHOLD);

  Serial.println("START ==>");
  Serial.print("1. RED    => GREEN  "); 
  Serial.print(nextTimeTotal/1000); 
  Serial.println(" (s)");
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
    case rLED:
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        nextTimeTotal += gTIME;
        Serial.print("2. GREEN  => YELLOW "); 
        Serial.print(nextTimeTotal/1000); 
        Serial.println(" (s)");
      } 
      break;

    case gLED:
      if (IsReady(ledTimeStart, gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        Serial.print("3. YELLOW => RED "); 
        Serial.print(nextTimeTotal/1000); 
        Serial.println(" (s)");        
      }
      break;

    case yLED:
      if (IsReady(ledTimeStart, yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        Serial.print("1. RED    => GREEN  "); 
        Serial.print(nextTimeTotal/1000); 
        Serial.println(" (s)");        
      }
      break;
  }  
}

void NonBlocking_Traffic_Light_TM1637(){
  if (!isBlinking) {  
    remainingTime = (ledTimeStart + 
                    (currentLED == rLED ? rTIME : 
                     (currentLED == gLED ? gTIME : yTIME))
                    - currentMiliseconds) / 1000;

    display.showNumberDec(remainingTime, true);
  }
}
void Check_Light_Sensor() {
  int lightValue = analogRead(LDR_PIN);
  Blynk.virtualWrite(V4, lightValue); 

  if (lightValue < LDR_THRESHOLD) {
      if (!isBlinking) {
          isBlinking = true;
          blinkStartTime = millis();
          digitalWrite(rLED, LOW);
          digitalWrite(gLED, LOW);
          digitalWrite(yLED, LOW);
          display.clear();
          Serial.println("Dark mode is on");
      }
  } else {
      if (isBlinking) {
          isBlinking = false;
          digitalWrite(yLED, LOW);
          currentLED = rLED;
          ledTimeStart = millis();
          remainingTime = rTIME / 1000;
          display.showNumberDec(remainingTime);
          Serial.println("Dark mode is off");
      }
  }
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
  int dg = digitalRead(btnBLED);
  if (dg == lastValue) return;
  lastValue = dg;
  if (dg == LOW) return;

  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
}

void updateDHT() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Read temperature and humidity for error");
    return;
  }

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

BLYNK_WRITE(V4) {
  LDR_THRESHOLD = param.asInt();
  Serial.print("Updated LDR Threshold: "); Serial.println(LDR_THRESHOLD);
}

BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Serial.print("Updated Blue Button: "); Serial.println(blueButtonON);
}
