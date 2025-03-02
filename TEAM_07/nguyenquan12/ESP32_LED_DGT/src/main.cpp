#include <Arduino.h>
#include <TM1637Display.h> // Thư viện điều khiển TM1637

// Pin LED
#define rLED  23
#define yLED  21
#define gLED  5

// Chân TM1637
#define CLK 4  // Chân Clock của TM1637
#define DIO 2  // Chân Data của TM1637

// Thời gian đèn (ms)
uint rTIME = 1000;   // 5 giây
uint yTIME = 1000;   // 3 giây
uint gTIME = 1000;   // 7 giây

// Biến thời gian
ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
int currentLED = rLED;
int countdown = rTIME / 1000; // Bắt đầu từ thời gian đèn đỏ

// Khai báo TM1637
TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();

void setup() {
  Serial.begin(115200);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  currentLED = rLED;

  Serial.println("== START ==>");  
  Serial.print("1. RED    => GREEN  "); Serial.print(rTIME / 1000); Serial.println(" (s)");

  // Khởi động TM1637
  display.setBrightness(0x0f); // Độ sáng tối đa
  display.showNumberDec(countdown, true); // Hiển thị thời gian đèn đỏ ban đầu
}

void loop() {
  currentMiliseconds = millis();
  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();
}

// Kiểm tra thời gian không chặn (non-blocking)
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Xử lý đèn giao thông
void NonBlocking_Traffic_Light() {
  switch (currentLED) {
    case rLED: // Đèn đỏ
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        countdown = gTIME / 1000; // Chuyển sang đếm ngược đèn xanh
        Serial.print("2. GREEN  => YELLOW "); Serial.print(gTIME / 1000); Serial.println(" (s)");
      } 
      break;

    case gLED: // Đèn xanh
      if (IsReady(ledTimeStart, gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        countdown = yTIME / 1000; // Chuyển sang đếm ngược đèn vàng
        Serial.print("3. YELLOW => RED    "); Serial.print(yTIME / 1000); Serial.println(" (s)");        
      }
      break;

    case yLED: // Đèn vàng
      if (IsReady(ledTimeStart, yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        countdown = rTIME / 1000; // Chuyển về đếm ngược đèn đỏ
        Serial.print("1. RED    => GREEN  "); Serial.print(rTIME / 1000); Serial.println(" (s)");        
      }
      break;
  }  
}

// Hiển thị thời gian còn lại trên TM1637
void NonBlocking_Traffic_Light_TM1637() {
  static ulong lastUpdate = 0;

  if (millis() - lastUpdate >= 1000) { // Cập nhật mỗi giây
    lastUpdate = millis();
    countdown--; // Giảm thời gian còn lại

    if (countdown < 0) { 
      // Khi hết thời gian, chuyển sang đếm ngược của đèn tiếp theo
      if (currentLED == rLED) countdown = gTIME / 1000;
      else if (currentLED == gLED) countdown = yTIME / 1000;
      else if (currentLED == yLED) countdown = rTIME / 1000;
    }

    // Hiển thị số giây còn lại
    display.showNumberDec(countdown, true);
  }
}