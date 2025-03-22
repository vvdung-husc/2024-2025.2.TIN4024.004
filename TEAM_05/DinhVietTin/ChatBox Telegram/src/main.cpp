#include <Arduino.h>              // Thư viện chuẩn cho Arduino
#include <WiFi.h>                 // Thư viện WiFi cho ESP32
#include <WiFiClientSecure.h>     // Thư viện cho kết nối bảo mật (HTTPS)
#include <UniversalTelegramBot.h> // Thư viện để giao tiếp với bot Telegram
#include <ArduinoJson.h>          // Thư viện xử lý JSON

// Thông tin kết nối WiFi
const char *ssid = "Wokwi-GUEST"; // Tên mạng WiFi của Wokwi
const char *password = "";        // Mật khẩu trống

// Khai báo token của bot Telegram
#define BOTtoken "7869624586:AAHbkuMDIIbX8wodUUvGXXh9X1AYr3sXjLU" // Token nhận từ BotFather

// ID nhóm Telegram
#define GROUP_ID "-4771876874"

WiFiClientSecure client;                    // Tạo client bảo mật để kết nối API Telegram
UniversalTelegramBot bot(BOTtoken, client); // Khởi tạo bot với token và client

const int motionSensor = 27;          // Chân GPIO của cảm biến chuyển động PIR
volatile bool motionDetected = false; // Biến kiểm tra có chuyển động hay không (volatile vì dùng trong ngắt)

// Biến để theo dõi trạng thái và thời gian
bool lastMotionState = false;              // Trạng thái chuyển động trước đó
unsigned long lastMotionTime = 0;          // Thời điểm cuối cùng phát hiện chuyển động
const unsigned long MOTION_TIMEOUT = 2000; // Thời gian chờ trước khi coi là "Chuyển động dừng" (2 giây)

// Hàm định dạng chuỗi theo kiểu printf
String StringFormat(const char *fmt, ...)
{
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char *buff = (char *)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// Hàm xử lý ngắt khi cảm biến phát hiện chuyển động
void IRAM_ATTR detectsMovement()
{
  motionDetected = true; // Đặt cờ báo có chuyển động
}

void setup()
{
  Serial.begin(115200); // Khởi động Serial Monitor với baudrate 115200

  pinMode(motionSensor, INPUT_PULLUP);                                           // Cấu hình chân cảm biến làm đầu vào với điện trở kéo lên
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING); // Ngắt khi có xung lên cao (RISING)

  Serial.print("Đang kết nối WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);                         // Chuyển sang chế độ Station
  WiFi.begin(ssid, password);                  // Kết nối WiFi
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Đặt chứng chỉ bảo mật cho Telegram

  while (WiFi.status() != WL_CONNECTED) // Chờ đến khi kết nối thành công
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nĐã kết nối WiFi thành công"); // Thông báo kết nối thành công
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP()); // In địa chỉ IP để kiểm tra

  // Gửi tin nhắn khởi động
  if (bot.sendMessage(GROUP_ID, "Hệ thống IoT Developer đã khởi động"))
  {
    Serial.println("Đã gửi tin nhắn khởi động thành công lên Telegram");
  }
  else
  {
    Serial.println("Không thể gửi tin nhắn khởi động lên Telegram");
  }
}

void loop()
{
  static uint count_ = 0; // Biến đếm số lần phát hiện chuyển động

  // Kiểm tra nếu có chuyển động (từ ngắt)
  if (motionDetected)
  {
    if (!lastMotionState) // Nếu trước đó không có chuyển động
    {
      ++count_;
      Serial.print(count_);
      Serial.println(". ĐÃ PHÁT HIỆN CHUYỂN ĐỘNG => Đang chờ gửi lên Telegram");
      String msg = StringFormat("%u => Phát hiện chuyển động!", count_);
      if (bot.sendMessage(GROUP_ID, msg.c_str()))
      {
        Serial.print(count_);
        Serial.println(". Đã gửi thành công lên Telegram: Phát hiện chuyển động");
      }
      else
      {
        Serial.println("Không thể gửi tin nhắn lên Telegram: Phát hiện chuyển động");
      }
      lastMotionState = true; // Cập nhật trạng thái
    }
    lastMotionTime = millis(); // Cập nhật thời điểm phát hiện chuyển động
    motionDetected = false;    // Reset cờ ngắt
  }

  // Kiểm tra nếu không còn chuyển động (dựa trên thời gian)
  if (lastMotionState && (millis() - lastMotionTime > MOTION_TIMEOUT))
  {
    if (digitalRead(motionSensor) == LOW) // Xác nhận cảm biến đã quay lại LOW
    {
      Serial.println("CHUYỂN ĐỘNG DỪNG => Đang chờ gửi lên Telegram");
      String msg = StringFormat("%u => Chuyển động đã dừng!", count_);
      if (bot.sendMessage(GROUP_ID, msg.c_str()))
      {
        Serial.println("Đã gửi thành công lên Telegram: Chuyển động đã dừng");
      }
      else
      {
        Serial.println("Không thể gửi tin nhắn lên Telegram: Chuyển động đã dừng");
      }
      lastMotionState = false; // Cập nhật trạng thái
    }
  }

  delay(100); // Tránh đọc quá nhanh
}