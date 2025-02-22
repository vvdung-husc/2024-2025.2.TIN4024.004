#include <Arduino.h>
#include <TM1637Display.h>

// Chân kết nối
#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25
#define LDR_PIN 13
#define CLK 18
#define DIO 19

// Cấu hình TM1637
TM1637Display display(CLK, DIO);

// Thời gian đèn (ms)
const uint32_t RED_TIME = 7000;
const uint32_t GREEN_TIME = 5000;
const uint32_t YELLOW_TIME = 3000;

// Biến toàn cục
uint32_t lastMillis = 0;
uint32_t countdown = 0;
int currentLED = RED_LED;
bool blinkingMode = false;

void setup() {
    Serial.begin(115200);
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(LDR_PIN, INPUT);
    display.setBrightness(5);
    
    // Bắt đầu với đèn đỏ
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    countdown = RED_TIME / 1000;
    display.showNumberDec(countdown);
    lastMillis = millis();
}

void loop() {
    int lightLevel = analogRead(LDR_PIN);
    if (lightLevel >= 1000) {
        // Chế độ đèn vàng nhấp nháy khi quá tối
        if (!blinkingMode) {
            blinkingMode = true;
            digitalWrite(RED_LED, LOW);
            digitalWrite(GREEN_LED, LOW);
        }
        digitalWrite(YELLOW_LED, millis() % 1000 < 500 ? HIGH : LOW);
        display.clear();
    } else {
        if (blinkingMode) {
            blinkingMode = false;
            digitalWrite(YELLOW_LED, LOW);
            digitalWrite(RED_LED, HIGH);
            currentLED = RED_LED;
            countdown = RED_TIME / 1000;
            lastMillis = millis();
        }

        // Giảm ngược số giây trên TM1637
        if (millis() - lastMillis >= 1000) {
            lastMillis += 1000;
            countdown--;
            display.showNumberDec(countdown);
        }

        // Chuyển đèn khi hết thời gian
        if (countdown == 0) {
            if (currentLED == RED_LED) {
                digitalWrite(RED_LED, LOW);
                digitalWrite(GREEN_LED, HIGH);
                currentLED = GREEN_LED;
                countdown = GREEN_TIME / 1000;
            } else if (currentLED == GREEN_LED) {
                digitalWrite(GREEN_LED, LOW);
                digitalWrite(YELLOW_LED, HIGH);
                currentLED = YELLOW_LED;
                countdown = YELLOW_TIME / 1000;
            } else if (currentLED == YELLOW_LED) {
                digitalWrite(YELLOW_LED, LOW);
                digitalWrite(RED_LED, HIGH);
                currentLED = RED_LED;
                countdown = RED_TIME / 1000;
            }
            display.showNumberDec(countdown);
            lastMillis = millis();
        }
    }
}
