#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

// ==== Blynk Credentials ====
#define BLYNK_TEMPLATE_ID "TMPL6SLIe5Lkl"
#define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
#define BLYNK_AUTH_TOKEN "Vhux4WLbXWw5lgLTYV04eFHYuBaov5pr"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// ==== WiFi Credentials ====
char ssid[] = "Wokwi-GUEST";  
char pass[] = "";

// ==== Định nghĩa các chân linh kiện ====
#define btnBLED  23  // Nút nhấn
#define pinBLED  21  // Đèn xanh
#define CLK 18       // Chân CLK TM1637
#define DIO 19       // Chân DIO TM1637
#define DHTPIN 16    // Chân cảm biến DHT22
#define LDRPIN 34    // Thay GPIO13 bằng GPIO34

#define DHTTYPE DHT22  

// ==== Chân LED đèn giao thông ====
#define rLED 27
#define yLED 26
#define gLED 25

// Thời gian đèn
uint rTIME = 5000;
uint yTIME = 3000;
uint gTIME = 7000;

// ==== Biến toàn cục ====
ulong currentMillis = 0;
ulong ledTimeStart = 0;
int currentLED = rLED;
int countdownTime = 5; 
bool led1State = true;  // Trạng thái hiển thị TM1637

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);
int darkThreshold = 1000;  // Ngưỡng ánh sáng để phát hiện ban đêm

void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();
bool IsReady(ulong &ulTimer, uint32_t milisecond);

void setup() {
  Serial.begin(115200);

  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);  // Nút nhấn sử dụng pull-up
  pinMode(LDRPIN, INPUT);

  display.setBrightness(7);
  
  digitalWrite(rLED, HIGH);
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);

  dht.begin(); // Khởi tạo DHT22
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Kết nối Blynk
}

void loop() {
  currentMillis = millis();

  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();
  Blynk.run(); 

  // Đọc cảm biến DHT22 và gửi lên Blynk
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (!isnan(humidity) && !isnan(temperature)) {
    Blynk.virtualWrite(V2, temperature);  // Gửi nhiệt độ lên Blynk
    Blynk.virtualWrite(V3, humidity);     // Gửi độ ẩm lên Blynk
  }

  // Đọc giá trị LDR và gửi lên Blynk
  int ldrValue = analogRead(LDRPIN);
  Blynk.virtualWrite(V4, ldrValue);  // Gửi giá trị LDR lên Blynk

  // Kiểm tra nút nhấn (chỉ cần bấm 1 lần)
  if (digitalRead(btnBLED) == LOW) {  // Nút nhấn được bấm
    delay(50);  // Debounce
    if (digitalRead(btnBLED) == LOW) {
      led1State = !led1State;  // Đảo trạng thái LED1
      digitalWrite(pinBLED, led1State ? HIGH : LOW);  // Bật/tắt LED1

      if (!led1State) {
        display.clear();  // Nếu LED1 tắt, xóa màn hình
      } else {
        // Nếu LED1 bật lại, hiển thị thời gian đếm ngược từ giá trị hiện tại
        char buf[3];
        sprintf(buf, "%02d", countdownTime);  // Đảm bảo luôn có 2 chữ số
        display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2);
      }

      Serial.println(led1State ? "LED1 ON - Hiển thị TM1637" : "LED1 OFF - Tắt TM1637");
      while (digitalRead(btnBLED) == LOW);  // Chờ thả nút
    }
  }
}

// ==== Hàm kiểm tra thời gian không chặn ====
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMillis - ulTimer < milisecond)
    return false;
  ulTimer = currentMillis;
  return true;
}

// Hàm điều khiển đèn giao thông không chặn
void NonBlocking_Traffic_Light()
{
  // Đọc giá trị cảm biến LDR
  int ldrValue = analogRead(LDRPIN); // Đọc giá trị từ LDR
  int threshold = 1000;          // Ngưỡng ánh sáng để quyết định ban ngày hay ban đêm

  if (ldrValue < threshold)
  { // Nếu ánh sáng yếu (ban đêm)
    // Ban đêm: chỉ đèn vàng sáng, các đèn còn lại tắt
    digitalWrite(rLED, LOW);  // Đèn đỏ tắt
    digitalWrite(gLED, LOW);  // Đèn xanh tắt
    digitalWrite(yLED, HIGH); // Đèn vàng sáng

    display.clear();
  }
  else
  {
    // Xử lý các trạng thái đèn giao thông
    switch (currentLED)
    {
    case rLED: // Đèn đỏ
      if (IsReady(ledTimeStart, rTIME))
      { // Kiểm tra thời gian đèn đỏ
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        countdownTime = gTIME / 1000; // Cập nhật thời gian đếm ngược cho đèn xanh

        // Hiển thị thời gian nếu LED1 bật
        if (led1State)
        {
          char buf[3];
          sprintf(buf, "%02d", countdownTime);                  // Đảm bảo luôn có 2 chữ số
          display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên màn hình TM1637
        }
        Serial.print("2. GREEN  => YELLOW ");
        Serial.print(gTIME / 1000);
        Serial.println(" (s)");
      }
      break;

    case gLED: // Đèn xanh
      if (IsReady(ledTimeStart, gTIME))
      { // Kiểm tra thời gian đèn xanh
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        countdownTime = yTIME / 1000; // Cập nhật thời gian đếm ngược cho đèn vàng

        // Hiển thị thời gian nếu LED1 bật
        if (led1State)
        {
          char buf[3];
          sprintf(buf, "%02d", countdownTime);                  // Đảm bảo luôn có 2 chữ số
          display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên màn hình TM1637
        }

        Serial.print("3. YELLOW => RED    ");
        Serial.print(yTIME / 1000);
        Serial.println(" (s)");
      }
      break;

    case yLED: // Đèn vàng
      if (IsReady(ledTimeStart, yTIME))
      { // Kiểm tra thời gian đèn vàng
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        countdownTime = rTIME / 1000; // Cập nhật thời gian đếm ngược cho đèn đỏ

        // Hiển thị thời gian nếu LED1 bật
        if (led1State)
        {
          char buf[3];
          sprintf(buf, "%02d", countdownTime);                  // Đảm bảo luôn có 2 chữ số
          display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên màn hình TM1637
        }

        Serial.print("1. RED    => GREEN  ");
        Serial.print(rTIME / 1000);
        Serial.println(" (s)");
      }
      break;
    }
  }
}

// ==== Hiển thị thời gian đếm ngược trên TM1637 ====
void NonBlocking_Traffic_Light_TM1637()
{
  static ulong lastUpdate = 0;

  if (millis() - lastUpdate >= 1000)
  { // Cập nhật mỗi giây
    lastUpdate = millis();

    if (countdownTime > 0)
    {
      countdownTime--; // Giảm thời gian đếm ngược

      // Nếu LED1 bật, hiển thị thời gian
      if (led1State)
      {
        int displayValue = countdownTime % 100; // Lấy 2 chữ số cuối của thời gian
        char buf[3];
        sprintf(buf, "%02d", displayValue);                   // Đảm bảo luôn có 2 chữ số
        display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên màn hình TM1637
      }
      else
      {
        display.clear(); // Nếu LED1 tắt, xóa màn hình TM1637
      }
    }
  }
}
