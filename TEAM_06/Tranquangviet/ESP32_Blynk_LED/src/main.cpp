#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6C4XcVTsM"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "jEExrLPInYHy-eF4NzAiOpG1vWNeEhTC"

// WiFi credentials
char ssid[] = "Wokwi-GUEST";  
char pass[] = "";            

// Define pins
#define btnBLED  23  // Button
#define pinBLED  21  // LED
#define CLK 18       // 7-Segment Display Clock
#define DIO 19       // 7-Segment Display Data
#define DHTPIN 16    // DHT22 Sensor Data pin
#define DHTTYPE DHT22

// Global variables
ulong currentMiliseconds = 0;
bool blueButtonON = true;
float nhietDo = 0, doAm = 0;

// Initialize TM1637 display
TM1637Display display(CLK, DIO);

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void hienThiThongTin();
void docDHT22();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  dht.begin();
  
  // WiFi & Blynk connection
  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("\nWiFi connected!");
  
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON);  

  Serial.println("== START SYSTEM ==");
}

void loop() {  
  Blynk.run();  

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  docDHT22();
  hienThiThongTin();
}

// Check if the timer is ready
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (millis() - ulTimer < milisecond) return false;
  ulTimer = millis();
  return true;
}

// Handle button press for LED control
void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  
  if (!IsReady(lastTime, 50)) return;
  
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  
  if (v == LOW) return;

  blueButtonON = !blueButtonON;
  
  Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  if (!blueButtonON) display.clear();
}

// Update uptime to Blynk
void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  
  ulong uptime = millis() / 1000;
  Blynk.virtualWrite(V0, uptime);
}

// Read temperature and humidity from DHT22
void docDHT22() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  nhietDo = dht.readTemperature();
  doAm = dht.readHumidity();

  if (isnan(nhietDo) || isnan(doAm)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temp: "); Serial.print(nhietDo);
  Serial.print("°C, Humidity: "); Serial.print(doAm);
  Serial.println("%");

  Blynk.virtualWrite(V2, nhietDo);
  Blynk.virtualWrite(V3, doAm);
}

// Display temperature and humidity on TM1637
void hienThiThongTin() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return; 

  if (blueButtonON) {
    Serial.print("Displaying Temperature: "); Serial.println(nhietDo);
    display.showNumberDec(nhietDo);
    delay(2000);
    
    Serial.print("Displaying Humidity: "); Serial.println(doAm);
    display.showNumberDec(doAm);
    delay(2000);
  }
}

// Control LED from Blynk
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  
  Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

  if (!blueButtonON) display.clear();
}
