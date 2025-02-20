#include <Arduino.h>
#include <TM1637Display.h>

//Pin
#define rLED  23
#define yLED  22
#define gLED  21

// Pin TM1637
#define CLK 16
#define DIO 17

TM1637Display display(CLK, DIO);

//1000 ms = 1 seconds
uint rTIME = 5000;   //5 seconds
uint yTIME = 3000;
uint gTIME = 7000;

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = rLED;
int remainingTime = rTIME / 1000; // Thời gian còn lại (giây)

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

  display.setBrightness(7); // Đặt độ sáng tối đa
  display.showNumberDec(remainingTime, false); // Hiển thị thời gian ban đầu
  
  Serial.println("== START ==>");  
  Serial.print("1. RED    => GREEN  "); Serial.println("5s");
}

void loop() {
  currentMiliseconds = millis();
  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();
}

// Kiểm tra xem đã đủ thời gian chưa
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void NonBlocking_Traffic_Light(){
  if (remainingTime == 0) { // Khi hết thời gian, đổi sang đèn tiếp theo
    switch (currentLED) {
      case rLED:
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        nextTimeTotal += gTIME;
        remainingTime = gTIME / 1000;
        Serial.print("2. GREEN  => YELLOW "); Serial.println("7s");
        break;
  
      case gLED:    
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        remainingTime = yTIME / 1000;
        Serial.print("3. YELLOW => RED    "); Serial.println("3s");        
        break;
  
      case yLED:
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        remainingTime += rTIME / 1000;
        Serial.print("1. RED    => GREEN  "); Serial.println("5s");        
        break;
    }
    display.showNumberDec(remainingTime, false); // Hiển thị ngay số mới
    ledTimeStart = millis(); // Reset lại bộ đếm
  }
}

// Hiển thị trên TM1637
void NonBlocking_Traffic_Light_TM1637() {
  static ulong lastUpdate = millis();

  if (millis() - lastUpdate >= 1000) { // Cập nhật mỗi giây
    lastUpdate = millis();

    if (remainingTime > 0) {
      remainingTime--;
      display.showNumberDec(remainingTime, false); // Cập nhật màn hình
    }
  }
}
