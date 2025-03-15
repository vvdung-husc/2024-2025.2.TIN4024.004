#include <Arduino.h>
#include <TM1637Display.h>

// Chân LED giao thông
#define R_PIN 27    // Chân cho đèn đỏ
#define Y_PIN 26    // Chân cho đèn vàng
#define G_PIN 25    // Chân cho đèn xanh
#define IND_PIN 21  // Chân cho LED chỉ thị (hiển thị trạng thái)

// Chân TM1637
#define CLK_PIN 18  // Chân clock của TM1637
#define DIO_PIN 19  // Chân dữ liệu của TM1637

// Chân nút nhấn
#define BTN_PIN 23  // Chân nút nhấn
bool dispOn = true; // Trạng thái hiển thị TM1637, mặc định bật

// Thời gian đèn (ms)
uint rTime = 5000;  // Thời gian đèn đỏ
uint yTime = 3000;  // Thời gian đèn vàng
uint gTime = 7000; // Thời gian đèn xanh

ulong now = 0;         // Thời gian hiện tại (millis)
ulong lightStart = 0;  // Thời điểm bắt đầu đèn hiện tại
int curLight = R_PIN;  // Đèn đang hoạt động (mặc định là đỏ)
int secs = rTime / 1000; // Giây đếm ngược, khởi tạo từ đèn đỏ

TM1637Display disp(CLK_PIN, DIO_PIN); // Khởi tạo đối tượng TM1637

// Khai báo hàm
void runTraffic();    // Điều khiển đèn giao thông không chặn
void updateDisp();    // Cập nhật hiển thị đếm ngược trên TM1637
bool isDone(ulong &timer, uint32_t ms); // Kiểm tra thời gian đã hết chưa

void setup()
{
  Serial.begin(115200); // Khởi động Serial với baud rate 115200

  // Cấu hình các chân
  pinMode(R_PIN, OUTPUT);    // Chân đèn đỏ là output
  pinMode(Y_PIN, OUTPUT);    // Chân đèn vàng là output
  pinMode(G_PIN, OUTPUT);    // Chân đèn xanh là output
  pinMode(IND_PIN, OUTPUT);  // Chân LED chỉ thị là output
  pinMode(BTN_PIN, INPUT_PULLUP); // Chân nút nhấn là input với pull-up

  // Trạng thái ban đầu
  digitalWrite(Y_PIN, LOW);   // Tắt đèn vàng
  digitalWrite(G_PIN, LOW);   // Tắt đèn xanh
  digitalWrite(R_PIN, HIGH);  // Bật đèn đỏ
  digitalWrite(IND_PIN, HIGH); // Bật LED chỉ thị

  disp.setBrightness(7); // Đặt độ sáng TM1637 (0-7)
  disp.showNumberDecEx(secs, 0x00, true, 2, 2); // Hiển thị giây ban đầu (2 chữ số, vị trí 2)

  // In thông báo khởi động
  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  ");
  Serial.print(rTime / 1000);
  Serial.println(" (s)");
}

void loop()
{
  now = millis(); // Lấy thời gian hiện tại

  runTraffic(); // Điều khiển đèn giao thông
  updateDisp(); // Cập nhật hiển thị TM1637

  // Xử lý nút nhấn
  if (digitalRead(BTN_PIN) == LOW) // Nếu nút được nhấn
  {
    delay(50); // Chống nhiễu (debounce)
    if (digitalRead(BTN_PIN) == LOW) // Xác nhận lại nút nhấn
    {
      dispOn = !dispOn; // Đảo trạng thái hiển thị
      digitalWrite(IND_PIN, dispOn ? HIGH : LOW); // Bật/tắt LED chỉ thị
      if (!dispOn) // Nếu tắt hiển thị
      {
        disp.clear(); // Xóa màn hình TM1637
      }
      else // Nếu bật hiển thị
      {
        char buf[3];
        sprintf(buf, "%02d", secs); // Chuẩn bị số giây dưới dạng 2 chữ số
        disp.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị lại trên TM1637
      }
      Serial.println(dispOn ? "LED1 ON - Hiển thị TM1637" : "LED1 OFF - Tắt TM1637"); // In trạng thái
      while (digitalRead(BTN_PIN) == LOW) // Chờ thả nút
        ;
    }
  }
}

