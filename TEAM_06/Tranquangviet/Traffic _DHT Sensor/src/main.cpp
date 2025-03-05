#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// Blynk Credentials
#define BLYNK_TEMPLATE_ID "TMPL6C4XcVTsM"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "jEExrLPInYHy-eF4NzAiOpG1vWNeEhTC"

// WiFi Credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// DHT Sensor
#define DHTPIN 15  // Chân kết nối DHT
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// LED Control
#define btnBLED  23
#define pinBLED  21
bool blueButtonON = true;

// TM1637 Display
#define CLK 18  
#define DIO 19  
TM1637Display display(CLK, DIO);

ulong currentMiliseconds = 0;
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT();

void setup() {
    Serial.begin(115200);
    pinMode(pinBLED, OUTPUT);
    pinMode(btnBLED, INPUT_PULLUP);
    
    display.setBrightness(0x0f);
    
    // Kết nối WiFi & Blynk
    Serial.print("Connecting to "); Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("WiFi connected");

    // Khởi động DHT
    dht.begin();

    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
}

void loop() {
    Blynk.run();
    currentMiliseconds = millis();
    
    uptimeBlynk();
    updateBlueButton();
    readDHT();
}

// Debounce nút nhấn
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
    if (currentMiliseconds - ulTimer < milisecond) return false;
    ulTimer = currentMiliseconds;
    return true;
}

// Điều khiển LED bằng nút nhấn
void updateBlueButton() {
    static ulong lastTime = 0;
    static int lastValue = HIGH;
    if (!IsReady(lastTime, 50)) return;
    int v = digitalRead(btnBLED);
    if (v == lastValue) return;
    lastValue = v;
    if (v == LOW) return;

    blueButtonON = !blueButtonON;
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);

    if (!blueButtonON) display.clear();
}

// Gửi uptime lên Blynk & hiển thị trên TM1637
void uptimeBlynk() {
    static ulong lastTime = 0;
    if (!IsReady(lastTime, 1000)) return;
    ulong value = lastTime / 1000;
    Blynk.virtualWrite(V0, value);
    if (blueButtonON) display.showNumberDec(value);
}

// Đọc cảm biến DHT & gửi dữ liệu lên Blynk
void readDHT() {
    static ulong lastTime = 0;
    if (!IsReady(lastTime, 2000)) return;  // Đọc mỗi 2 giây
    
    float nhietDo = dht.readTemperature();
    float doAm = dht.readHumidity();

    if (isnan(nhietDo) || isnan(doAm)) {
        Serial.println("Lỗi đọc cảm biến DHT!");
        return;
    }

    Serial.print("Nhiệt độ: "); Serial.print(nhietDo);
    Serial.print(" °C - Độ ẩm: "); Serial.print(doAm); Serial.println(" %");

    Blynk.virtualWrite(V2, nhietDo);
    Blynk.virtualWrite(V3, doAm);
}

// Nhận điều khiển từ Blynk
BLYNK_WRITE(V1) {
    blueButtonON = param.asInt();
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    if (!blueButtonON) display.clear();
}
