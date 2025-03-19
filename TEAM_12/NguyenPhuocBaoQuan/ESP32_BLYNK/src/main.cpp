/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6Ki0L76yw"
#define BLYNK_TEMPLATE_NAME "ESP32 NDDA"
#define BLYNK_AUTH_TOKEN "E-fN-MWNjj37N7gifOlI56A6-_YKxiSv"

#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <DHT_U.h>



char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define btnBLED  23 
#define pinBLED  21 
#define CLK 18  
#define DIO 19  

#define DHTPIN 16  
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

ulong currentMiliseconds = 0;
bool blueButtonON = true;

TM1637Display display(CLK, DIO);

float temperature = 0;
float humidity = 0;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void updateDHT22();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(0x0f);
  dht.begin();
  
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass);
  
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON);
  
  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run();
  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  updateDHT22();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 50)) return;  // Kiểm tra debounce mỗi 50ms
  
  int buttonState = digitalRead(btnBLED);
  if (buttonState == LOW) { // Nút nhấn xuống
    blueButtonON = !blueButtonON; // Đảo trạng thái LED
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);  
    Blynk.virtualWrite(V1, blueButtonON);
    
    Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");

    if (!blueButtonON) {
      display.clear(); // Xóa màn hình khi tắt
    }
    
    delay(200); // Tránh nhiễu khi nhấn giữ nút
  }
}


void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V3, value);
  if (blueButtonON) {
    display.showNumberDec(value);
  }
}

void updateDHT22() {
  static ulong lastTimeDHT = 0;
  if (!IsReady(lastTimeDHT, 2000)) return;
  
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Không đọc được dữ liệu từ cảm biến DHT22!");
    return;
  }
  
  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.print("°C  Độ ẩm: ");
  Serial.print(humidity);
  Serial.println("%");

  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V2, humidity);
}