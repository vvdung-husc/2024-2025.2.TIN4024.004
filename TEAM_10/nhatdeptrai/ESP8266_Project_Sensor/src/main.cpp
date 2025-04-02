#include <Arduino.h>
#include <utils.h>  // Chỉ khai báo, không định nghĩa lại IsReady()

#include <Adafruit_Sensor.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>

//Trần Văn Minh Nhật
#define BLYNK_TEMPLATE_ID "TMPL6rKfVu6HP"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "NVohCxt35pdmPIROGthShHSki6xWgVdf"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "CNTT-MTT";
char pass[] = "13572468";

// Định nghĩa chân GPIO
#define btnBLED D3  
#define pinBLED D4  
#define rLED D5
#define yLED D6
#define gLED D7
#define CLK D1
#define DIO D2
#define DHTPIN D0
#define DHTTYPE DHT22
#define ldrPIN A0  

// Thời gian LED
const uint rTIME = 5000;
const uint yTIME = 3000;
const uint gTIME = 10000;

ulong currentMillis = 0;
ulong ledTimeStart = 0;
int currentLED = rLED;
int tmCounter = rTIME / 1000;
bool blueButtonON = true;
int darkThreshold = 1000;

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

void updateBlueButton();
void uptimeBlynk();
void readDHTSensor();
void NonBlocking_Traffic_Light_TM1637();
bool isDark();
void YellowLED_Blink();

void setup()
{
  Serial.begin(115200);
  
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(ldrPIN, INPUT);

  display.setBrightness(7);
  dht.begin(); // Khởi động cảm biến DHT22

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  display.showNumberDec(tmCounter--, true, 2, 2);
}

void loop()
{
  Blynk.run();
  currentMillis = millis();

  uptimeBlynk();
  updateBlueButton();
  readDHTSensor();

  if (isDark())
    YellowLED_Blink();
  else
    NonBlocking_Traffic_Light_TM1637();
}

void NonBlocking_Traffic_Light_TM1637()
{
  if (!blueButtonON) return;

  if (IsReady(ledTimeStart, (currentLED == rLED ? rTIME : (currentLED == gLED ? gTIME : yTIME))))
  {
    digitalWrite(currentLED, LOW);
    if (currentLED == rLED)
      currentLED = gLED;
    else if (currentLED == gLED)
      currentLED = yLED;
    else
      currentLED = rLED;
    
    digitalWrite(currentLED, HIGH);
    tmCounter = (currentLED == rLED ? rTIME : (currentLED == gLED ? gTIME : yTIME)) / 1000 - 1;
  }
  
  if (IsReady(ledTimeStart, 1000))
    display.showNumberDec(tmCounter--, true, 2, 2);
}

bool isDark()
{
  static ulong darkTimeStart = 0;
  static uint16_t lastValue = 0;
  static bool bDark = false;

  if (!IsReady(darkTimeStart, 50)) return bDark;

  uint16_t value = analogRead(ldrPIN);
  if (value == lastValue) return bDark;

  if (value < darkThreshold)
  {
    if (!bDark && blueButtonON) digitalWrite(currentLED, LOW);
    bDark = blueButtonON;
  }
  else
  {
    if (bDark) digitalWrite(yLED, LOW);
    bDark = false;
  }
  lastValue = value;
  return bDark;
}

void YellowLED_Blink()
{
  if (!blueButtonON) return;

  static ulong yLedStart = 0;
  static bool isON = false;

  if (!IsReady(yLedStart, 1000)) return;
  digitalWrite(yLED, isON ? LOW : HIGH);
  isON = !isON;
}

void updateBlueButton()
{
  static ulong lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50)) return;

  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON;

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON)
  {
    digitalWrite(rLED, LOW);
    digitalWrite(yLED, LOW);
    digitalWrite(gLED, LOW);
    display.clear();
  }
  Blynk.virtualWrite(V1, blueButtonON);
}

void uptimeBlynk()
{
  static ulong lastTime = 0;
  static ulong runTime = 0;

  if (!IsReady(lastTime, 1000)) return;

  if (blueButtonON)
  {
    runTime++;
    Blynk.virtualWrite(V0, runTime);
  }
}