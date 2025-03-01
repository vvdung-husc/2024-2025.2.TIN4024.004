#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân kết nối
#define LED_RED 27
#define LED_YELLOW 26
#define LED_GREEN 25
#define BUTTON 23
#define LDR_PIN 13
#define TM1637_CLK 18
#define TM1637_DIO 19

// Khai báo màn hình TM1637
TM1637Display display(TM1637_CLK, TM1637_DIO);

// Trạng thái đèn giao thông
enum TrafficState { RED, YELLOW, GREEN };
TrafficState currentState = RED;
int countdownTime = 10;  // Thời gian đếm ngược

// Biến kiểm tra nút nhấn
bool buttonPressed = false;

// Đọc giá trị cảm biến ánh sáng
int readLightLevel() {
    int ldrValue = analogRead(LDR_PIN);  // Đọc giá trị LDR (0-4095)
    return map(ldrValue, 0, 4095, 0, 255); // Chuyển đổi về mức độ sáng
}

// Chuyển trạng thái đèn
void changeState() {
    switch (currentState) {
        case RED:
            digitalWrite(LED_RED, HIGH);
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_GREEN, LOW);
            countdownTime = 10;
            currentState = GREEN;
            break;

        case GREEN:
            digitalWrite(LED_RED, LOW);
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_GREEN, HIGH);
            countdownTime = 10;
            currentState = YELLOW;
            break;

        case YELLOW:
            digitalWrite(LED_RED, LOW);
            digitalWrite(LED_YELLOW, HIGH);
            digitalWrite(LED_GREEN, LOW);
            countdownTime = 3;
            currentState = RED;
            break;
    }
}

// Kiểm tra nút nhấn
void IRAM_ATTR handleButtonPress() {
    buttonPressed = true;
}

void setup() {
    // Cấu hình đầu vào & đầu ra
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    attachInterrupt(BUTTON, handleButtonPress, FALLING); // Ngắt khi nhấn nút

    // Cấu hình màn hình 7 đoạn
    display.setBrightness(7);
    display.showNumberDec(countdownTime);
}

void loop() {
    if (buttonPressed) {
        buttonPressed = false;
        changeState();
    }

    for (int i = countdownTime; i > 0; i--) {
        display.showNumberDec(i);
        analogWrite(LED_RED, readLightLevel());  // Điều chỉnh độ sáng đèn đỏ
        analogWrite(LED_YELLOW, readLightLevel());  // Điều chỉnh độ sáng đèn vàng
        analogWrite(LED_GREEN, readLightLevel());  // Điều chỉnh độ sáng đèn xanh
        delay(1000);
    }

    changeState();
}
