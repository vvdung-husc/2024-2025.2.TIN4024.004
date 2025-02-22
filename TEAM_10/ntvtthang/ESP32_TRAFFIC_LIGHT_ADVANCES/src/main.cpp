#include <Arduino.h>
#include <TM1637Display.h>

#define rLED 23
#define yLED 22
#define gLED 21
#define CLK 15
#define DIO 2

TM1637Display display(CLK, DIO);

unsigned long startTime;
unsigned long currentMiliseconds = 0;
unsigned long ledTimeStart = 0;
unsigned long lastSerialPrint = 0;
int currentLED = rLED;

unsigned int rTIME = 5; // Thời gian đèn đỏ (giây)
unsigned int yTIME = 3; // Thời gian đèn vàng (giây)
unsigned int gTIME = 7; // Thời gian đèn xanh (giây)
unsigned long timeLeft;

bool IsReady(unsigned long &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();

void setup()
{
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  Serial.begin(115200);
  startTime = millis();

  display.setBrightness(7);
  
  // Bắt đầu với đèn đỏ
  timeLeft = rTIME;
  display.showNumberDec(timeLeft, true);
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  currentLED = rLED;
  ledTimeStart = millis();

  Serial.println("== START ==>");
}

void loop()
{
  currentMiliseconds = millis();

  // Giảm tần suất in Serial Monitor để tránh spam
  if (millis() - lastSerialPrint >= 1000)
  {
    Serial.print("Thời gian đã trôi qua: ");
    Serial.print((currentMiliseconds - startTime) / 1000);
    Serial.println(" giây");
    lastSerialPrint = millis();
  }

  NonBlocking_Traffic_Light();
}

bool IsReady(unsigned long &ulTimer, uint32_t milisecond)
{
  if ((millis() - ulTimer) >= milisecond)
  {
    ulTimer = millis();
    return true;
  }
  return false;
}

void NonBlocking_Traffic_Light()
{
  if (IsReady(ledTimeStart, 1000)) // Kiểm tra mỗi giây
  {
    if (timeLeft > 0)
    {
      timeLeft--;
      display.showNumberDec(timeLeft, true);
    }
    else
    {
      switch (currentLED)
      {
      case rLED:
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        timeLeft = gTIME;
        Serial.println("Đổi sang đèn xanh");
        break;

      case gLED:
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        timeLeft = yTIME;
        Serial.println("Đổi sang đèn vàng");
        break;

      case yLED:
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        timeLeft = rTIME;
        Serial.println("Đổi sang đèn đỏ");
        break;
      }
    }
  }
}