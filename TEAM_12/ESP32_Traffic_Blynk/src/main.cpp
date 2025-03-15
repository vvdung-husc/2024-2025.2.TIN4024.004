/* Fill in information from Blynk Device Info here */

// Nguyễn Thị Bảo Ngọc
#define BLYNK_TEMPLATE_ID "TMPL6pEvA_RXf"
#define BLYNK_TEMPLATE_NAME "BLYNK"
#define BLYNK_AUTH_TOKEN "VYupZeVSLopvlKEBTu9_tKlX5pI3rSYr"

#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <DHT_U.h>

// Thông tin WiFi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Khai báo chân kết nối (không thay đổi pin)
#define btnBLED  23  // Chân nút bấm
#define pinBLED  21  // Chân đèn xanh
#define rLED     27  // Chân đèn đỏ
#define yLED     26  // Chân đèn vàng
#define gLED     25  // Chân đèn xanh lá
#define CLK      18  // Chân CLK của TM1637
#define DIO      19  // Chân DIO của TM1637
#define DHTPIN   16  // Chân cảm biến DHT22
#define ldrPIN   33  // Chân cảm biến ánh sáng (LDR)

// Khai báo thời gian chờ đèn (ms)
#define rTIME 5000   // 5 giây cho đèn đỏ
#define yTIME 3000   // 3 giây cho đèn vàng
#define gTIME 10000  // 10 giây cho đèn xanh lá

// Ngưỡng ánh sáng tối
int darkThreshold = 1000;
// Khởi tạo đối tượng
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHT22);

// Biến toàn cục
ulong currentMiliseconds = 0;  // Thời gian hiện tại
ulong ledTimeStart = 0;        // Thời gian bắt đầu chạy đèn
int currentLED = rLED;         // Đèn hiện tại đang sáng
int tmCounter = rTIME / 1000;  // Biến đếm ngược trên TM1637
bool blueButtonON = true;      // Trạng thái đèn xanh
bool isDarkState = false;      // Trạng thái ánh sáng tối

// Hàm kiểm tra thời gian đã trôi qua
bool IsReady(ulong &lastTime, uint32_t interval) {
  if (millis() - lastTime < interval) return false;
  lastTime = millis();
  return true;
}

// Hàm điều khiển đèn giao thông
void controlLEDs(int rState, int yState, int gState) {
  digitalWrite(rLED, rState);
  digitalWrite(yLED, yState);
  digitalWrite(gLED, gState);
}

// Hàm hiển thị số trên TM1637
void displayNumber(int number) {
  display.showNumberDec(number, true, 2, 2);
}

// Hàm xử lý đèn giao thông và hiển thị TM1637
void handleTrafficLight() {
  static ulong counterTime = 0;

  if (!blueButtonON) {
    controlLEDs(LOW, LOW, LOW);  // Tắt tất cả đèn nếu đèn xanh tắt
    return;
  }

  switch (currentLED) {
    case rLED:  // Đèn đỏ
      if (IsReady(ledTimeStart, rTIME)) {
        controlLEDs(LOW, LOW, HIGH);  // Chuyển sang đèn xanh lá
        currentLED = gLED;
        tmCounter = gTIME / 1000;
        Serial.println("Chuyển từ ĐỎ -> XANH LÁ");
      }
      break;

    case gLED:  // Đèn xanh lá
      if (IsReady(ledTimeStart, gTIME)) {
        controlLEDs(LOW, HIGH, LOW);  // Chuyển sang đèn vàng
        currentLED = yLED;
        tmCounter = yTIME / 1000;
        Serial.println("Chuyển từ XANH LÁ -> VÀNG");
      }
      break;

    case yLED:  // Đèn vàng
      if (IsReady(ledTimeStart, yTIME)) {
        controlLEDs(HIGH, LOW, LOW);  // Chuyển sang đèn đỏ
        currentLED = rLED;
        tmCounter = rTIME / 1000;
        Serial.println("Chuyển từ VÀNG -> ĐỎ");
      }
      break;
  }

  // Hiển thị đếm ngược trên TM1637
  if (IsReady(counterTime, 1000)) {
    displayNumber(tmCounter--);
  }
}

