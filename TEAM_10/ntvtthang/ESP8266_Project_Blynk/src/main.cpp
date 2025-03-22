#include <Arduino.h>
#include <utils.h>  // Chỉ khai báo, không định nghĩa lại IsReady()

#include <Adafruit_Sensor.h>
#include <TM1637Display.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>

// Nguyễn Trần Viết Thắng
#define BLYNK_TEMPLATE_ID "TMPL6RbRbfIUd"
#define BLYNK_TEMPLATE_NAME "ESP8266ProjectBlynk"
#define BLYNK_AUTH_TOKEN "ZJX_hztr1EMBQuavBSDaPxjqVjkgjTw6"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

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
uint rTIME = 5000;
uint yTIME = 3000;
uint gTIME = 10000;

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = 0;
int tmCounter = rTIME / 1000;
ulong counterTime = 0;

bool blueButtonON = true;
ulong ledOnTime = 0;
int darkThreshold = 1000;

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

void updateBlueButton();
void uptimeBlynk();
void readDHTSensor();
void updateLedTime();
void NonBlocking_Traffic_Light_TM1637();
bool isDark();
void YellowLED_Blink();

void setup()
{
  Serial.begin(115200);
  
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin(); // Khởi động cảm biến DHT22

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(ldrPIN, INPUT);

  tmCounter = (rTIME / 1000) - 1;
  display.setBrightness(7);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  display.showNumberDec(tmCounter--, true, 2, 2);

  currentLED = rLED;
  nextTimeTotal += rTIME;
}

void loop()
{
  Blynk.run();
  currentMiliseconds = millis();

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

  bool bShowCounter = false;
  switch (currentLED)
  {
  case rLED:
    if (IsReady(ledTimeStart, rTIME))
    {
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, HIGH);
      currentLED = gLED;
      nextTimeTotal += gTIME;
      tmCounter = (gTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
    }
    break;
  case gLED:
    if (IsReady(ledTimeStart, gTIME))
    {
      digitalWrite(gLED, LOW);
      digitalWrite(yLED, HIGH);
      currentLED = yLED;
      nextTimeTotal += yTIME;
      tmCounter = (yTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
    }
    break;
  case yLED:
    if (IsReady(ledTimeStart, yTIME))
    {
      digitalWrite(yLED, LOW);
      digitalWrite(rLED, HIGH);
      currentLED = rLED;
      nextTimeTotal += rTIME;
      tmCounter = (rTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
    }
    break;
  }
  if (!bShowCounter)
    bShowCounter = IsReady(counterTime, 1000);
  if (bShowCounter)
  {
    display.showNumberDec(tmCounter--, true, 2, 2);
  }
}

bool isDark()
{
  static ulong darkTimeStart = 0;
  static uint16_t lastValue = 0;
  static bool bDark = false;

  if (!IsReady(darkTimeStart, 50))
    return bDark;
  uint16_t value = analogRead(ldrPIN);
  if (value == lastValue)
    return bDark;

  if (value < darkThreshold)
  {
    if (!bDark && blueButtonON)
    {
      digitalWrite(currentLED, LOW);
    }
    bDark = blueButtonON;
  }
  else
  {
    if (bDark)
    {
      digitalWrite(yLED, LOW);
    }
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

  if (!IsReady(yLedStart, 1000))
    return;
  digitalWrite(yLED, isON ? LOW : HIGH);
  isON = !isON;
}

void updateBlueButton()
{
  static ulong lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50))
    return;

  int v = digitalRead(btnBLED);
  if (v == lastValue)
    return;
  lastValue = v;
  if (v == LOW)
    return;

  blueButtonON = !blueButtonON;

  if (blueButtonON)
  {
    digitalWrite(pinBLED, HIGH);
  }
  else
  {
    digitalWrite(pinBLED, LOW);
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

  if (!IsReady(lastTime, 1000))
    return;

  if (blueButtonON)
  {
    runTime++;
    Blynk.virtualWrite(V0, runTime);
  }
}
