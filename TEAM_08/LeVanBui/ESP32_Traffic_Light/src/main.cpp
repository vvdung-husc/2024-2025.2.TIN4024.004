#include <Arduino.h>

// Định nghĩa chân
#define LDR_PIN 13
#define BTN_PIN 23
#define LED_BLUE 21
#define LED_GREEN 25
#define LED_YELLOW 26
#define LED_RED 27
#define CLK 18
#define DIO 19

// Biến lưu giá trị đếm
int counter = 0;
bool lastButtonState = LOW;

// Bảng mã hiển thị số trên 7 đoạn
const uint8_t digitMap[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

void tm1637Start();
void tm1637Stop();
void tm1637WriteByte(uint8_t data);
void tm1637DisplayNumber(int num);

void setup() {
    pinMode(LDR_PIN, INPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(DIO, OUTPUT);

    tm1637DisplayNumber(counter);
}

void loop() {
    int ldrValue = analogRead(LDR_PIN);
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);

    if (ldrValue < 1000) {
        digitalWrite(LED_YELLOW, HIGH);
    } else {
        digitalWrite(LED_YELLOW, LOW);
    }

    bool buttonState = digitalRead(BTN_PIN) == LOW;
    if (buttonState && !lastButtonState) {
        counter++;
        tm1637DisplayNumber(counter);
    }
    lastButtonState = buttonState;

    digitalWrite(LED_BLUE, counter % 2);
    digitalWrite(LED_GREEN, counter % 3 == 0);
    digitalWrite(LED_RED, counter % 5 == 0);

    delay(100);
}

void tm1637Start() {
    digitalWrite(CLK, HIGH);
    digitalWrite(DIO, HIGH);
    delayMicroseconds(2);
    digitalWrite(DIO, LOW);
}

void tm1637Stop() {
    digitalWrite(CLK, LOW);
    digitalWrite(DIO, LOW);
    delayMicroseconds(2);
    digitalWrite(CLK, HIGH);
    digitalWrite(DIO, HIGH);
}

void tm1637WriteByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(CLK, LOW);
        digitalWrite(DIO, (data & 0x01) ? HIGH : LOW);
        delayMicroseconds(3);
        digitalWrite(CLK, HIGH);
        data >>= 1;
    }
    digitalWrite(CLK, LOW);
    pinMode(DIO, INPUT);
    delayMicroseconds(3);
    pinMode(DIO, OUTPUT);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(3);
}

void tm1637DisplayNumber(int num) {
    tm1637Start();
    tm1637WriteByte(0x40);
    tm1637Stop();
    
    tm1637Start();
    tm1637WriteByte(0xC0);
    tm1637WriteByte(digitMap[num / 10 % 10]);
    tm1637WriteByte(digitMap[num % 10]);
    tm1637Stop();
}
