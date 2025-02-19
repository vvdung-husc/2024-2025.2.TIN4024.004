#include <Arduino.h>
#include <TM1637Display.h>

// Pin đèn LED
#define rLED  5
#define yLED  17
#define gLED  16

// Cấu hình TM1637
#define CLK  15  // Chân CLK của TM1637
#define DIO  2   // Chân DIO của TM1637
TM1637Display display(CLK, DIO);

// Thời gian đèn sáng (ms)
uint rTIME = 5000;  // Đèn đỏ 5 giây
uint yTIME = 3000;  // Đèn vàng 3 giây
uint gTIME = 7000;  // Đèn xanh 7 giây

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = rLED;
uint remainingTime = rTIME / 1000;  // Thời gian còn lại của đèn hiện tại

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();

void setup() {
  Serial.begin(115200);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);

  // Mặc định bật đèn đỏ
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  currentLED = rLED;
  nextTimeTotal += rTIME;
  
  // Cấu hình TM1637
  display.setBrightness(5);
  display.showNumberDec(remainingTime); // Hiển thị số giây ban đầu
  
  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal / 1000); Serial.println(" (s)");
}

void loop() {
  currentMiliseconds = millis();
  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();
}

// Kiểm tra nếu đã hết thời gian của một đèn
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Điều khiển đèn giao thông
void NonBlocking_Traffic_Light() {
  switch (currentLED) {
    case rLED: // Đèn đỏ
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        remainingTime = gTIME / 1000;  // Cập nhật thời gian đèn xanh
        nextTimeTotal += gTIME;
        Serial.print("2. GREEN  => YELLOW "); Serial.print(nextTimeTotal / 1000); Serial.println(" (s)");
      }
      break;

    case gLED: // Đèn xanh
      if (IsReady(ledTimeStart, gTIME)) {
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        remainingTime = yTIME / 1000;  // Cập nhật thời gian đèn vàng
        nextTimeTotal += yTIME;
        Serial.print("3. YELLOW => RED    "); Serial.print(nextTimeTotal / 1000); Serial.println(" (s)");
      }
      break;

    case yLED: // Đèn vàng
      if (IsReady(ledTimeStart, yTIME)) {
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        remainingTime = rTIME / 1000;  // Cập nhật thời gian đèn đỏ
        nextTimeTotal += rTIME;
        Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal / 1000); Serial.println(" (s)");
      }
      break;
  }
}

// Hiển thị số giây trên TM1637
void NonBlocking_Traffic_Light_TM1637() {
  static ulong lastUpdate = 0;
  if (millis() - lastUpdate >= 1000) {  // Cập nhật mỗi giây
    lastUpdate = millis();
    if (remainingTime > 0) {
      remainingTime--;  // Giảm số giây còn lại
      display.showNumberDec(remainingTime);
    }
  }
}
