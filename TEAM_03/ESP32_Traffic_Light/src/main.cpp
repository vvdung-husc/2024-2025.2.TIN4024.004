#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân LED giao thông
#define rLED  27  
#define yLED  26  
#define gLED  25  
#define led1  21  

// Định nghĩa chân TM1637
#define CLK  18  
#define DIO  19  

// Định nghĩa nút nhấn
#define btn1 23  
bool led1State = true;  

// Thời gian đèn
uint rTIME = 5000;  
uint yTIME = 3000;  
uint gTIME = 7000;  

ulong currentMillis = 0;
ulong ledTimeStart = 0;
int currentLED = rLED;
int countdownTime = rTIME / 1000;  

TM1637Display display(CLK, DIO);

void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();
bool IsReady(ulong &ulTimer, uint32_t milisecond);

void setup() {
  Serial.begin(115200);
  
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(btn1, INPUT_PULLUP);  

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  digitalWrite(led1, HIGH);
  
  display.setBrightness(7);  
  display.showNumberDec(countdownTime, false, 2, 2);  // ⚡ Hiển thị ngay số đầu tiên

  Serial.println("== START ==>");  
  Serial.print("1. RED    => GREEN  "); Serial.print(rTIME / 1000); Serial.println(" (s)");
}

void loop() {
  currentMillis = millis();
  
  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();

  if (digitalRead(btn1) == LOW) {
    delay(50);  
    if (digitalRead(btn1) == LOW) {
      led1State = !led1State;  
      digitalWrite(led1, led1State ? HIGH : LOW);
      Serial.println(led1State ? "LED1 ON - Hiển thị TM1637" : "LED1 OFF - Tắt TM1637");
      while (digitalRead(btn1) == LOW);  
    }
  }
}

// Hàm kiểm tra thời gian không chặn chương trình
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMillis - ulTimer < milisecond) return false;
  ulTimer = currentMillis;
  return true;
}

// Hàm điều khiển đèn giao thông
void NonBlocking_Traffic_Light() {
  switch (currentLED) {
    case rLED: 
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        countdownTime = gTIME / 1000;
        display.showNumberDec(countdownTime, false, 2, 2);  // ⚡ Hiển thị ngay
        Serial.print("2. GREEN  => YELLOW "); Serial.print(gTIME / 1000); Serial.println(" (s)");
      }
      break;

    case gLED: 
      if (IsReady(ledTimeStart, gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        countdownTime = yTIME / 1000;
        display.showNumberDec(countdownTime, false, 2, 2);  
        Serial.print("3. YELLOW => RED    "); Serial.print(yTIME / 1000); Serial.println(" (s)");        
      }
      break;

    case yLED: 
      if (IsReady(ledTimeStart, yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        countdownTime = rTIME / 1000;
        display.showNumberDec(countdownTime, false, 2, 2);  
        Serial.print("1. RED    => GREEN  "); Serial.print(rTIME / 1000); Serial.println(" (s)");        
      }
      break;
  }  
}

// Hiển thị đếm ngược trên TM1637
void NonBlocking_Traffic_Light_TM1637() {
  static ulong lastUpdate = 0;

  if (millis() - lastUpdate >= 1000) {  
    lastUpdate = millis();
    
    if (countdownTime > 0) {
      countdownTime--;

      if (led1State) {
        int displayValue = countdownTime % 100; 
        char buf[3];
        sprintf(buf, "%02d", displayValue);  // 🛠 Luôn có 2 chữ số
        display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2);  
      } else {
        display.clear();  
      }
    }
  }
}
