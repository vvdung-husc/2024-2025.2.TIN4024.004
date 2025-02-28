#include <Arduino.h>
#include <TM1637Display.h>

// Pin - Các đèn LED
#define RED_LED 27    // Đèn đỏ
#define YELLOW_LED 26 // Đèn vàng
#define GREEN_LED 25  // Đèn xanh
#define BLUE_LED 21   // Đèn xanh da trời (điều khiển bằng nút nhấn).
#define BUTTON_PIN 23 // Chân của nút nhấn.

// Pin - TM1637
#define CLK 18 // Chân Clock pin
#define DIO 19 // Chân Data pin

// Pin - Cảm biến quang điện trở (LDR)
#define ldrPIN 13

// Thời gian (ms)
const uint32_t RED_TIME = 5000;    // Đèn đỏ: 5 giây
const uint32_t YELLOW_TIME = 3000; // Đèn vàng: 3 giây
const uint32_t GREEN_TIME = 10000; // Đèn xanh: 10 giây

// Biến toàn cục
unsigned long currentMilliseconds = 0; // Thời gian hiện tại (ms)
unsigned long ledTimeStart = 0;        // Thời gian bắt đầu trạng thái LED
unsigned long nextTimeTotal = 0;       // Tổng thời gian để debug
int currentLED = RED_LED;              // LED hiện tại đang sáng
int tmCounter = RED_TIME / 1000;       // Giá trị đếm ngược trên TM1637
unsigned long counteRED_TIME = 0;      // Thời gian bắt đầu đếm ngược

bool blueLedState = false;   // Lưu trạng thái đèn xanh da trời
bool lastButtonState = HIGH; // Lưu trạng thái trước đó của nút nhấn

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
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Cấu hình chân LDR
  pinMode(ldrPIN, INPUT);

  // Khai báo chân nút nhấn là INPUT_PULLUP(thêm)
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Khởi tạo TM1637
  display.setBrightness(7);          // Độ sáng tối đa (0-7)
  tmCounter = (RED_TIME / 1000) - 1; //(RED_TIME / 1000)

  // Trạng thái ban đầu: Đèn đỏ sáng
  digitalWrite(RED_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  display.showNumberDec(tmCounter, true, 2, 2); // Hiển thị số đếm ngược

  ledTimeStart = millis();
  nextTimeTotal = RED_TIME;
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

  bool currentButtonState = digitalRead(BUTTON_PIN);

  // Kiểm tra nếu nút được nhấn (chuyển từ HIGH -> LOW)
  if (currentButtonState == LOW && lastButtonState == HIGH)
  {
    blueLedState = !blueLedState;         // Đảo trạng thái đèn
    digitalWrite(BLUE_LED, blueLedState); // Ghi trạng thái trực tiếp
  }

  lastButtonState = currentButtonState; // Lưu trạng thái nút nhấn
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
  case RED_LED: // Đèn đỏ
    if (isReady(ledTimeStart, RED_TIME))
    {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      currentLED = GREEN_LED;
      nextTimeTotal += GREEN_TIME;
      tmCounter = (GREEN_TIME / 1000) - 1; //(GREEN_TIME / 1000)
      updateCounter = true;
      counteRED_TIME = currentMilliseconds;
      Serial.print("2. GREEN  => YELLOW ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" (s)");
    }
    break;

  case GREEN_LED: // Đèn xanh
    if (isReady(ledTimeStart, GREEN_TIME))
    {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      currentLED = YELLOW_LED;
      nextTimeTotal += YELLOW_TIME;
      tmCounter = (YELLOW_TIME / 1000) - 1; //(YELLOW_TIME / 1000)
      updateCounter = true;
      counteRED_TIME = currentMilliseconds;
      Serial.print("3. YELLOW => RED    ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" (s)");
    }
    break;

  case YELLOW_LED: // Đèn vàng
    if (isReady(ledTimeStart, YELLOW_TIME))
    {
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      currentLED = RED_LED;
      nextTimeTotal += RED_TIME;
      tmCounter = (RED_TIME / 1000) - 1; // (RED_TIME / 1000)
      updateCounter = true;
      counteRED_TIME = currentMilliseconds;
      Serial.print("1. RED    => GREEN  ");
      Serial.print(nextTimeTotal / 1000);
      Serial.println(" (s)");
    }
    break;
  }

  // Cập nhật đồng hồ đếm ngược mỗi giây
  if (!updateCounter)
  {
    updateCounter = isReady(counteRED_TIME, 1000);
  }
  if (updateCounter && tmCounter >= 0) // > 1
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
      digitalWrite(YELLOW_LED, LOW); // Tắt đèn vàng nếu đang nhấp nháy
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
  static unsigned long YELLOW_LEDStart = 0;
  static bool isON = false;

  if (!isReady(YELLOW_LEDStart, 1000))
  { // Nhấp nháy mỗi 1 giây
    return;
  }

  isON = !isON;
  digitalWrite(YELLOW_LED, isON ? HIGH : LOW);
}