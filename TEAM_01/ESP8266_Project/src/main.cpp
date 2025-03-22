#define BLYNK_TEMPLATE_ID "TMPL6edj9LqxJ"
#define BLYNK_TEMPLATE_NAME "TeamProject02"
#define BLYNK_AUTH_TOKEN "w1ceRxYKYOJ6IVG_54mUkTdrNtkVONOR"

#include <Arduino.h>
#include "utils.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <U8g2lib.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <BlynkSimpleEsp8266.h>

char ssid[] = "CNTT-MMT";  //Tên mạng WiFi
char pass[] = "13572468";             //Mật khẩu mạng WiFi

#define gPIN 15        // Đèn xanh
#define yPIN 2         // Đèn vàng
#define rPIN 5         // Đèn đỏ
#define switchPIN 14   // Công tắc chuyển chế độ nhấp nháy

#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

DHT dht(D0, DHT11);

bool yellowBlinkMode = false; // Biến trạng thái chế độ đèn vàng nhấp nháy

float temperature = 0.0;
float humidity = 0.0;

// Hàm tạo số ngẫu nhiên trong khoảng min-max
float randomFloat(float minVal, float maxVal) {
    return minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
}

// Kiểm tra công tắc nhấp nháy
void checkSwitch() {
    static bool lastSwitchState = HIGH;
    bool currentSwitchState = digitalRead(switchPIN);

    if (currentSwitchState == LOW && lastSwitchState == HIGH) { // Khi nhấn công tắc
        yellowBlinkMode = !yellowBlinkMode;
        digitalWrite(yPIN, LOW); // Đảm bảo tắt đèn vàng trước khi nhấp nháy
        Serial.println(yellowBlinkMode ? "Yellow light flashing mode ON" : " Traffic mode ON");
    }

    lastSwitchState = currentSwitchState;
}
BLYNK_WRITE(V3) {
  int buttonState = param.asInt();  // Nhận giá trị từ Blynk
  yellowBlinkMode = (buttonState == 1);  // Nếu button = 1 → bật chế độ nhấp nháy
  digitalWrite(yPIN, LOW); // Tắt đèn vàng trước khi bật chế độ nhấp nháy
}


// Hàm hiển thị thông tin lên OLED
void updateOLED() {
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);

    // Hiển thị nhiệt độ
    String tempString = String("Nhiet do: ") + String(temperature, 2) + " °C";
    oled.drawUTF8(0, 20, tempString.c_str());

    // Hiển thị độ ẩm
    String humString = String("Do am: ") + String(humidity, 2) + " %";
    oled.drawUTF8(0, 40, humString.c_str());

    oled.sendBuffer();
}

// Cập nhật giá trị nhiệt độ & độ ẩm (giả lập)
void updateDHT() {
    static ulong lastTimer = 0;
    if (!IsReady(lastTimer, 2000)) return; // Cập nhật mỗi 2 giây

    temperature = randomFloat(-40.0, 80.0);
    humidity = randomFloat(0.0, 100.0);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Gửi dữ liệu lên Blynk
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);
}

// Điều khiển đèn giao thông
void updateTrafficLight() {
    static ulong lastTimer = 0;
    if (!IsReady(lastTimer, 1000)) return; // Chuyển đèn mỗi giây

    if (yellowBlinkMode) {
        digitalWrite(gPIN, LOW);
        digitalWrite(rPIN, LOW);
        digitalWrite(yPIN, !digitalRead(yPIN)); // Nhấp nháy đèn vàng mỗi 500ms
    } else {
        static int currentLed = 0;
        static const int ledPins[3] = {gPIN, yPIN, rPIN};

        digitalWrite(ledPins[(currentLed + 2) % 3], LOW); // Tắt đèn trước đó
        digitalWrite(ledPins[currentLed], HIGH); // Bật đèn hiện tại
        currentLed = (currentLed + 1) % 3;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(gPIN, OUTPUT);
    pinMode(yPIN, OUTPUT);
    pinMode(rPIN, OUTPUT);
    pinMode(switchPIN, INPUT_PULLUP);

    Serial.print("Connecting to ");Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi
    Serial.println("WiFi connected");

    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    digitalWrite(rPIN, LOW);

    Wire.begin(OLED_SDA, OLED_SCL);
    oled.begin();
    oled.clearBuffer();
    oled.sendBuffer();
    
    Serial.println("System boot complete!");
}

void loop() {
    checkSwitch();
    updateTrafficLight();
    updateDHT();
    updateOLED();
}
