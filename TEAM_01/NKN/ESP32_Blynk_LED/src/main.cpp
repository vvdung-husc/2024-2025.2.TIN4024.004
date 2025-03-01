#define BLYNK_TEMPLATE_ID "TMPL6GJTI7LIN"
#define BLYNK_TEMPLATE_NAME "NKN"
#define BLYNK_AUTH_TOKEN "KZ1xq6-gQmQwZj-DllHQpLWY8oDNATIt"

#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"


char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED  23
#define pinBLED  21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22

ulong currentMiliseconds = 0;
bool blueButtonON = true;

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void updateDHT();

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
  updateDHT();
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

  if (!blueButtonON) {
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  } else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
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

void updateDHT() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("⚠️ Failed to read from DHT sensor! Retrying...");
    return;
  }

  Serial.print("🌡️ Temperature: "); Serial.print(temperature); Serial.print("°C");
  Serial.print(" | 💧 Humidity: "); Serial.print(humidity); Serial.println("%");

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}




BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  if (blueButtonON) {
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  } else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);   
    display.clear(); 
  }
}