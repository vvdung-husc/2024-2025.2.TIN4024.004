#include <Arduino.h>
#include <TM1637Display.h>

// Khai báo chân LED
#define rLED  23
#define yLED  22
#define gLED  21

// Khai báo chân TM1637
#define CLK 15
#define DIO 2

// Thời gian cho từng đèn (milliseconds)
uint rTIME = 5000;   // 5 giây
uint yTIME = 3000;   // 3 giây
uint gTIME = 7000;   // 7 giây

// Biến thời gian
ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = rLED;
int timeLeft = 0;  // Biến lưu thời gian đếm ngược

// Khởi tạo TM1637
TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();

void setup() {
  Serial.begin(115200);

  // Cấu hình chân LED
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);

  // Tắt tất cả đèn, bật đèn đỏ đầu tiên
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  
  currentLED = rLED;
  nextTimeTotal += rTIME;
  timeLeft = rTIME / 1000; // Chuyển đổi sang giây

  // Khởi tạo TM1637
  display.setBrightness(7); // Độ sáng tối đa (0-7)
  display.showNumberDec(timeLeft);

  Serial.println("== START ==>");  
  Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal/1000); Serial.println(" (s)");
}

void loop() {
  currentMiliseconds = millis();
  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();
}

// Hàm kiểm tra thời gian đã trôi qua hay chưa
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
        nextTimeTotal += gTIME;
        timeLeft = gTIME / 1000; // Đếm ngược
        Serial.print("2. GREEN  => YELLOW "); Serial.print(nextTimeTotal/1000); Serial.println(" (s)");
      } 
      break;

    case gLED: // Đèn xanh
      if (IsReady(ledTimeStart, gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        timeLeft = yTIME / 1000; // Đếm ngược
        Serial.print("3. YELLOW => RED    "); Serial.print(nextTimeTotal/1000); Serial.println(" (s)");        
      }
      break;

    case yLED: // Đèn vàng
      if (IsReady(ledTimeStart, yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        timeLeft = rTIME / 1000; // Đếm ngược
        Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal/1000); Serial.println(" (s)");        
      }
      break;
  }  
}

// Hàm hiển thị đếm ngược trên TM1637
void NonBlocking_Traffic_Light_TM1637() {
  static ulong lastUpdate = 0;

  if (millis() - lastUpdate >= 1000) { // Cập nhật mỗi giây
    lastUpdate = millis();
    if (timeLeft > 0) {
      timeLeft--;
    }
    display.showNumberDec(timeLeft);
  }
}
