#include <Arduino.h>
#include <TM1637Display.h>

// Pin - Các đèn LED
#define rLED 27 // Đèn đỏ
#define yLED 26 // Đèn vàng
#define gLED 25 // Đèn xanh

#define BLUE_LED 21
#define BUTTON_PIN 23

// Pin - TM1637
#define CLK 18 // Clock pin
#define DIO 19 // Data pin

// Pin - Cảm biến quang điện trở (LDR)
#define ldrPIN 13

// Thời gian (ms)
const uint32_t rTIME = 5000;  // Đèn đỏ: 5 giây
const uint32_t yTIME = 3000;  // Đèn vàng: 3 giây
const uint32_t gTIME = 10000; // Đèn xanh: 10 giây

// Biến toàn cục
unsigned long currentMilliseconds = 0; // Thời gian hiện tại (ms)
unsigned long ledTimeStart = 0;        // Thời gian bắt đầu trạng thái LED
unsigned long nextTimeTotal = 0;       // Tổng thời gian để debug
int currentLED = rLED;                 // LED hiện tại đang sáng
int tmCounter = rTIME / 1000;          // Giá trị đếm ngược trên TM1637
unsigned long counterTime = 0;         // Thời gian bắt đầu đếm ngược

const int darkThreshold = 1000; // Ngưỡng ánh sáng: < 1000 là tối

TM1637Display display(CLK, DIO); // Đối tượng TM1637Display

// Hàm nguyên mẫu
bool isReady(unsigned long &timer, uint32_t milliseconds);
void nonBlockingTrafficLight();
bool isDark();
void yellowLEDBlink();

void setup()
{
  Serial.begin(115200);

  // Cấu hình các chân LED
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);

  // Cấu hình chân LDR
  pinMode(ldrPIN, INPUT);

  // Khai báo chân nút nhấn là INPUT_PULLUP(thêm)
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Khởi tạo TM1637
  display.setBrightness(7); // Độ sáng tối đa (0-7)
  tmCounter = (rTIME / 1000) - 1;

  // Trạng thái ban đầu: Đèn đỏ sáng
  digitalWrite(rLED, HIGH);
  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(BLUE_LED, LOW);
  display.showNumberDec(tmCounter, true, 2, 2); // Hiển thị số đếm ngược

  ledTimeStart = millis();
  nextTimeTotal = rTIME;
  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  ");
  Serial.print(nextTimeTotal / 1000);
  Serial.println(" (s)");
}

void loop()
{
  currentMilliseconds = millis();
  if (isDark())
  {
    yellowLEDBlink(); // Nếu trời tối, nhấp nháy đèn vàng
  }
  else
  {
    nonBlockingTrafficLight(); // Chạy đèn giao thông bình thường
  }

  // Đọc trạng thái nút nhấn (LOW khi nhấn do dùng INPUT_PULLUP)
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    // Tắt các đèn LED khác
    digitalWrite(rLED, LOW);
    digitalWrite(yLED, LOW);
    digitalWrite(gLED, LOW);
    // Bật đèn xanh da trời
    digitalWrite(BLUE_LED, HIGH);
  }
  else
  {
    // Tắt đèn xanh da trời khi nút không được nhấn
    digitalWrite(BLUE_LED, LOW);
  }
}

bool isReady(unsigned long &timer, uint32_t milliseconds)
{
  if (currentMilliseconds - timer < milliseconds)
  {
    return false;
  }
  timer = currentMilliseconds;
  return true;
}

void nonBlockingTrafficLight()
{
  bool updateCounter = false;

  switch (currentLED)
  {
  case rLED: // Đèn đỏ
    if (isReady(ledTimeStart, rTIME))
    {
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, HIGH);
      currentLED = gLED;
      nextTimeTotal += gTIME;
      tmCounter = (gTIME / 1000) - 1;
      updateCounter = true;
      counterTime = currentMilliseconds;
      Serial.print("2. GREEN  => YELLOW ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" (s)");
    }
    break;

  case gLED: // Đèn xanh
    if (isReady(ledTimeStart, gTIME))
    {
      digitalWrite(gLED, LOW);
      digitalWrite(yLED, HIGH);
      currentLED = yLED;
      nextTimeTotal += yTIME;
      tmCounter = (yTIME / 1000) - 1;
      updateCounter = true;
      counterTime = currentMilliseconds;
      Serial.print("3. YELLOW => RED    ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" (s)");
    }
    break;

  case yLED: // Đèn vàng
    if (isReady(ledTimeStart, yTIME))
    {
      digitalWrite(yLED, LOW);
      digitalWrite(rLED, HIGH);
      currentLED = rLED;
      nextTimeTotal += rTIME;
      tmCounter = (rTIME / 1000) - 1;
      updateCounter = true;
      counterTime = currentMilliseconds;
      Serial.print("1. RED    => GREEN  ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" (s)");
    }
    break;
  }

  // Cập nhật đồng hồ đếm ngược mỗi giây
  if (!updateCounter)
  {
    updateCounter = isReady(counterTime, 1000);
  }
  if (updateCounter && tmCounter >= 0)
  {
    display.showNumberDec(tmCounter--, true, 2, 2);
  }
}

bool isDark()
{
  static unsigned long darkTimeStart = 0;
  static uint16_t lastValue = 0;
  static bool darkState = false;

  if (!isReady(darkTimeStart, 50))
  { // Đọc cảm biến mỗi 50ms
    return darkState;
  }

  uint16_t value = analogRead(ldrPIN);
  if (value == lastValue)
  {
    return darkState;
  }

  if (value < darkThreshold)
  {
    if (!darkState)
    {
      digitalWrite(currentLED, LOW); // Tắt đèn hiện tại
      Serial.print("DARK  value: ");
      Serial.println(value);
    }
    darkState = true;
  }
  else
  {
    if (darkState)
    {
      digitalWrite(yLED, LOW); // Tắt đèn vàng nếu đang nhấp nháy
      Serial.print("LIGHT value: ");
      Serial.println(value);
    }
    darkState = false;
  }

  lastValue = value;
  return darkState;
}

void yellowLEDBlink()
{
  static unsigned long yLedStart = 0;
  static bool isON = false;

  if (!isReady(yLedStart, 1000))
  { // Nhấp nháy mỗi 1 giây
    return;
  }

  isON = !isON;
  digitalWrite(yLED, isON ? HIGH : LOW);
}