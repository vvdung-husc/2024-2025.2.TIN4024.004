#include <Arduino.h>

// Pin định nghĩa cho đèn LED giao thông
const int redLedPin = 23;
const int yellowLedPin = 22;
const int greenLedPin = 21;

// Pin định nghĩa cho 7-segment
const int segmentPins[] = {13, 12, 14, 27, 26, 25, 33, 32}; // A, B, C, D, E, F, G, DP
const int digitPins[] = {4, 16, 17, 5}; // Chữ số từ trái sang phải: DIG1, DIG2, DIG3, DIG4

// Thời gian đếm ngược của từng đèn (giây)
const int redDuration = 5;
const int yellowDuration = 3;
const int greenDuration = 5;

// Trạng thái đèn hiện tại
int currentLed = 0;  // 0: đỏ, 1: vàng, 2: xanh
int remainingTime = 0;
unsigned long lastUpdate = 0;

// Dữ liệu hiển thị trên 7-segment (common cathode)
const byte digitPatterns[] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

void startNewLight(int light) {
    currentLed = light;
    switch (light) {
        case 0: remainingTime = redDuration; break;
        case 1: remainingTime = yellowDuration; break;
        case 2: remainingTime = greenDuration; break;
    }
    
    digitalWrite(redLedPin, (light == 0) ? HIGH : LOW);
    digitalWrite(yellowLedPin, (light == 1) ? HIGH : LOW);
    digitalWrite(greenLedPin, (light == 2) ? HIGH : LOW);
    lastUpdate = millis();
}

void setup() {
    pinMode(redLedPin, OUTPUT);
    pinMode(yellowLedPin, OUTPUT);
    pinMode(greenLedPin, OUTPUT);
    
    for (int i = 0; i < 8; i++) {
        pinMode(segmentPins[i], OUTPUT);
        digitalWrite(segmentPins[i], HIGH);  // Tắt các đoạn LED
    }
    
    for (int i = 0; i < 4; i++) {
        pinMode(digitPins[i], OUTPUT);
        digitalWrite(digitPins[i], HIGH);  // Tắt các chữ số
    }
    
    startNewLight(0);
}

void updateCountdown() {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= 1000) { // Mỗi giây cập nhật một lần
        lastUpdate = currentTime;
        if (remainingTime > 0) {
            remainingTime--;
        } else {
            int nextLight = (currentLed + 1) % 3;
            startNewLight(nextLight);
        }
    }
}

void displayNumber(int num) {
    static int currentDigit = 0; // Chữ số đang quét
    static unsigned long lastScanTime = 0;
    const int scanDelay = 2; // Độ trễ quét
    
    if (millis() - lastScanTime < scanDelay) return;
    lastScanTime = millis();
    
    // Xác định vị trí hiển thị dựa vào số chữ số
    int numDigits = (num >= 10) ? 2 : 1;
    int startPos = 4 - numDigits; // Vị trí bắt đầu từ phải sang trái
    
    // Tắt tất cả chữ số trước khi quét chữ số mới
    for (int i = 0; i < 4; i++) {
        digitalWrite(digitPins[i], HIGH);
    }
    
    // Tắt tất cả segment trước khi hiển thị số mới
    for (int i = 0; i < 8; i++) {
        digitalWrite(segmentPins[i], HIGH);
    }
    
    // Hiển thị số lên 7-segment tại vị trí phù hợp
    int digitValue = (currentDigit < numDigits) ? ((num % (int)pow(10, numDigits)) / (int)pow(10, numDigits - currentDigit - 1)) : 0;
    
    for (int i = 0; i < 8; i++) {
        digitalWrite(segmentPins[i], !bitRead(digitPatterns[digitValue], i));
    }
    
    // Bật chữ số phù hợp từ phải sang trái
    digitalWrite(digitPins[startPos + currentDigit], LOW);
    
    // Chuyển sang chữ số tiếp theo
    currentDigit = (currentDigit + 1) % numDigits;
}

void loop() {
    updateCountdown();
    displayNumber(remainingTime);
}