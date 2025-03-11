#include <Arduino.h>
#include <TM1637Display.h>

// Pin
#define rLED  17
#define yLED  16
#define gLED  4

// Pin TM1637
#define CLK 18
#define DIO 5

// Cảm biến ánh sáng (LDR)
#define LDR_PIN 15
#define LDR_THRESHOLD 1000  // Ngưỡng để kích hoạt đèn vàng sáng

// Thời gian đèn
//1000 ms = 1 seconds
uint rTIME = 5000;   //5 seconds
uint yTIME = 3000;
uint gTIME = 7000;
uint blinkInterval = 500; // Thời gian nhấp nháy

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
ulong blinkStartTime = 0;
bool isBlinking = false;
bool yellowState = false;

int currentLED = rLED;
uint remainingTime = 0;

TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();
void Check_Light_Sensor();
void Blink_Yellow_Light();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  
  display.setBrightness(0x0F);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);

  currentLED = rLED;
  nextTimeTotal += rTIME;
  remainingTime = rTIME / 1000;
  
  Serial.println("== START ==>");  
  Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal / 1000); Serial.println(" (ms)");
}

void loop() {
  // put your main code here, to run repeatedly:
  currentMiliseconds = millis();
  Check_Light_Sensor();
  
  if (isBlinking) {
    Blink_Yellow_Light();
  } else {
    NonBlocking_Traffic_Light();
    NonBlocking_Traffic_Light_TM1637();
  }
}

// put function definitions here:
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
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
        Serial.print("2. GREEN  => YELLOW "); Serial.print(nextTimeTotal / 1000); Serial.println(" (ms)");
      } 
      break;

    case gLED: // Đèn xanh: 7 giây
      if (IsReady(ledTimeStart, gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        Serial.print("3. YELLOW => RED    "); Serial.print(nextTimeTotal / 1000); Serial.println(" (ms)");        
      }
      break;

    case yLED: // Đèn vàng: 2 giây
      if (IsReady(ledTimeStart, yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal / 1000); Serial.println(" (ms)");        
      }
      break;
  }  
}

void NonBlocking_Traffic_Light_TM1637(){
  remainingTime = (ledTimeStart + (currentLED == rLED ? rTIME : (currentLED == gLED ? gTIME : yTIME)) - currentMiliseconds) / 1000;
  display.showNumberDec(remainingTime, true);
}

void Check_Light_Sensor() {
  int lightValue = analogRead(LDR_PIN);
  Serial.print("LDR Value: "); Serial.println(lightValue);
  
  if (lightValue < LDR_THRESHOLD) {
      if (!isBlinking) {
          isBlinking = true;
          digitalWrite(rLED, LOW);
          digitalWrite(gLED, LOW);
          digitalWrite(yLED, HIGH);  // Đèn vàng sáng liên tục khi ánh sáng yếu
          Serial.println("Low light detected - Yellow Light ON");
      }
  } else {
      if (isBlinking) {
          isBlinking = false;
          digitalWrite(yLED, LOW);
          currentLED = rLED;
          ledTimeStart = millis();
          Serial.println("Light restored - Returning to normal mode");
      }
  }
}

void Blink_Yellow_Light() {
  if (IsReady(blinkStartTime, blinkInterval)) {
      yellowState = !yellowState;
      digitalWrite(yLED, yellowState);
  }
}
