// #include <esp32-hal-gpio.h>
#include <Arduino.h>
#define RED_LED    23
#define YELLOW_LED 22
#define GREEN_LED  21

unsigned long previousMillis = 0;
unsigned long blinkMillis = 0;
const int blinkInterval = 500; // Nhấp nháy mỗi 500ms

const int greenTime = 5000;   // 5s
const int yellowTime = 3000;  // 3s
const int redTime = 5000;     // 5s

enum TrafficState { GREEN, YELLOW, RED };
TrafficState currentState = GREEN;

bool greenLedState = false;
bool yellowLedState = false;
bool redLedState = false;

// Khai báo hàm trước khi sử dụng
int getStateDuration(TrafficState state);

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
}

void loop() {
    unsigned long currentMillis = millis();

    // Điều khiển nhấp nháy cho đèn hiện tại
    if (currentMillis - blinkMillis >= blinkInterval) {
        blinkMillis = currentMillis;

        switch (currentState) {
            case GREEN:
                greenLedState = !greenLedState;
                digitalWrite(GREEN_LED, greenLedState);
                break;

            case YELLOW:
                yellowLedState = !yellowLedState;
                digitalWrite(YELLOW_LED, yellowLedState);
                break;

            case RED:
                redLedState = !redLedState;
                digitalWrite(RED_LED, redLedState);
                break;
        }
    }

    // Chuyển trạng thái sau thời gian định sẵn
    if (currentMillis - previousMillis >= getStateDuration(currentState)) {
        previousMillis = currentMillis;
        switch (currentState) {
            case GREEN:
                digitalWrite(GREEN_LED, LOW); // Tắt đèn trước khi chuyển
                greenLedState = false;
                currentState = YELLOW;
                break;
            case YELLOW:
                digitalWrite(YELLOW_LED, LOW);
                yellowLedState = false;
                currentState = RED;
                break;
            case RED:
                digitalWrite(RED_LED, LOW);
                redLedState = false;
                currentState = GREEN;
                break;
        }
    }
}

// Hàm lấy thời gian của từng trạng thái
int getStateDuration(TrafficState state) {
    switch (state) {
        case GREEN: return greenTime;
        case YELLOW: return yellowTime;
        case RED: return redTime;
        default: return 0;
    }
}
