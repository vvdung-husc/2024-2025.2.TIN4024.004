#include <Arduino.h>
#include <TM1637Display.h>

#define rLED 5
#define yLED 17
#define gLED 16
#define extraGLED 4 // Đèn xanh bổ sung
#define buttonPin 12 // PushButton

#define CLK 15
#define DIO 2
#define ldrPIN  13

#define BLYNK_TEMPLATE_ID "TMPL67oSIKeZ_"
#define BLYNK_TEMPLATE_NAME "TRAFFICBLYNK"
#define BLYNK_AUTH_TOKEN "91kc0yWhKvwNWkPtFF4lmGE7vVrxsa9e"

uint rTIME = 5000;
uint yTIME = 3000;
uint gTIME = 10000;

ulong currentMiliseconds = 0;
ulong ledTimeStart = 0;
ulong nextTimeTotal = 0;
int currentLED = 0;
int tmCounter = rTIME / 1000;
ulong counterTime = 0;
int darkThreshold = 1000;
bool isCounting = true;

TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light_TM1637();
bool isDark();
void YellowLED_Blink();
void checkButton();

void setup()
{
  Serial.begin(115200);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(extraGLED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ldrPIN, INPUT);

  tmCounter = (rTIME / 1000) - 1;
  display.setBrightness(7);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  digitalWrite(extraGLED, HIGH);
  display.showNumberDec(tmCounter--, true, 2, 2);

  currentLED = rLED;
  nextTimeTotal += rTIME;
  Serial.println("== START ==>");
}

void loop()
{
  currentMiliseconds = millis();
  checkButton();
  if (isDark()) {
    YellowLED_Blink();
    digitalWrite(extraGLED, LOW); // Tắt đèn xanh khi ở night mode
  }
  else {
    if (isCounting) {
      NonBlocking_Traffic_Light_TM1637();
      digitalWrite(extraGLED, HIGH);
    }
  }
}

void checkButton() {
  if (digitalRead(buttonPin) == LOW) {
    isCounting = !isCounting;
    digitalWrite(extraGLED, isCounting ? HIGH : LOW);
    delay(200);
  }
}

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

void NonBlocking_Traffic_Light_TM1637()
{
  if (!isCounting) return;
  bool bShowCounter = false;
  switch (currentLED)
  {
  case rLED:
    if (IsReady(ledTimeStart, rTIME))
    {
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, HIGH);
      currentLED = gLED;
      nextTimeTotal += gTIME;
      tmCounter = (gTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
    }
    break;
  case gLED:
    if (IsReady(ledTimeStart, gTIME))
    {
      digitalWrite(gLED, LOW);
      digitalWrite(yLED, HIGH);
      currentLED = yLED;
      nextTimeTotal += yTIME;
      tmCounter = (yTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
    }
    break;
  case yLED:
    if (IsReady(ledTimeStart, yTIME))
    {
      digitalWrite(yLED, LOW);
      digitalWrite(rLED, HIGH);
      currentLED = rLED;
      nextTimeTotal += rTIME;
      tmCounter = (rTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
    }
    break;
  }
  if (!bShowCounter)
    bShowCounter = IsReady(counterTime, 1000);
  if (bShowCounter)
  {
    display.showNumberDec(tmCounter--, true, 2, 2);
  }
}

bool isDark()
{
  static ulong darkTimeStart = 0;
  static uint16_t lastValue = 0;
  static bool bDark = false;

  if (!IsReady(darkTimeStart, 50))
    return bDark;
  uint16_t value = analogRead(ldrPIN);
  if (value == lastValue)
    return bDark;

  if (value < darkThreshold)
  {
    if (!bDark)
    {
      digitalWrite(currentLED, LOW);
      Serial.print("DARK value: ");
      Serial.println(value);
    }
    bDark = true;
  }
  else
  {
    if (bDark)
    {
      digitalWrite(yLED, LOW);
      Serial.print("LIGHT value: ");
      Serial.println(value);
    }
    bDark = false;
  }

  lastValue = value;
  return bDark;
}

void YellowLED_Blink()
{
  static ulong yLedStart = 0;
  static bool isON = false;

  if (!IsReady(yLedStart, 1000))
    return;
  if (!isON)
    digitalWrite(yLED, HIGH);
  else
    digitalWrite(yLED, LOW);
  isON = !isON;
}
