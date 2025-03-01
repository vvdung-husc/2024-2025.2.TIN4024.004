#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân kết nối LED
#define rLED 5
#define yLED 17
#define gLED 16

// Cấu hình TM1637
#define CLK 15
#define DIO 2
TM1637Display display(CLK, DIO);

// Thời gian bật các đèn (ms)
unsigned int rTIME = 5000;
unsigned int yTIME = 3000;
unsigned int gTIME = 7000;

// Biến điều khiển thời gian
unsigned long currentMilliseconds = 0;
unsigned long ledTimeStart = 0;
unsigned long nextTimeTotal = 0;
int currentLED = rLED;
unsigned int remainingTime = rTIME / 1000;

bool IsReady(unsigned long &ulTimer, uint32_t duration);
void UpdateTrafficLight();
void UpdateDisplay();

void setup()
{
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
  display.setBrightness(7);
  display.showNumberDec(remainingTime);

  Serial.println("== TRAFFIC LIGHT START ==");
  Serial.print("1. RED => GREEN in ");
  Serial.print(nextTimeTotal / 1000);
  Serial.println(" seconds");
}

void loop()
{
  currentMilliseconds = millis();
  UpdateDisplay(); // Cập nhật màn hình trước
  UpdateTrafficLight();
}

// Kiểm tra nếu đã đến thời gian chuyển đèn
bool IsReady(unsigned long &ulTimer, uint32_t duration)
{
  unsigned long elapsedTime = currentMilliseconds - ulTimer;
  if (elapsedTime < duration)
    return false;
  ulTimer = currentMilliseconds;
  return true;
}

// Điều khiển đèn giao thông
void UpdateTrafficLight()
{
  switch (currentLED)
  {
  case rLED: // Đèn đỏ
    if (IsReady(ledTimeStart, rTIME))
    {
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, HIGH);
      currentLED = gLED;
      remainingTime = gTIME / 1000;
      nextTimeTotal += gTIME;
      Serial.print("2. GREEN => YELLOW in ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" seconds");
    }
    break;

  case gLED: // Đèn xanh
    if (IsReady(ledTimeStart, gTIME))
    {
      digitalWrite(gLED, LOW);
      digitalWrite(yLED, HIGH);
      currentLED = yLED;
      remainingTime = yTIME / 1000;
      nextTimeTotal += yTIME;
      Serial.print("3. YELLOW => RED in ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" seconds");
    }
    break;

  case yLED: // Đèn vàng
    if (IsReady(ledTimeStart, yTIME))
    {
      digitalWrite(yLED, LOW);
      digitalWrite(rLED, HIGH);
      currentLED = rLED;
      remainingTime = rTIME / 1000;
      nextTimeTotal += rTIME;
      Serial.print("1. RED => GREEN in ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" seconds");
    }
    break;
  }
}

// Cập nhật số giây còn lại trên TM1637
void UpdateDisplay()
{
  static unsigned long lastUpdate = 0;
  if (currentMilliseconds - lastUpdate >= 1000)
  {
    lastUpdate = currentMilliseconds;
    if (remainingTime > 0)
    {
      remainingTime--;
      display.showNumberDec(remainingTime);
    }
  }
}