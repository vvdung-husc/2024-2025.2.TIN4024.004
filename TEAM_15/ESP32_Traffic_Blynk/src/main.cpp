#define BLYNK_PRINT Serial
#include <Arduino.h>
#include <TM1637Display.h>

// Blynk Authentication
// Dương Thị Thu Uyên - 21T1020099
// #define BLYNK_TEMPLATE_ID "TMPL6oG3Nj8wA"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "HURYMOI5FgZO17csG7YVZ4HtJRk7Zm3z"

// Lê Trung Tín - 21T1020086
// #define BLYNK_TEMPLATE_ID "TMPL68WAJ8Swo"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "csd6WGS69yFRnakxq0Jdmc47F8aV2sEn"

// Hoàng Ngọc Bảo Minh -21T1020140
// #define BLYNK_TEMPLATE_ID "TMPL6_s4gxZGU"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "aQGBGdJcaKBcCaIffVGqvrBWit6R62aC"

//Nguyễn Anh Trí -21T1020772
// #define BLYNK_TEMPLATE_ID "TMPL60LJI06BG"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "0P0pw_Qgr8RObz-yaO3EB5JAc0yWxh-u"

// Trần Quốc Hưng - 21T1020403
// #define BLYNK_TEMPLATE_ID "TMPL6zx9_fQlK"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "hSeNSB8cA_yCm_WNtTMtKOZj4kUIPd0P"


char ssid[] = "Wokwi-GUEST";
char pass[] = "";
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
// Pin Definitions
#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25
#define BLUE_LED 21
#define BUTTON_PIN 23
#define LDR_PIN 32
#define CLK 18
#define DIO 19
#define DHT_PIN 16
#define DHT_TYPE DHT22

// TM1637 Display
TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

// Traffic Light Timings
const uint32_t RED_TIME = 7000;
const uint32_t GREEN_TIME = 5000;
const uint32_t YELLOW_TIME = 3000;

// State Variables
uint32_t lastMillis = 0, countdown = 0, lastButtonPress = 0;
bool displayEnabled = true, blinkingMode = false;
int currentLED = RED_LED;
int lightThreshold = 1000; // Adjustable via Blynk
uint32_t runtimeSeconds = 0;

// Function Declarations
void setTrafficLight(int led, uint32_t time);
void checkLightSensor();
void enableBlinkingMode();
void disableBlinkingMode();
void updateCountdown();
void switchTrafficLight();
void handleButtonPress();
void sendSensorData();
void updateRuntime(); // Khai báo hàm trước khi sử dụng

void setup()
{
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "128.199.144.129", 80);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  display.setBrightness(7);
  dht.begin();

  setTrafficLight(RED_LED, RED_TIME);
}

void loop()
{
  Blynk.run();
  checkLightSensor();
  handleButtonPress();
  if (!blinkingMode)
  {
    updateCountdown();
    switchTrafficLight();
  }
  sendSensorData();
  updateRuntime(); // Cập nhật thời gian chạy
}

void updateRuntime()
{
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate >= 1000) // Cập nhật mỗi giây
  {
    lastUpdate = millis();
    runtimeSeconds++;                       // Tăng thời gian chạy lên 1 giây
    Blynk.virtualWrite(V0, runtimeSeconds); // Gửi giá trị lên Blynk
  }
}

void checkLightSensor()
{
  static uint32_t lightCheckMillis = 0;
  if (millis() - lightCheckMillis < 500)
    return;
  lightCheckMillis = millis();

  int lightLevel = analogRead(LDR_PIN);
  if (lightLevel >= lightThreshold)
  {
    if (!blinkingMode)
      Serial.println("Trời tối, chuyển sang chế độ nhấp nháy!");
    enableBlinkingMode();
  }
  else if (blinkingMode)
  {
    Serial.println("Trời sáng, trở lại chế độ bình thường!");
    disableBlinkingMode();
  }
}

void enableBlinkingMode()
{
  blinkingMode = true;
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  display.clear();
  digitalWrite(YELLOW_LED, millis() % 1000 < 500 ? HIGH : LOW);
}

void disableBlinkingMode()
{
  blinkingMode = false;
  setTrafficLight(RED_LED, RED_TIME);
}

void setTrafficLight(int led, uint32_t time)
{
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(led, HIGH);

  currentLED = led;
  countdown = time / 1000;
  lastMillis = millis();

  if (displayEnabled)
  {
    display.showNumberDec(countdown);
    digitalWrite(BLUE_LED, HIGH);
  }
  else
  {
    display.clear();
    digitalWrite(BLUE_LED, LOW);
  }
}

void updateCountdown()
{
  if (millis() - lastMillis >= 1000)
  {
    lastMillis += 1000;
    countdown--;

    if (displayEnabled)
      display.showNumberDec(countdown);
  }
}

void switchTrafficLight()
{
  if (countdown == 0)
  {
    if (currentLED == RED_LED)
      setTrafficLight(GREEN_LED, GREEN_TIME);
    else if (currentLED == GREEN_LED)
      setTrafficLight(YELLOW_LED, YELLOW_TIME);
    else if (currentLED == YELLOW_LED)
      setTrafficLight(RED_LED, RED_TIME);
  }
}

void handleButtonPress()
{
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW && lastButtonState == HIGH && millis() - lastButtonPress > 200)
  {
    displayEnabled = !displayEnabled;
    lastButtonPress = millis();

    if (displayEnabled)
    {
      Serial.println("Bảng đếm lùi: BẬT");
      display.showNumberDec(countdown);
      digitalWrite(BLUE_LED, HIGH);
    }
    else
    {
      Serial.println("Bảng đếm lùi: TẮT");
      display.clear();
      digitalWrite(BLUE_LED, LOW);
    }
    Blynk.virtualWrite(V1, displayEnabled);
  }
  lastButtonState = buttonState;
}

void sendSensorData()
{
  static uint32_t lastSensorUpdate = 0;
  if (millis() - lastSensorUpdate < 2000)
    return;
  lastSensorUpdate = millis();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidity))
  {
    Blynk.virtualWrite(V2, temperature);
    Blynk.virtualWrite(V3, humidity);
  }
}

BLYNK_WRITE(V4)
{
  lightThreshold = param.asInt();
  Serial.print("Ngưỡng ánh sáng mới: ");
  Serial.println(lightThreshold);
}

BLYNK_WRITE(V1)
{
  displayEnabled = param.asInt(); // Nhận trạng thái từ Blynk

  if (displayEnabled)
  {
    Serial.println("Bảng đếm lùi: BẬT");
    display.showNumberDec(countdown);
    digitalWrite(BLUE_LED, HIGH);
  }
  else
  {
    Serial.println("Bảng đếm lùi: TẮT");
    display.clear();
    digitalWrite(BLUE_LED, LOW);
  }
}