// Kiểm tra thời gian không chặn
bool isDone(ulong &timer, uint32_t ms)
{
  if (now - timer < ms) // Nếu chưa đủ thời gian
    return false;
  timer = now; // Cập nhật thời gian bắt đầu mới
  return true; // Đã đủ thời gian
}

// Điều khiển đèn giao thông
void runTraffic()
{
  int ldr = analogRead(13); // Đọc giá trị cảm biến ánh sáng (LDR)
  int thresh = 1000;        // Ngưỡng ánh sáng (ban ngày/ban đêm)

  if (ldr < thresh) // Nếu ánh sáng yếu (ban đêm)
  {
    digitalWrite(R_PIN, LOW);  // Tắt đèn đỏ
    digitalWrite(G_PIN, LOW);  // Tắt đèn xanh
    digitalWrite(Y_PIN, HIGH); // Bật đèn vàng
    disp.clear();              // Xóa hiển thị TM1637
  }

  // Chuyển trạng thái đèn giao thông (ban ngày)
  switch (curLight)
  {
  case R_PIN: // Đèn đỏ
    if (isDone(lightStart, rTime)) // Nếu hết thời gian đèn đỏ
    {
      digitalWrite(R_PIN, LOW);  // Tắt đèn đỏ
      digitalWrite(G_PIN, HIGH); // Bật đèn xanh
      curLight = G_PIN;          // Chuyển sang đèn xanh
      secs = gTime / 1000;       // Cập nhật giây cho đèn xanh
      if (dispOn) // Nếu hiển thị bật
      {
        char buf[3];
        sprintf(buf, "%02d", secs); // Chuẩn bị số giây
        disp.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên TM1637
      }
      Serial.print("2. GREEN  => YELLOW ");
      Serial.print(gTime / 1000);
      Serial.println(" (s)");
    }
    break;

  case G_PIN: // Đèn xanh
    if (isDone(lightStart, gTime)) // Nếu hết thời gian đèn xanh
    {
      digitalWrite(G_PIN, LOW);  // Tắt đèn xanh
      digitalWrite(Y_PIN, HIGH); // Bật đèn vàng
      curLight = Y_PIN;          // Chuyển sang đèn vàng
      secs = yTime / 1000;       // Cập nhật giây cho đèn vàng
      if (dispOn) // Nếu hiển thị bật
      {
        char buf[3];
        sprintf(buf, "%02d", secs);
        disp.showNumberDecEx(atoi(buf), 0x00, true, 2, 2);
      }
      Serial.print("3. YELLOW => RED    ");
      Serial.print(yTime / 1000);
      Serial.println(" (s)");
    }
    break;

  case Y_PIN: // Đèn vàng
    if (isDone(lightStart, yTime)) // Nếu hết thời gian đèn vàng
    {
      digitalWrite(Y_PIN, LOW);  // Tắt đèn vàng
      digitalWrite(R_PIN, HIGH); // Bật đèn đỏ
      curLight = R_PIN;          // Chuyển sang đèn đỏ
      secs = rTime / 1000;       // Cập nhật giây cho đèn đỏ
      if (dispOn) // Nếu hiển thị bật
      {
        char buf[3];
        sprintf(buf, "%02d", secs);
        disp.showNumberDecEx(atoi(buf), 0x00, true, 2, 2);
      }
      Serial.print("1. RED    => GREEN  ");
      Serial.print(rTime / 1000);
      Serial.println(" (s)");
    }
    break;
  }
}

// Cập nhật hiển thị đếm ngược trên TM1637
void updateDisp()
{
  static ulong last = 0; // Thời điểm cập nhật cuối cùng

  if (millis() - last >= 1000) // Cập nhật mỗi giây
  {
    last = millis(); // Cập nhật thời gian cuối
    if (secs > 0)    // Nếu còn thời gian
    {
      secs--; // Giảm giây đếm ngược
      if (dispOn) // Nếu hiển thị bật
      {
        int val = secs % 100; // Lấy 2 chữ số cuối
        char buf[3];
        sprintf(buf, "%02d", val); // Chuẩn bị số để hiển thị
        disp.showNumberDecEx(atoi(buf), 0x00, true, 2, 2); // Hiển thị trên TM1637
      }
      else // Nếu hiển thị tắt
      {
        disp.clear(); // Xóa màn hình TM1637
      }
    }
  }
}