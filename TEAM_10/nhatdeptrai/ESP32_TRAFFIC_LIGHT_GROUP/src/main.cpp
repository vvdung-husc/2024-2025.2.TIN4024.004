#include <Arduino.h>
#include <TM1637Display.h>

#define rLED 27
#define yLED 26
#define gLED 25
#define CLK 15
#define DIO 2
#define BTN_PIN 23
#define BLUE_LED 21

TM1637Display display(CLK, DIO);

unsigned long startTime;
unsigned long currentMiliseconds = 0;
unsigned long ledTimeStart = 0;
int currentLED = rLED;

unsigned int rTIME = 5000;
unsigned int yTIME = 3000;
unsigned int gTIME = 7000;
unsigned long timeLeft = rTIME / 1000;

void NonBlocking_Traffic_Light(); // <== THÊM DÒNG NÀY

void setup() {
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BLUE_LED, OUTPUT);

  Serial.begin(115200);

  display.setBrightness(7);
  display.showNumberDec(timeLeft, true);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  digitalWrite(BLUE_LED, LOW);

  currentLED = rLED;
  ledTimeStart = millis();
}

void loop() {
  currentMiliseconds = millis();
  unsigned long elapsedTime = (currentMiliseconds - startTime) / 1000;
  Serial.print("Thoi gian da troi qua: ");
  Serial.print(elapsedTime);
  Serial.println(" giay");
  NonBlocking_Traffic_Light();
}

bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (millis() - ulTimer < milisecond) return false;
  ulTimer = millis();
  return true;
}

void NonBlocking_Traffic_Light() {  // <== HÀM ĐÃ KHAI BÁO ĐÚNG
  if (IsReady(ledTimeStart, 1000)) {
    if (timeLeft > 0) {
      timeLeft--;
      display.showNumberDec(timeLeft, true);
    } else {
      switch (currentLED) {
        case rLED:
          digitalWrite(rLED, LOW);
          digitalWrite(gLED, HIGH);
          currentLED = gLED;
          timeLeft = gTIME / 1000;
          Serial.println("Đổi sang đèn xanh");
          break;
        case gLED:
          digitalWrite(gLED, LOW);
          digitalWrite(yLED, HIGH);
          currentLED = yLED;
          timeLeft = yTIME / 1000;
          Serial.println("Đổi sang đèn vàng");
          break;
        case yLED:
          digitalWrite(yLED, LOW);
          digitalWrite(rLED, HIGH);
          currentLED = rLED;
          timeLeft = rTIME / 1000;
          Serial.println("Đổi sang đèn đỏ");
          break;
      }
    }
  }
}
