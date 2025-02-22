#include <Arduino.h>

//Pin
#define rLED  23
#define yLED  22
#define gLED  21

//1000 ms = 1 seconds
uint rTIME = 5000;   //5 seconds
uint yTIME = 3000;
uint gTIME = 7000;

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = rLED;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  currentLED = rLED;
  nextTimeTotal += rTIME;
  
  Serial.println("== START ==>");  
  Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal/1000); Serial.println(" (ms)");

}

void loop() {
  // put your main code here, to run repeatedly:
  currentMiliseconds = millis();
  NonBlocking_Traffic_Light();
  //NonBlocking_Traffic_Light_TM1637();
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
        Serial.print("2. GREEN  => YELLOW "); Serial.print(nextTimeTotal/1000); Serial.println(" (ms)");
      } 
      break;

    case gLED: // Đèn xanh: 7 giây
      if (IsReady(ledTimeStart,gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        nextTimeTotal += yTIME;
        Serial.print("3. YELLOW => RED    "); Serial.print(nextTimeTotal/1000); Serial.println(" (ms)");        
      }
      break;

    case yLED: // Đèn vàng: 2 giây
      if (IsReady(ledTimeStart,yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        nextTimeTotal += rTIME;
        Serial.print("1. RED    => GREEN  "); Serial.print(nextTimeTotal/1000); Serial.println(" (ms)");        
      }
      break;
  }  
}

void NonBlocking_Traffic_Light_TM1637(){

}