#include <Arduino.h>
#define LED_PIN 23

//Khởi tạo, chạy 1 lần duy nhất cho đến khi hết pin
void setup() {
  // put your setup code here, to run once:
  //cau lenh mac dinh de chay duoc ham print ben duoi
  // Serial.begin(115200);
  // Serial.println("Hello IOT");

  pinMode(LED_PIN, OUTPUT);

}

//Lặp chạy mãi mãi
void loop() {
  // put your main code here, to run repeatedly:
  // static int i = 0;
  // ulong t = millis();
  // Serial.print(t);
  // Serial.print(" Loop time: ");
  // Serial.println(i++);
  // delay(1000);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}

