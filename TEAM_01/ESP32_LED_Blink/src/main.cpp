#include <Arduino.h>


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("HELLO IOT");
}

void loop() {
  // put your main code here, to run repeatedly:
  static int i = 0;
  ulong t = millis();
  Serial.print(t);
  Serial.print("Lop tiem: ");
  Serial.println(i++);
  delay(1000);
}
