
#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6EuZYqC1_"
#define BLYNK_TEMPLATE_NAME "NgocBao"
#define BLYNK_AUTH_TOKEN "4hpN9hiXTN_J1IfoyjPPmEBmMzpdWVXq"
// Phải để trước khai báo sử dụng thư viện Blynk

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// char ssid[] = "CNTT-MMT";  //Tên mạng WiFi
// char pass[] = "13572468";             //Mật khẩu mạng WiFi

#define btnBLED  23 
#define pinBLED  21 
#define DHTPIN 16      
#define DHTTYPE DHT22  
#define CLK 18  
#define DIO 19  

#define LDR_PIN 13
#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

ulong currentMiliseconds = 0;
bool blueButtonON = true;
ulong lastMillis = 0;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void updateLightIndicator();

void setup() {
    Serial.begin(115200);
    pinMode(pinBLED, OUTPUT);
    pinMode(btnBLED, INPUT_PULLUP);
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    
    dht.begin();
    display.setBrightness(0x0f);
    display.showNumberDec(0, false);

    Serial.print("Connecting to "); Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("WiFi connected");

    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
}

void loop() {
    Blynk.run();

    if (millis() - lastMillis >= 2000) {
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

    updateLightIndicator();
    updateBlueButton();
    uptimeBlynk();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
    if (millis() - ulTimer < milisecond) return false;
    ulTimer = millis();
    return true;
}

void updateBlueButton() {
    static ulong lastTime = 0;
    static int lastValue = HIGH;
    if (!IsReady(lastTime, 50)) return;
    int v = digitalRead(btnBLED);
    if (v == lastValue) return;
    lastValue = v;
    if (v == LOW) return;

    blueButtonON = !blueButtonON;
    Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);
    if (!blueButtonON) display.clear();
}

void uptimeBlynk() {
    static ulong lastTime = 0;
    if (!IsReady(lastTime, 1000)) return;
    ulong value = lastTime / 1000;
    Blynk.virtualWrite(V0, value);
    if (blueButtonON) display.showNumberDec(value);
}

void updateLightIndicator() {
    static ulong lastBlinkTime = 0;
    static bool ledState = false;
    if (!IsReady(lastBlinkTime, 500)) return;
    ledState = !ledState;
    
    int ldrValue = analogRead(LDR_PIN);
    Serial.print("LDR Value: "); Serial.println(ldrValue);

    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);

    if (ldrValue < 500) {
        digitalWrite(RED_LED, ledState);
    } else if (ldrValue < 1000) {
        digitalWrite(YELLOW_LED, ledState);
    } else {
        digitalWrite(GREEN_LED, ledState);
    }
}

