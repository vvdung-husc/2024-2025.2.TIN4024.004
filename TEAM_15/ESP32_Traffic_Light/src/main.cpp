#include <Arduino.h>
#include <TM1637Display.h>

// Chân kết nối
#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25
#define BLUE_LED 21
#define BUTTON_PIN 23
#define LDR_PIN 13
#define CLK 18
#define DIO 19

// Cấu hình TM1637
TM1637Display display(CLK, DIO);

// Thời gian đèn (ms)
const uint32_t RED_TIME = 7000;
const uint32_t GREEN_TIME = 5000;
const uint32_t YELLOW_TIME = 3000;

// Biến trạng thái
uint32_t lastMillis = 0;
uint32_t countdown = 0;
uint32_t lastButtonPress = 0;
bool displayEnabled = true; // Mặc định bật bảng đếm lùi
int currentLED = RED_LED;
bool blinkingMode = false; // Chế độ nhấp nháy khi trời tối

// Khai báo hàm
void setTrafficLight(int led, uint32_t time);
void checkLightSensor();
void enableBlinkingMode();
void disableBlinkingMode();
void updateCountdown();
void switchTrafficLight();
void handleButtonPress();

void setup()
{
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  display.setBrightness(7);

  setTrafficLight(RED_LED, RED_TIME);
}

void loop()
{
  checkLightSensor();
  handleButtonPress(); // Xử lý nút bấm

  if (!blinkingMode)
  {
    updateCountdown();
    switchTrafficLight();
  }
}

// Kiểm tra cảm biến ánh sáng
void checkLightSensor()
{
  static uint32_t lightCheckMillis = 0;
  if (millis() - lightCheckMillis < 500)
    return;
  lightCheckMillis = millis();

  int lightLevel = analogRead(LDR_PIN);
  if (lightLevel >= 1000)
  {
    if (!blinkingMode)
    {
      Serial.println("Trời đang TỐI, chuyển sang chế độ nhấp nháy!");
    }
    enableBlinkingMode();
  }
  else if (blinkingMode)
  {
    Serial.println("Trời đang SÁNG, trở lại chế độ bình thường!");

    disableBlinkingMode();
  }
}

// Bật chế độ nhấp nháy khi trời tối
void enableBlinkingMode()
{
  blinkingMode = true;
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  display.clear();
  digitalWrite(YELLOW_LED, millis() % 1000 < 500 ? HIGH : LOW);
}

// Tắt chế độ nhấp nháy khi trời sáng
void disableBlinkingMode()
{
  blinkingMode = false;
  setTrafficLight(RED_LED, RED_TIME);
}

// Chuyển đổi đèn giao thông
void setTrafficLight(int led, uint32_t time)
{
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(led, HIGH);

  currentLED = led;
  countdown = time / 1000;
  lastMillis = millis();

  // Log trạng thái đèn giao thông
  if (led == RED_LED)
  {
    Serial.println("Đèn giao thông: Đỏ - Sáng trong 7 giây");
  }
  else if (led == GREEN_LED)
  {
    Serial.println("Đèn giao thông: Xanh - Sáng trong 5 giây");
  }
  else if (led == YELLOW_LED)
  {
    Serial.println("Đèn giao thông: Vàng - Sáng trong 3 giây");
  }

  if (displayEnabled)
  {
    display.showNumberDec(countdown);
    digitalWrite(BLUE_LED, HIGH); // Đèn xanh dương sáng khi bảng đếm lùi bật
  }
  else
  {
    display.clear();
    digitalWrite(BLUE_LED, LOW); // Đèn xanh dương tắt khi bảng đếm lùi tắt
  }
}

// Cập nhật bộ đếm lùi mỗi giây
void updateCountdown()
{
  if (millis() - lastMillis >= 1000)
  {
    lastMillis += 1000;
    countdown--;

    if (displayEnabled)
    {
      display.showNumberDec(countdown);
    }
  }
}

// Chuyển đổi đèn khi hết thời gian
void switchTrafficLight()
{
  if (countdown == 0)
  {
    if (currentLED == RED_LED)
    {
      setTrafficLight(GREEN_LED, GREEN_TIME);
    }
    else if (currentLED == GREEN_LED)
    {
      setTrafficLight(YELLOW_LED, YELLOW_TIME);
    }
    else if (currentLED == YELLOW_LED)
    {
      setTrafficLight(RED_LED, RED_TIME);
    }
  }
}

// Xử lý nút nhấn bật/tắt bảng đếm lùi
void handleButtonPress()
{
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW && lastButtonState == HIGH && millis() - lastButtonPress > 200)
  {
    displayEnabled = !displayEnabled; // Đảo trạng thái bảng đếm lùi
    lastButtonPress = millis();

    if (displayEnabled)
    {
      Serial.println("Bảng đếm lùi: BẬT");
      display.showNumberDec(countdown);
      digitalWrite(BLUE_LED, HIGH);
    }
    else
    {
      Serial.println("Bảng đếm lùi: TẮT");
      display.clear();
      digitalWrite(BLUE_LED, LOW);
    }
  }

  lastButtonState = buttonState;
}
