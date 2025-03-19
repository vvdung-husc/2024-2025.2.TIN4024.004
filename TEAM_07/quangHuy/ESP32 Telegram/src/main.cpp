
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram BOT Token (lấy từ BotFather)
#define BOTtoken "7983807519:AAFdWnon-5LQWGGJ7jhRsJmO0_jnc6fnS2k"

// ID người nhận tin nhắn (có thể là user hoặc group)
#define CHAT_ID "7611229584"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int ledPin = 2;  // LED tích hợp trên ESP32 (GPIO 2)
bool ledState = false; // Trạng thái LED

int botRequestDelay = 1000; // Thời gian kiểm tra tin nhắn
unsigned long lastTimeBotRan;

// Kết nối WiFi
void connectWiFi() {
  Serial.print("Đang kết nối WiFi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Chứng chỉ bảo mật

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi đã kết nối!");
}

// Xử lý lệnh từ Telegram
void handleNewMessages(int numNewMessages) {
  Serial.println("Nhận tin nhắn mới từ Telegram");

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    Serial.println("Tin nhắn nhận được: " + text);

    if (chat_id != CHAT_ID) {
      Serial.println("Tin nhắn từ người không hợp lệ!");
      bot.sendMessage(chat_id, "Xin lỗi, bạn không có quyền sử dụng bot này.");
      continue;
    }

    if (text == "/ledon") {
      digitalWrite(ledPin, HIGH);
      ledState = true;
      bot.sendMessage(CHAT_ID, "💡 Đèn đã BẬT!");
    } 
    else if (text == "/ledoff") {
      digitalWrite(ledPin, LOW);
      ledState = false;
      bot.sendMessage(CHAT_ID, "💡 Đèn đã TẮT!");
    } 
    else if (text == "/status") {
      String stateMsg = ledState ? "💡 Đèn đang BẬT!" : "💡 Đèn đang TẮT!";
      bot.sendMessage(CHAT_ID, stateMsg);
    } 
    else {
      bot.sendMessage(CHAT_ID, "❓ Lệnh không hợp lệ! Vui lòng nhập:\n"
                               "/ledon - Bật đèn\n"
                               "/ledoff - Tắt đèn\n"
                               "/status - Kiểm tra trạng thái đèn");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Mặc định tắt LED

  connectWiFi();
  bot.sendMessage(CHAT_ID, "🤖 Bot Telegram đã khởi động! Gõ /ledon, /ledoff hoặc /status.");
}

void loop() {
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
