#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6c2fJ4lN_"
#define BLYNK_TEMPLATE_NAME "DHTHuy"
#define BLYNK_AUTH_TOKEN "kTD2YUCEsNn2yRqfIZKvKrtLv0dUfsYu"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi settings (Wokwi)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Chân kết nối phần cứng
#define btnBLED 23 // Nút bấm
#define pinBLED 21 // Đèn xanh LED
#define CLK 18     // Chân CLK TM1637
#define DIO 19     // Chân DIO TM1637
#define DHTPIN 16  // Chân DATA của DHT22
#define DHTTYPE DHT22

// Khởi tạo đối tượng
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

// Biến toàn cục
ulong currentMiliseconds = 0;
bool blueButtonON = true; // Trạng thái nút bấm
float temperature = 0.0;
float humidity = 0.0;

// Khai báo hàm
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void updateSensor();
void uptimeBlynk();

void setup()
{
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  display.setBrightness(0x0f);
  dht.begin();

  // Kết nối WiFi + Blynk
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
  updateSensor();
}

// Hàm kiểm tra thời gian
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Cập nhật trạng thái nút bấm
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

// Cập nhật nhiệt độ và độ ẩm từ DHT22
void updateSensor()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000))
    return; // Đọc dữ liệu mỗi 2 giây

  float newTemp = dht.readTemperature();
  float newHum = dht.readHumidity();

  if (!isnan(newTemp) && !isnan(newHum))
  {
    temperature = newTemp;
    humidity = newHum;
    Serial.printf("Temp: %.1f °C, Humidity: %.1f%%\n", temperature, humidity);

    Blynk.virtualWrite(V2, temperature); // Gửi nhiệt độ lên Blynk (V2)
    Blynk.virtualWrite(V3, humidity);    // Gửi độ ẩm lên Blynk (V3)
  }
  else
  {
    Serial.println("Failed to read from DHT sensor!");
  }
}

// Cập nhật thời gian uptime trên Blynk
void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return;

  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (blueButtonON)
  {
    display.showNumberDec(value);
  }
}

// Xử lý lệnh từ Blynk
BLYNK_WRITE(V1)
{
  blueButtonON = param.asInt();
  Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

  if (!blueButtonON)
    display.clear();
}
