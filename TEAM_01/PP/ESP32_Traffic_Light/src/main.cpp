#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân kết nối
#define BUTTON_PIN 23
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define LED_BLUE 21
#define LDR_PIN 13
#define CLK 18
#define DIO 19

// Khai báo TM1637
TM1637Display display(CLK, DIO);

// Biến trạng thái
bool isDisplayOn = true;
int timer = 3;
int currentLight = 0; // 0: Red, 1: Green, 2: Yellow
unsigned long previousMillis = 0;
bool isDark = false;
bool yellowState = false;
int lastLightValue = -1;
int lastLightState = -1; // Lưu trạng thái đèn trước đó
unsigned long lightChangeMillis = 0; // Thời gian đổi đèn

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    display.setBrightness(7);
    display.showNumberDec(timer);
}

void loop() {
    // Đọc giá trị cảm biến ánh sáng
    int lightValue = analogRead(LDR_PIN);

    // Kiểm tra nếu giá trị ánh sáng thay đổi đáng kể mới in ra terminal
    if (abs(lightValue - lastLightValue) > 50) {
        Serial.print("LDR Value: ");
        Serial.println(lightValue);
        lastLightValue = lightValue;
    }

    // Kiểm tra điều kiện trời tối
    isDark = (lightValue < 500);

    // Đọc nút nhấn để bật/tắt bộ đếm và đèn xanh dương
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(200); // Chống dội phím
        isDisplayOn = !isDisplayOn;
        if (!isDisplayOn) {
            display.clear(); // Tắt màn hình
            digitalWrite(LED_BLUE, LOW); // Tắt đèn xanh dương
        } else {
            display.showNumberDec(timer); // Hiển thị lại số
            digitalWrite(LED_BLUE, HIGH); // Bật đèn xanh dương
        }
    }

    unsigned long currentMillis = millis();

    if (isDark) {
        // Đèn vàng nhấp nháy mỗi 1000ms khi trời tối
        if (currentMillis - previousMillis >= 1000) {
            previousMillis = currentMillis;
            yellowState = !yellowState;
            digitalWrite(LED_YELLOW, yellowState ? HIGH : LOW);
        }
        // Tắt các đèn khác
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, LOW);

        // Chỉ in ra terminal khi vào chế độ đèn vàng nhấp nháy
        if (lastLightState != 3) {
            Serial.println("Traffic Light: Blinking Yellow (Night Mode)");
            lastLightState = 3;
        }
    } else {
        // Chế độ hoạt động bình thường khi trời sáng
        if (currentMillis - previousMillis >= 1000) { // Mỗi giây
            previousMillis = currentMillis;
            timer--;
            if (isDisplayOn) {
                display.showNumberDec(timer); // Chỉ hiển thị nếu được bật
            }
        }

        if (timer == 0) {
            timer = 3;
            int previousLight = currentLight;
            currentLight = (currentLight + 1) % 3; // Đổi trạng thái đèn
            unsigned long transitionTime = millis() - lightChangeMillis;
            lightChangeMillis = millis();

            // In ra terminal thời gian chuyển đổi giữa các đèn
            Serial.print(previousLight == 0 ? "1. RED" : (previousLight == 1 ? "2. GREEN" : "3. YELLOW"));
            Serial.print(" => ");
            Serial.print(currentLight == 0 ? "RED" : (currentLight == 1 ? "GREEN" : "YELLOW"));
            Serial.print(" (");
            Serial.print(transitionTime);
            Serial.println("ms)");
        }

        // Cập nhật trạng thái đèn giao thông
        digitalWrite(LED_RED, currentLight == 0 ? HIGH : LOW);
        digitalWrite(LED_GREEN, currentLight == 1 ? HIGH : LOW);
        digitalWrite(LED_YELLOW, currentLight == 2 ? HIGH : LOW);
    }

    delay(100);
}
