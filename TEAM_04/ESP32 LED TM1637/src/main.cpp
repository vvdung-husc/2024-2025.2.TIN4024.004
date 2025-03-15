#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6Z64wYt7o"
#define BLYNK_TEMPLATE_NAME "BinkLed"
#define BLYNK_AUTH_TOKEN "JyNJ1eIYR7qjthWwnR_m8KABDoT8LeZt"

// Thư viện Blynk và WiFi
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi dùng cho mô phỏng Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED  23  // Chân kết nối nút bấm
#define pinBLED  21  // Chân kết nối đèn xanh

#define DHTPIN 16      // Chân kết nối cảm biến DHT22
#define DHTTYPE DHT22  // Loại cảm biến

#define CLK 18  // Chân kết nối CLK của TM1637
#define DIO 19  // Chân kết nối DIO của TM1637

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

// Biến toàn cục
ulong currentMiliseconds = 0;
bool blueButtonON = true;
ulong lastMillis = 0;
ulong lastTime = 0;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  
  dht.begin();
  display.setBrightness(0x0f);
  display.showNumberDec(0, false);

  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);  // Kết nối WiFi

  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); // Đồng bộ trạng thái đèn với Blynk

  Serial.println("== START ==>");  
}

void loop() {  
  Blynk.run();  // Chạy Blynk để cập nhật trạng thái từ Blynk Cloud

  if (millis() - lastMillis >= 2000) { // Cập nhật mỗi 2 giây
    lastMillis = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Lỗi đọc cảm biến DHT22!");
      return;
    }

    Serial.print("Nhiệt độ: "); Serial.print(temperature); Serial.print("°C, ");
    Serial.print("Độ ẩm: "); Serial.print(humidity); Serial.println("%");

    display.showNumberDec((int)temperature, false);

    Blynk.virtualWrite(V2, temperature); 
    Blynk.virtualWrite(V3, humidity);    
  }

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
}

// Kiểm tra khoảng thời gian giữa 2 lần cập nhật
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Cập nhật trạng thái nút bấm
void updateBlueButton() {
  static ulong lastTimeButton = 0;
  static int lastValue = HIGH;
  
  if (!IsReady(lastTimeButton, 50)) return;
  
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON;
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON);

  if (!blueButtonON) {
    display.clear();
    Serial.println("Blue Light OFF");
  } else {
    Serial.println("Blue Light ON");
  }
}

// Gửi thời gian chạy lên Blynk & hiển thị trên TM1637
void uptimeBlynk() {
  if (!IsReady(lastTime, 1000)) return; // Cập nhật mỗi giây

  ulong value = lastTime / 1000;

  // Giới hạn giá trị để tránh lỗi hiển thị
  if (value > 9999) value = 9999;

  Blynk.virtualWrite(V0, value);

  if (blueButtonON) {
    display.showNumberDec((int)value);
  }
}

// Xử lý dữ liệu từ Blynk
BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

  if (!blueButtonON) {
    display.clear();
    Serial.println("Blynk -> Blue Light OFF");
  } else {
    Serial.println("Blynk -> Blue Light ON");
  }
}
