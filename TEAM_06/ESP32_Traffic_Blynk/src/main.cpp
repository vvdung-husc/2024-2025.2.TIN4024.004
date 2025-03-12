//Trong
#define BLYNK_TEMPLATE_ID "TMPL6j37JFNQC"
#define BLYNK_TEMPLATE_NAME "ESP32 Blynk LED"
#define BLYNK_AUTH_TOKEN "72PWXm4riLyYaEzU-8e5-wh5WJh7OxiE"
//Tue
// #define BLYNK_TEMPLATE_ID "TMPL6W2ZuQ3Vs"
// #define BLYNK_TEMPLATE_NAME "ESP32 Blynk LED"
// #define BLYNK_AUTH_TOKEN "VlSDmhqoC5PRF5qza-WwZPx4BB7TSbzA"
// //Thanh
// #define BLYNK_TEMPLATE_ID "TMPL6L7gW1LVm"
// #define BLYNK_TEMPLATE_NAME "ESP32 BLYNK TRAFFIC AND DHT SENSOR"
// #define BLYNK_AUTH_TOKEN "bgNpzWSld20tBSqenKBNwmKFkbiq32L
//Viet
// #define BLYNK_TEMPLATE_ID "TMPL6C4XcVTsM"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "jEExrLPInYHy-eF4NzAiOpG1vWNeEhTC"
// //Hoang
// #define BLYNK_TEMPLATE_ID "TMPL6L580oKYQ"
// #define BLYNK_TEMPLATE_NAME "ESP32 Blynk LED"
// #define BLYNK_AUTH_TOKEN "q_aGCPUEtd2P1-7IKOHlYxOlwldQ6Oq8"
#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED  23
#define pinBLED  21
#define RED_LED_PIN 27
#define YELLOW_LED_PIN 26
#define GREEN_LED_PIN 25
#define CLK 18
#define DIO 19
#define DHT_PIN 16

const unsigned long redInterval = 5000;
const unsigned long yellowInterval = 3000;
const unsigned long greenInterval = 7000;
const int LDR_NIGHT_THRESHOLD = 1000;

unsigned long currentMillis = 0;
bool blueButtonON = false;
unsigned long previousMillisTraffic = 0;
int currentLED = 2;
unsigned long countdownTime = greenInterval;
bool isNightMode = false;
unsigned long previousMillisBlink = 0;
bool yellowBlinkState = false;
int ldrValue = 1000;

TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT22);

bool IsReady(unsigned long &ulTimer, uint32_t milisecond);
void updateBlueButton();
void updateTrafficLights();
void updateLDR();
void updateDHT();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(pinBLED, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);
  display.setBrightness(0x0f);
  dht.begin();
  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");
  Blynk.virtualWrite(V1, blueButtonON);
  Blynk.virtualWrite(V4, ldrValue);
  Serial.println("== START ==>");
}

void loop() {
  Blynk.run();
  currentMillis = millis();
  unsigned long runningTime = currentMillis / 1000;
  Blynk.virtualWrite(V0, runningTime);
  Serial.print("Running Time: ");
  Serial.println(runningTime);
  updateBlueButton();
  updateLDR();
  updateTrafficLights();
  updateDHT();
}

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMillis - ulTimer < milisecond) return false;
  ulTimer = currentMillis;
  return true;
}

void updateBlueButton() {
  static unsigned long lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;
  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
  if (!blueButtonON) {
    display.clear();
  }
}

void updateTrafficLights() {
  if (isNightMode) {
    if (IsReady(previousMillisBlink, 500)) {
      yellowBlinkState = !yellowBlinkState;
      digitalWrite(YELLOW_LED_PIN, yellowBlinkState ? HIGH : LOW);
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      if (blueButtonON) {
        display.showNumberDec(0);
      }
      Serial.println("Night mode: Yellow LED blinking");
    }
  } else {
    int timeRemaining = (countdownTime - (currentMillis - previousMillisTraffic)) / 1000;
    if (blueButtonON && timeRemaining >= 0) {
      display.showNumberDecEx(timeRemaining, 0b01000000, true);
    }
    if ((currentLED == 0 && currentMillis - previousMillisTraffic >= redInterval) ||
        (currentLED == 1 && currentMillis - previousMillisTraffic >= yellowInterval) ||
        (currentLED == 2 && currentMillis - previousMillisTraffic >= greenInterval)) {
      previousMillisTraffic = currentMillis;
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      if (currentLED == 0) {
        digitalWrite(GREEN_LED_PIN, HIGH);
        currentLED = 2;
        countdownTime = greenInterval;
        Serial.println("RED => GREEN");
      } else if (currentLED == 1) {
        digitalWrite(RED_LED_PIN, HIGH);
        currentLED = 0;
        countdownTime = redInterval;
        Serial.println("YELLOW => RED");
      } else if (currentLED == 2) {
        digitalWrite(YELLOW_LED_PIN, HIGH);
        currentLED = 1;
        countdownTime = yellowInterval;
        Serial.println("GREEN => YELLOW");
      }
    }
  }
}

void updateLDR() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  Blynk.virtualWrite(V4, ldrValue);
  Serial.print("LDR Virtual Value: ");
  Serial.println(ldrValue);
  isNightMode = (ldrValue < LDR_NIGHT_THRESHOLD);
  if (isNightMode) {
    Serial.println("Night mode activated");
  } else {
    Serial.println("Day mode activated");
  }
}

void updateDHT() {
  static unsigned long lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT22 sensor!");
    return;
  }
  Blynk.virtualWrite(V3, temperature);
  Blynk.virtualWrite(V2, humidity);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C | Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
}

BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
  if (!blueButtonON) {
    display.clear();
  }
}

BLYNK_WRITE(V4) {
  ldrValue = param.asInt();
  Serial.print("LDR Virtual Value updated from Blynk: ");
  Serial.println(ldrValue);
}
