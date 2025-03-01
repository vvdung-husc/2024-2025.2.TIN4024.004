#include <Arduino.h>
#include <TM1637Display.h>

#define BLYNK_TEMPLATE_ID "TMPL68WAJ8Swo"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "csd6WGS69yFRnakxq0Jdmc47F8aV2sEn"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED 23
#define pinBLED 21
#define CLK 18
#define DIO 19
#define DHTPIN 16 // Chân kết nối DHT22
#define DHTTYPE DHT22

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

ulong currentMiliseconds = 0;
bool blueButtonON = true;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT();

void setup()
{
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  display.setBrightness(0x0f);
  dht.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Serial.println("== START ==>");
}

void loop()
{
  Blynk.run();
  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  readDHT();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
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
  Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  if (!blueButtonON)
    display.clear();
}

void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (blueButtonON)
    display.showNumberDec(value);
}

void readDHT()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000))
    return; // Đọc DHT mỗi 2 giây
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp) || isnan(hum))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Serial.print("Temperature: ");
  // Serial.print(temp);
  // Serial.print(" °C");
  // Serial.print("  Humidity: ");
  // Serial.print(hum);
  // Serial.println(" %");
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
}

BLYNK_WRITE(V1)
{
  blueButtonON = param.asInt();
  Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  if (!blueButtonON)
    display.clear();
}
