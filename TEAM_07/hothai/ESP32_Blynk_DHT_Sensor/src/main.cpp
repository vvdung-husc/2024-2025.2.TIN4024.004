#include <Arduino.h>
#include <TM1637Display.h>


/* Blynk Credentials */
#define BLYNK_TEMPLATE_ID "TMPL6YwPr4U8Y"
#define BLYNK_TEMPLATE_NAME "template sensor"
#define BLYNK_AUTH_TOKEN "-fymtbbZajlOwno89VqOzO936vwok-UQ"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi credentials
char ssid[] = "Wokwi-GUEST";  // WiFi Name
char pass[] = "";             // WiFi Password

// Chân kết nối phần cứng
#define DHTPIN 16         // Chân DATA của cảm biến DHT
#define DHTTYPE DHT22     // Loại cảm biến: DHT22
#define CLK 18            // Chân CLK của TM1637
#define DIO 19            // Chân DIO của TM1637
#define btnBLED 23        // Chân nút nhấn
#define pinBLED 21        // Chân LED

// Khởi tạo cảm biến DHT và màn hình TM1637
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

// Biến toàn cục
ulong currentMiliseconds = 0;
bool blueButtonON = true;

// Hàm kiểm tra thời gian chạy
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void sendSensorData();
void uptimeBlynk();

void setup() {
    Serial.begin(115200);
    pinMode(pinBLED, OUTPUT);
    pinMode(btnBLED, INPUT_PULLUP);
    
    display.setBrightness(0x0f);
    dht.begin();  // Khởi động DHT sensor
    
    Serial.print("Connecting to WiFi: "); Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("WiFi connected!");

    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    Serial.println("== START ==");
}

void loop() {
    Blynk.run();
    currentMiliseconds = millis();
    updateBlueButton();
    sendSensorData();
    uptimeBlynk(); // Cập nhật thời gian chạy
}

// Kiểm tra debounce của nút nhấn
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

// Hàm đọc cảm biến và gửi dữ liệu lên Blynk
void sendSensorData() {
    static ulong lastTime = 0;
    if (!IsReady(lastTime, 2000)) return; // Đọc dữ liệu mỗi 2 giây

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    Serial.print("Temperature: "); Serial.print(t); Serial.println(" °C");
    Serial.print("Humidity: "); Serial.print(h); Serial.println(" %");

    Blynk.virtualWrite(V2, t); // Gửi nhiệt độ lên Blynk
    Blynk.virtualWrite(V3, h); // Gửi độ ẩm lên Blynk

    if (blueButtonON) {
        display.showNumberDec(t, false); // Hiển thị nhiệt độ trên TM1637
    }
}

// Cập nhật thời gian chạy lên Blynk & hiển thị trên TM1637
void uptimeBlynk() {
    static ulong lastTime = 0;
    if (!IsReady(lastTime, 1000)) return; // Cập nhật mỗi giây

    ulong uptime = millis() / 1000; // Thời gian chạy tính bằng giây
    Blynk.virtualWrite(V0, uptime); // Gửi lên Blynk

    if (blueButtonON) {
        display.showNumberDec(uptime % 10000, false); // Hiển thị 4 chữ số cuối để tránh tràn màn hình
    }
}

// Kiểm tra thời gian định kỳ
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
    if (currentMiliseconds - ulTimer < milisecond) return false;
    ulTimer = currentMiliseconds;
    return true;
}

// Nhận lệnh từ Blynk để bật/tắt LED
BLYNK_WRITE(V1) {
    blueButtonON = param.asInt();
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    if (!blueButtonON) display.clear();
}
