#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân LED giao thông
#define rLED 27
#define yLED 26
#define gLED 25
#define led1 21

// Định nghĩa chân TM1637
#define CLK 18
#define DIO 19

// Định nghĩa nút nhấn
#define btn1 23
bool led1State = true; // Mặc định bật (hiển thị đếm ngược)

// Thời gian đèn
uint rTIME = 5000;
uint yTIME = 3000;
uint gTIME = 7000;

ulong currentMillis = 0;
ulong ledTimeStart = 0;
int currentLED = rLED;
int countdownTime = rTIME / 1000; // Thời gian đếm ngược cho đèn đỏ

TM1637Display display(CLK, DIO);

void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();
bool IsReady(ulong &ulTimer, uint32_t milisecond);

void setup()
{
  Serial.begin(115200);

  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(btn1, INPUT_PULLUP); // Nút nhấn sử dụng pull-up

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  digitalWrite(led1, HIGH);

  display.setBrightness(7);
  display.showNumberDecEx(countdownTime, 0x00, true, 2, 2); // Hiển thị ngay số đầu tiên

  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  ");
  Serial.print(rTIME / 1000);
  Serial.println(" (s)");
}

void loop()
{
  currentMillis = millis();

  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();

  // Kiểm tra nút nhấn (chỉ cần bấm 1 lần)
  if (digitalRead(btn1) == LOW)
  {            // Kiểm tra nút nhấn để tắt hiển thị
    delay(50); // Debounce
    if (digitalRead(btn1) == LOW)
    {
      led1State = !led1State;                     // Đảo trạng thái LED1
      digitalWrite(led1, led1State ? HIGH : LOW); // Bật/Tắt LED1
      if (!led1State)
      {                  // Nếu LED1 tắt, xóa màn hình
        display.clear(); // Xóa màn hình TM1637
      }
      else
      {
        // Nếu LED1 bật lại, hiển thị lại thời gian đếm ngược từ giá trị hiện tại
        char buf[3];
        sprintf(buf, "%02d", countdownTime); // Đảm bảo luôn có 2 chữ số
        display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2);
      }
      Serial.println(led1State ? "LED1 ON - Hiển thị TM1637" : "LED1 OFF - Tắt TM1637");
      while (digitalRead(btn1) == LOW)
        ; // Chờ thả nút
    }
  }
}

// Hàm kiểm tra thời gian không chặn chương trình
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMillis - ulTimer < milisecond)
    return false;          // Kiểm tra xem đã đủ thời gian chưa
  ulTimer = currentMillis; // Cập nhật lại thời gian bắt đầu
  return true;             // Đủ thời gian
}

// Hàm điều khiển đèn giao thông không chặn
void NonBlocking_Traffic_Light()
{
  // Đọc giá trị cảm biến LDR
  int ldrValue = analogRead(13); // Đọc giá trị từ LDR
  int threshold = 1000;          // Ngưỡng ánh sáng để quyết định ban ngày hay ban đêm

  if (ldrValue < threshold)
  { // Nếu ánh sáng yếu (ban đêm)
    // Ban đêm: chỉ đèn vàng sáng, các đèn còn lại tắt
    digitalWrite(rLED, LOW);  // Đèn đỏ tắt
    digitalWrite(gLED, LOW);  // Đèn xanh tắt
    digitalWrite(yLED, HIGH); // Đèn vàng sáng

    display.clear();
  }
  else
  {
    // Xử lý các trạng thái đèn giao thông
    switch (currentLED)
    {
    case rLED: // Đèn đỏ
      if (IsReady(ledTimeStart, rTIME))
      { // Kiểm tra thời gian đèn đỏ
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        countdownTime = gTIME / 1000; // Cập nhật thời gian đếm ngược cho đèn xanh

        // Hiển thị thời gian nếu LED1 bật
        if (led1State)
        {
          char buf[3];
          sprintf(buf, "%02d", countdownTime);                  // Đảm bảo luôn có 2 chữ số
          display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên màn hình TM1637
        }
        Serial.print("2. GREEN  => YELLOW ");
        Serial.print(gTIME / 1000);
        Serial.println(" (s)");
      }
      break;

    case gLED: // Đèn xanh
      if (IsReady(ledTimeStart, gTIME))
      { // Kiểm tra thời gian đèn xanh
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        countdownTime = yTIME / 1000; // Cập nhật thời gian đếm ngược cho đèn vàng

        // Hiển thị thời gian nếu LED1 bật
        if (led1State)
        {
          char buf[3];
          sprintf(buf, "%02d", countdownTime);                  // Đảm bảo luôn có 2 chữ số
          display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên màn hình TM1637
        }

        Serial.print("3. YELLOW => RED    ");
        Serial.print(yTIME / 1000);
        Serial.println(" (s)");
      }
      break;

    case yLED: // Đèn vàng
      if (IsReady(ledTimeStart, yTIME))
      { // Kiểm tra thời gian đèn vàng
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        countdownTime = rTIME / 1000; // Cập nhật thời gian đếm ngược cho đèn đỏ

        // Hiển thị thời gian nếu LED1 bật
        if (led1State)
        {
          char buf[3];
          sprintf(buf, "%02d", countdownTime);                  // Đảm bảo luôn có 2 chữ số
          display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên màn hình TM1637
        }

        Serial.print("1. RED    => GREEN  ");
        Serial.print(rTIME / 1000);
        Serial.println(" (s)");
      }
      break;
    }
  }
}

// Hiển thị đếm ngược trên TM1637
void NonBlocking_Traffic_Light_TM1637()
{
  static ulong lastUpdate = 0;

  if (millis() - lastUpdate >= 1000)
  { // Cập nhật mỗi giây
    lastUpdate = millis();

    if (countdownTime > 0)
    {
      countdownTime--; // Giảm thời gian đếm ngược

      // Nếu LED1 bật, hiển thị thời gian
      if (led1State)
      {
        int displayValue = countdownTime % 100; // Lấy 2 chữ số cuối của thời gian
        char buf[3];
        sprintf(buf, "%02d", displayValue);                   // Đảm bảo luôn có 2 chữ số
        display.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên màn hình TM1637
      }
      else
      {
        display.clear(); // Nếu LED1 tắt, xóa màn hình TM1637
      }
    }
  }
}