// Hàm xử lý trạng thái tối (nhấp nháy đèn vàng)
void handleDarkState() {
  static ulong blinkTime = 0;
  static bool isBlinking = false;

  if (!blueButtonON) return;  // Không nhấp nháy nếu đèn xanh tắt

  if (IsReady(blinkTime, 500)) {  // Nhấp nháy mỗi 500ms
    digitalWrite(yLED, isBlinking ? LOW : HIGH);
    isBlinking = !isBlinking;
  }
}

// Hàm đọc cảm biến ánh sáng (LDR)
void readLDR() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;

  int ldrValue = analogRead(ldrPIN);
  isDarkState = (ldrValue < darkThreshold);

  Serial.print("Giá trị LDR: ");
  Serial.println(ldrValue);
  Blynk.virtualWrite(V4, ldrValue);
}

// Hàm đọc cảm biến DHT22
void readDHT() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Lỗi đọc cảm biến DHT22!");
    return;
  }

  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.print("Độ ẩm: ");
  Serial.print(humidity);
  Serial.println("%");

  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V2, humidity);
}

// Hàm xử lý nút bấm
void handleButton() {
  static ulong lastTime = 0;
  static int lastState = HIGH;

  if (!IsReady(lastTime, 50)) return;  // Chống nhiễu nút bấm

  int currentState = digitalRead(btnBLED);
  if (currentState == lastState) return;
  lastState = currentState;

  if (currentState == LOW) return;  // Chỉ xử lý khi nhả nút

  blueButtonON = !blueButtonON;  // Đảo trạng thái đèn xanh
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  if (!blueButtonON) {
    controlLEDs(LOW, LOW, LOW);  // Tắt tất cả đèn giao thông
    display.clear();
  } else {
    controlLEDs(currentLED == rLED ? HIGH : LOW, currentLED == yLED ? HIGH : LOW, currentLED == gLED ? HIGH : LOW);
  }

  Serial.println(blueButtonON ? "Đèn xanh BẬT" : "Đèn xanh TẮT");
}

// Hàm gửi thời gian chạy lên Blynk
void sendUptime() {
  static ulong lastTime = 0;
  static ulong uptime = 0;

  if (!IsReady(lastTime, 1000)) return;

  if (blueButtonON) {
    uptime++;
    Blynk.virtualWrite(V3, uptime);
  }
}

// Hàm setup
void setup() {
  Serial.begin(115200);

  // Khởi tạo chân
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(pinBLED, OUTPUT);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(ldrPIN, INPUT);

  // Khởi tạo TM1637 và DHT22
  display.setBrightness(7);
  dht.begin();

  // Kết nối WiFi và Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Đã kết nối WiFi và Blynk");

  // Khởi tạo trạng thái ban đầu
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  controlLEDs(HIGH, LOW, LOW);  // Bắt đầu với đèn đỏ
  displayNumber(tmCounter);
}

// Hàm loop
void loop() {
  Blynk.run();
  currentMiliseconds = millis();

  handleButton();
  readLDR();
  readDHT();
  sendUptime();

  if (isDarkState) {
    handleDarkState();
  } else {
    handleTrafficLight();
  }
}

// Hàm xử lý sự kiện từ Blynk
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

  if (!blueButtonON) {
    controlLEDs(LOW, LOW, LOW);
    display.clear();
  } else {
    controlLEDs(currentLED == rLED ? HIGH : LOW, currentLED == yLED ? HIGH : LOW, currentLED == gLED ? HIGH : LOW);
  }
}

BLYNK_WRITE(V4) {
  int threshold = param.asInt();
  darkThreshold = threshold;
  Serial.print("Ngưỡng ánh sáng mới: ");
  Serial.println(darkThreshold);
}