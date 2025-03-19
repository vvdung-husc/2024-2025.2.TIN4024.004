#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Wokwi-GUEST";  // Thay thế bằng tên WiFi của bạn
const char* password = "";         // Thay thế bằng mật khẩu WiFi của bạn

// Initialize Telegram BOT
#define BOTtoken "8077551853:AAG0hT1ZgPslaKuP4jh10I_0r1TAZJpe53w"  // Thay thế bằng Token Bot Telegram của bạn (lấy từ BotFather)
#define GROUP_ID "-4682103420" // Thay thế bằng Chat ID của nhóm (thường là số âm)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; // PIR Motion Sensor
bool motionDetected = false;

// Định dạng chuỗi %s,%d,...
String StringFormat(const char* fmt, ...) {
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// Interrupt function for motion detection
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Connecting to WiFi
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nWiFi connected");

  // Thông báo khi chương trình bắt đầu chạy
  String startupMsg = "The IoT Developer system has started and is ready to detect motion.";
  bot.sendMessage(GROUP_ID, startupMsg);
  Serial.println("Startup message sent to Telegram.");
}

void loop() {
  static uint count_ = 0;

  if (motionDetected) {
    ++count_;
    Serial.printf("%u. MOTION DETECTED => Waiting to send to Telegram\n", count_);
    String msg = StringFormat("%u => Motion detected!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.printf("%u. Sent successfully to Telegram: Motion Detected\n", count_);
    motionDetected = false;
  }
}
