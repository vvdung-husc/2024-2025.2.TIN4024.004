// Đinh Viết Tín
#define BLYNK_TEMPLATE_ID "TMPL6Apebb-Pq"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
#define BLYNK_AUTH_TOKEN "h4lqrwEQAXA1nea9m-39iH4VCYMcrpTL"

// Nguyễn Thanh Bạch
// #define BLYNK_TEMPLATE_ID "TMPL6DNfYnwLr"
// #define BLYNK_TEMPLATE_NAME "TRAFFICBLYNK"
// #define BLYNK_AUTH_TOKEN "6MKNAytVfwkKa5dPUCugliQqzol2Na3U" 

#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Cấu hình WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Định nghĩa chân
#define PIN_GREEN_LED 25
#define PIN_YELLOW_LED 26
#define PIN_RED_LED 27
#define PIN_BLUE_BUTTON 23
#define PIN_BLUE_LED 21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22
#define PIN_LIGHT_SENSOR 33

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

ulong currentMillis = 0;
bool blueButtonON = true;
bool isNightMode = false;

enum TrafficState
{
  GREEN,
  YELLOW,
  RED
};
TrafficState currentState = GREEN;

// Thời gian sáng của mỗi đèn
const int GREEN_TIME = 10;
const int YELLOW_TIME = 3;
const int RED_TIME = 5;

int timeLeft = GREEN_TIME;
float darkThreshold = 1000; // Ngưỡng mặc định sáng/tối

// Khai báo hàm
bool IsReady(ulong &timer, uint32_t interval);
void updateBlueButton();
void uptimeBlynk();
void readDHT();
void controlLights();
void updateDisplay();
void checkLight();

void setup()
{
  pinMode(PIN_GREEN_LED, OUTPUT);
  pinMode(PIN_YELLOW_LED, OUTPUT);
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_BLUE_LED, OUTPUT);
  pinMode(PIN_BLUE_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LIGHT_SENSOR, INPUT);

  Serial.begin(115200);
  display.setBrightness(0x0f);
  dht.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("WiFi connected");

  digitalWrite(PIN_BLUE_LED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
  Serial.println("== START ==");
}

void loop()
{
  Blynk.run();
  currentMillis = millis();
  uptimeBlynk();
  updateBlueButton();
  readDHT();
  checkLight();
  controlLights();
  updateDisplay();
}

// Kiểm tra thời gian
bool IsReady(ulong &timer, uint32_t interval)
{
  if (currentMillis - timer < interval)
    return false;
  timer = currentMillis;
  return true;
}

// Xử lý nút nhấn bật/tắt đèn LED xanh
void updateBlueButton()
{
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50))
    return;

  int v = digitalRead(PIN_BLUE_BUTTON);
  if (v == lastValue)
    return;
  lastValue = v;
  if (v == LOW)
    return;

  blueButtonON = !blueButtonON;
  Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
  digitalWrite(PIN_BLUE_LED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);
}

// Cập nhật số giây chạy lên Blynk
void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return;
  Blynk.virtualWrite(V0, lastTime / 1000);
}

// Đọc dữ liệu từ cảm biến DHT
void readDHT()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 500))
    return;
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp) || isnan(hum))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
}

// Điều khiển đèn từ Blynk
BLYNK_WRITE(V1)
{
  blueButtonON = param.asInt();
  Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
  digitalWrite(PIN_BLUE_LED, blueButtonON ? HIGH : LOW);
}

BLYNK_WRITE(V4)
{
  darkThreshold = param.asFloat();
  Serial.print("Cập nhật ngưỡng trời tối từ Blynk: ");
  Serial.println(darkThreshold);
  Blynk.virtualWrite(V4, darkThreshold);
  checkLight();
}

// Điều khiển đèn giao thông
void controlLights()
{
  static ulong lastUpdate = 0;
  if (!IsReady(lastUpdate, 500))
    return;

  if (isNightMode)
  {
    // Ban đêm: chỉ bật đèn vàng nhấp nháy
    digitalWrite(PIN_GREEN_LED, LOW);
    digitalWrite(PIN_YELLOW_LED, HIGH);
    digitalWrite(PIN_RED_LED, LOW);
    return;
  }
  else
  {
    digitalWrite(PIN_YELLOW_LED, LOW); // Đảm bảo tắt đèn vàng khi vào chế độ ngày
  }

  // Ban ngày: chạy hệ thống đèn giao thông bình thường
  timeLeft--;
  if (timeLeft <= 0)
  {
    switch (currentState)
    {
    case GREEN:
      currentState = YELLOW;
      timeLeft = YELLOW_TIME;
      break;
    case YELLOW:
      currentState = RED;
      timeLeft = RED_TIME;
      break;
    case RED:
      currentState = GREEN;
      timeLeft = GREEN_TIME;
      break;
    }
  }

  digitalWrite(PIN_GREEN_LED, currentState == GREEN);
  digitalWrite(PIN_YELLOW_LED, currentState == YELLOW);
  digitalWrite(PIN_RED_LED, currentState == RED);
}

// Hiển thị số giây lên màn hình
void updateDisplay()
{
  static ulong lastUpdate = 0;
  if (!IsReady(lastUpdate, 500))
    return;

  if (isNightMode)
  {
    // Ban đêm: hiển thị số 0 và dấu ":" nhấp nháy
    display.showNumberDecEx(0, 0b01000000, true, 2, 2);
  }
  else
  {
    // Ban ngày: hiển thị thời gian đếm ngược
    display.showNumberDec(timeLeft % 100, true, 2, 2);
  }
}

// Kiểm tra ánh sáng và chuyển chế độ ngày/đêm
void checkLight()
{
  static ulong lastCheck = 0;
  if (!IsReady(lastCheck, 500))
    return;

  int lightValue = analogRead(PIN_LIGHT_SENSOR);
  bool wasNightMode = isNightMode;
  isNightMode = (lightValue < darkThreshold);

  if (wasNightMode != isNightMode)
  {
    Serial.print("Chuyển chế độ: ");
    Serial.println(isNightMode ? " ĐÊM (đèn vàng)" : " NGÀY (đèn giao thông)");
    Blynk.virtualWrite(V6, isNightMode);
    controlLights();
    updateDisplay();
  }

  // Gửi giá trị ánh sáng lên Blynk
  Blynk.virtualWrite(V8, lightValue);
}
