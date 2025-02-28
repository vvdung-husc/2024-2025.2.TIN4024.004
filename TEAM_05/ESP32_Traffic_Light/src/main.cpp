#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân kết nối phần cứng
#define PIN_GREEN_LED 25    // Chân đèn xanh
#define PIN_YELLOW_LED 26   // Chân đèn vàng
#define PIN_RED_LED 27      // Chân đèn đỏ
#define PIN_BLUE_LED 21     // Chân đèn xanh dương (bật khi màn hình hoạt động)
#define PIN_BUTTON 23       // Chân nút nhấn để bật/tắt màn hình
#define PIN_LIGHT_SENSOR 13 // Chân cảm biến ánh sáng
#define PIN_DISPLAY_CLK 18  // Chân CLK của màn hình TM1637
#define PIN_DISPLAY_DIO 19  // Chân DIO của màn hình TM1637

// Ngưỡng ánh sáng để chuyển chế độ ngày/đêm
#define LIGHT_THRESHOLD 1000 // Dưới 1000 là đêm, trên 1000 là ngày

// Khởi tạo màn hình TM1637
TM1637Display display(PIN_DISPLAY_CLK, PIN_DISPLAY_DIO);

// Các trạng thái đèn giao thông
enum TrafficState
{
  GREEN,
  YELLOW,
  RED
}; // Xanh, Vàng, Đỏ
TrafficState currentState = GREEN; // Trạng thái hiện tại, bắt đầu là xanh

// Thời gian cho từng trạng thái (giây)
const int GREEN_TIME = 10; // Thời gian đèn xanh
const int YELLOW_TIME = 3; // Thời gian đèn vàng
const int RED_TIME = 5;    // Thời gian đèn đỏ

// Biến toàn cục
int timeLeft;             // Thời gian còn lại của trạng thái hiện tại
bool isDisplayOn = true;  // Màn hình bật (true) hay tắt (false)
bool isNightMode = false; // Chế độ đêm (true) hay ngày (false)

// Hàm bật/tắt đèn giao thông dựa trên trạng thái
void controlLights()
{
  if (isNightMode)
  {
    // Chế độ đêm: chỉ bật đèn vàng
    digitalWrite(PIN_GREEN_LED, LOW);
    digitalWrite(PIN_YELLOW_LED, HIGH);
    digitalWrite(PIN_RED_LED, LOW);
  }
  else
  {
    // Chế độ ngày: bật đèn theo trạng thái hiện tại
    digitalWrite(PIN_GREEN_LED, currentState == GREEN);
    digitalWrite(PIN_YELLOW_LED, currentState == YELLOW);
    digitalWrite(PIN_RED_LED, currentState == RED);
  }
}

// Hàm hiển thị thời gian lên màn hình
void updateDisplay()
{
  if (isNightMode)
  {
    // Chế độ đêm: hiển thị "0.0"
    display.showNumberDecEx(0, 0b01000000, true, 2, 2); // Hiển thị "0.0"
  }
  else if (isDisplayOn)
  {
    // Chế độ ngày + màn hình bật: hiển thị thời gian còn lại
    display.showNumberDec(timeLeft, true, 2 , 2);
  }
  // else
  // {
  //   // Chế độ ngày + màn hình tắt: xóa màn hình
  //   display.clear();
  // }
  digitalWrite(PIN_BLUE_LED, isDisplayOn); // Đèn xanh dương bật khi màn hình hiển thị
}

// Hàm xử lý nút nhấn để bật/tắt màn hình
void handleButton()
{
  static bool lastButtonState = HIGH;         // Trạng thái trước đó của nút
  bool buttonState = digitalRead(PIN_BUTTON); // Đọc trạng thái hiện tại

  if (lastButtonState == HIGH && buttonState == LOW)
  {
    // Nút được nhấn: chuyển từ HIGH sang LOW
    isDisplayOn = !isDisplayOn; // Đổi trạng thái màn hình
    updateDisplay();            // Cập nhật màn hình ngay lập tức
  }
  lastButtonState = buttonState; // Lưu trạng thái hiện tại
}

// Hàm kiểm tra ánh sáng và chuyển chế độ ngày/đêm
void checkLight()
{
  int lightValue = analogRead(PIN_LIGHT_SENSOR); // Đọc giá trị ánh sáng
  bool wasNightMode = isNightMode;               // Lưu trạng thái trước đó
  isNightMode = (lightValue < LIGHT_THRESHOLD);  // Cập nhật chế độ

  if (wasNightMode != isNightMode)
  {
    // Nếu chế độ thay đổi (ngày sang đêm hoặc ngược lại)
    controlLights(); // Cập nhật đèn
    updateDisplay(); // Cập nhật màn hình
  }
}

// Hàm chuyển trạng thái đèn giao thông (chỉ dùng ở chế độ ngày)
void switchState()
{
  if (timeLeft > 0)
  {
    timeLeft--; // Giảm thời gian còn lại
    return;     // Chưa hết thời gian, không đổi trạng thái
  }

  // Hết thời gian, chuyển sang trạng thái tiếp theo
  switch (currentState)
  {
  case GREEN:
    currentState = YELLOW;
    timeLeft = YELLOW_TIME;
    break;
  case YELLOW:
    currentState = RED;
    timeLeft = RED_TIME;
    break;
  case RED:
    currentState = GREEN;
    timeLeft = GREEN_TIME;
    break;
  }
  controlLights(); // Cập nhật đèn khi đổi trạng thái
}

// Hàm thiết lập ban đầu
void setup()
{
  // Cấu hình các chân
  pinMode(PIN_GREEN_LED, OUTPUT);
  pinMode(PIN_YELLOW_LED, OUTPUT);
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_BLUE_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP); // Nút dùng điện trở kéo lên
  pinMode(PIN_LIGHT_SENSOR, INPUT);

  display.setBrightness(7); // Đặt độ sáng màn hình (0-7)
  display.clear();          // Xóa màn hình

  timeLeft = GREEN_TIME; // Bắt đầu với thời gian đèn xanh
  controlLights();       // Bật đèn xanh đầu tiên
  updateDisplay();       // Hiển thị thời gian ban đầu
}

// Hàm vòng lặp chính
void loop()
{
  static unsigned long lastUpdate = 0; // Thời gian cập nhật cuối cùng
  unsigned long now = millis();        // Thời gian hiện tại

  handleButton(); // Xử lý nút nhấn
  checkLight();   // Kiểm tra chế độ ngày/đêm

  // Cập nhật mỗi giây
  if (now - lastUpdate >= 1000)
  {
    lastUpdate = now; // Cập nhật thời gian

    if (isNightMode)
    {
      timeLeft = YELLOW_TIME; // Chế độ đêm: không đếm ngược, giữ đèn vàng
    }
    else
    {
      switchState(); // Chế độ ngày: đếm ngược và chuyển trạng thái
    }
    updateDisplay(); // Cập nhật màn hình
  }
}
