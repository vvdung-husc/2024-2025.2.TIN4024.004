#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// 🔹 Thông tin Blynk
#define BLYNK_TEMPLATE_ID "TMPL6lCsC-LTG"
#define BLYNK_TEMPLATE_NAME "ESP32FinalTest"
#define BLYNK_AUTH_TOKEN "8KSvJabgCjWzmnXJPMR2HxW2fSP2qXoB"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";  // Tên mạng WiFi
char pass[] = "";             // Mật khẩu mạng WiFi

// 🔹 Khai báo chân kết nối
#define DHTPIN 16       // Chân cảm biến DHT22
#define DHTTYPE DHT22  // Loại cảm biến DHT
#define RELAYPIN 22     // Chân Relay điều khiển quạt
#define LEDPIN 21       // Chân LED báo hiệu quạt
#define BUTTONPIN 23    // Chân nút nhấn (GPIO5)

DHT dht(DHTPIN, DHTTYPE);
float thresholdTemperature = 28.0; // Ngưỡng nhiệt độ
bool blueButtonON = true; 

bool isManualMode = false; // Mặc định là chế độ tự động
bool manualFanState = false; // Trạng thái quạt trong chế độ thủ công

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);  // Kết nối Blynk
  dht.begin();

  pinMode(RELAYPIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT); // Sử dụng điện trở nội

  digitalWrite(RELAYPIN, HIGH); // Tắt quạt ban đầu
  digitalWrite(LEDPIN, LOW);    // LED tắt ban đầu
}
// 🔹 Xử lý khi nhấn nút Blynk V2
BLYNK_WRITE(V2) {
  int buttonState = param.asInt(); // Đọc giá trị từ Blynk
  isManualMode = (buttonState == 1); // Chuyển chế độ
  if (isManualMode) {
    manualFanState = !manualFanState; // Đảo trạng thái quạt nếu đang ở chế độ thủ công
  }
  Serial.println(isManualMode ? "🔄 Chuyển sang chế độ thủ công (Blynk)" : "🔄 Chuyển sang chế độ tự động (Blynk)");
}

void updateBlueButton(){
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  int v = digitalRead(BUTTONPIN);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(LEDPIN, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(LEDPIN, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
  }    
}
// 🔹 Đọc nhiệt độ & gửi lên Blynk
void sendTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Không thể đọc dữ liệu từ DHT22!");
    return;
  }

  bool isFanOn = isManualMode ? manualFanState : (t > thresholdTemperature);

  digitalWrite(RELAYPIN, isFanOn ? HIGH : LOW);
  digitalWrite(LEDPIN, isFanOn ? HIGH : LOW);

  // 🔥 Thông báo trạng thái quạt
  Serial.print(isFanOn ? "🔥 Quạt đang bật." : "❄️ Quạt đang tắt.");
  Serial.println(isManualMode ? " (Thủ công)" : " (Tự động)");

  Serial.print("🌡 Nhiệt độ: ");
  Serial.print(t);
  Serial.println(" °C");

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V0, t);        // Gửi nhiệt độ lên V0
  Blynk.virtualWrite(V1, isFanOn);  // Gửi trạng thái quạt lên V1
}


// 🔹 Kiểm tra nút nhấn để chuyển chế độ
void checkButton() {
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTONPIN);

  if (buttonState == LOW && lastButtonState == HIGH) { // Khi nhấn nút
    delay(50); // Chống rung nút nhấn
    if (digitalRead(BUTTONPIN) == LOW) { 
      isManualMode = !isManualMode; // Đảo chế độ
      manualFanState = isManualMode ? !manualFanState : false; // Bật quạt nếu vào chế độ thủ công
      Serial.println(isManualMode ? "🔄 Chế độ THỦ CÔNG" : "🔄 Chế độ TỰ ĐỘNG");
      Blynk.virtualWrite(V2, isManualMode); // Gửi trạng thái lên Blynk
    }
  }
  lastButtonState = buttonState;
}


void loop() {
  Blynk.run();         // Chạy Blynk
  checkButton();       // Kiểm tra nút nhấn
  sendTemperature();   // Gửi dữ liệu lên Blynk
  delay(2000);         // Đọc lại mỗi 2 giây
}
