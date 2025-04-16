#include <Arduino.h>
#include <Wire.h>

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6c2fJ4lN_"
#define BLYNK_TEMPLATE_NAME "DHTHuy"
#define BLYNK_AUTH_TOKEN "kTD2YUCEsNn2yRqfIZKvKrtLv0dUfsYu"

// Cho ESP8266
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi settings (Wokwi)
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

// den giao thong
#define gPIN 15
#define yPIN 2
#define rPIN 5

// khai bao man hinh oled
#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Biến toàn cục
ulong currentMiliseconds = 0;
bool yellowBlinkMode = false; // Trạng thái nhap nhay cua den vang
float temperature = 0.0;
float humidity = 0.0;

// Khai báo hàm
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void updateSensor();
void uptimeBlynk();

// Cập nhật màn hình OLED
void updateOLED()
{
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.setCursor(0, 15);
  oled.print("Temperature: ");
  oled.print(temperature, 1);
  oled.print(" C");

  oled.setCursor(0, 35);
  oled.print("Humidity: ");
  oled.print(humidity, 1);
  oled.print(" %");

  oled.sendBuffer();
}

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
