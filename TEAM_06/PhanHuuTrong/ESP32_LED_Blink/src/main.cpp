#include <Arduino.h>

const int ledPin = 23;  // Chân kết nối tới LED

void setup() {
  Serial.begin(115200);  // Khởi động serial với tốc độ 115200
  Serial.println("Cai j day");  // In ra câu "Cai j day" để kiểm tra

  pinMode(ledPin, OUTPUT);  // Đặt chân LED là chân xuất
}

void loop() {
  static int i = 0;  // Biến đếm số lần lặp
  unsigned long t = millis();  // Lấy thời gian hiện tại tính bằng mili giây từ khi ESP32 bắt đầu chạy
  Serial.println(t);  // In ra thời gian hiện tại
  Serial.println("Loop time: ");  // In ra chuỗi "Loop time: "
  Serial.println(++i);  // Tăng biến đếm lên 1 và in ra giá trị

  // Làm LED nhấp nháy
  digitalWrite(ledPin, HIGH);  // Bật LED
  delay(500);  // Chờ 500 mili giây
  digitalWrite(ledPin, LOW);  // Tắt LED
  delay(500);  // Chờ 500 mili giây
}
