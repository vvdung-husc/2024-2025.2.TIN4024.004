#include <Arduino.h>
#include <TM1637Display.h>

// Pin LED
#define rLED  23
#define yLED  22
#define gLED  21

// Pin TM1637
#define CLK 16
#define DIO 17

TM1637Display display(CLK, DIO);

// 1000 ms = 1 giây
uint rTIME = 5000;   // 5 giây
uint yTIME = 3000;   // 3 giây
uint gTIME = 7000;   // 7 giây

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
  
  currentLED = rLED;
  nextTimeTotal += rTIME;

  display.setBrightness(7); // Độ sáng tối đa
  display.showNumberDec(remainingTime, false);

  Serial.println("== START ==>");  
  Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal / 1000); Serial.println(" (s)");
}

void loop() {
  currentMiliseconds = millis();
  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void NonBlocking_Traffic_Light(){
  switch (currentLED) {
    case rLED: // Đèn đỏ: 5 giây
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        nextTimeTotal += gTIME;
        remainingTime = gTIME / 1000;
        Serial.print("2. GREEN  => YELLOW "); Serial.print(nextTimeTotal / 1000); Serial.println(" (s)");
      } 
      break;

    case gLED: // Đèn xanh: 7 giây
      if (IsReady(ledTimeStart, gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        remainingTime = yTIME / 1000;
        Serial.print("3. YELLOW => RED    "); Serial.print(nextTimeTotal / 1000); Serial.println(" (s)");        
      }
      break;

    case yLED: // Đèn vàng: 3 giây
      if (IsReady(ledTimeStart, yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        remainingTime = rTIME / 1000;
        Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal / 1000); Serial.println(" (s)");        
      }
      break;
  }  
}

void NonBlocking_Traffic_Light_TM1637() {
  static ulong lastUpdate = 0;
  
  if (millis() - lastUpdate >= 1000) { // Cập nhật mỗi giây
    lastUpdate = millis();
    
    if (remainingTime > 0) {
      remainingTime--;
    }
    
    display.showNumberDec(remainingTime, false); // Hiển thị thời gian còn lại
  }
}
