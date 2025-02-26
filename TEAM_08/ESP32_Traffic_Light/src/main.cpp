#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân kết nối
#define BTN_PIN 23
#define LED1_PIN 21
#define LED2_PIN 26
#define LED3_PIN 27
#define LED4_PIN 25
#define LDR_PIN 13
#define CLK_PIN 18
#define DIO_PIN 19

// Khởi tạo đối tượng màn hình 7 đoạn
TM1637Display display(CLK_PIN, DIO_PIN);

void setup() {
  // Khởi tạo các chân
  pinMode(BTN_PIN, INPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  // Khởi tạo màn hình 7 đoạn
  display.setBrightness(7); // Độ sáng tối đa
  display.showNumberDec(0); // Hiển thị số 0 ban đầu
}

void loop() {
  // Đọc trạng thái nút nhấn
  bool btnState = digitalRead(BTN_PIN);

  // Điều khiển LED1 dựa trên trạng thái nút nhấn
  digitalWrite(LED1_PIN, btnState);

  // Đọc giá trị cảm biến ánh sáng
  int ldrValue = analogRead(LDR_PIN);

  // Điều khiển LED2, LED3, LED4 dựa trên giá trị cảm biến ánh sáng
  if (ldrValue < 1000) {
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED3_PIN, LOW);
    digitalWrite(LED4_PIN, LOW);
  } else if (ldrValue >= 1000 && ldrValue < 2000) {
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, HIGH);
    digitalWrite(LED4_PIN, LOW);
  } else {
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);
    digitalWrite(LED4_PIN, HIGH);
  }

  // Hiển thị giá trị cảm biến ánh sáng trên màn hình 7 đoạn
  display.showNumberDec(ldrValue);

  // Đợi một chút trước khi lặp lại
  delay(100);
}