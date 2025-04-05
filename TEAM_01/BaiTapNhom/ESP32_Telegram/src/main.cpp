#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// 🔗 Thông tin WiFi
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";

// 🔗 Token của Telegram Bot (lấy từ BotFather)
#define BOTtoken "7511280600:AAGH0_yT3YVbBhdzsufSaLprwsPPoCQvmUs"

// 🆔 ID của nhóm Telegram (Thường là số âm)
#define GROUP_ID "-1002679961618" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; // Cảm biến chuyển động PIR
bool motionDetected = false;

// 📝 Định dạng chuỗi theo C-Style
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

// 🏃‍♂️ Khi phát hiện chuyển động
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("🔌 Đang kết nối WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n✅ WiFi đã kết nối!");
  bot.sendMessage(GROUP_ID, "🤖 IoT Developer Bot đã khởi động!");
}

// 📩 Xử lý tin nhắn từ Telegram
bool botActive = true;
void handleNewMessages(int numNewMessages) {
  Serial.println("📩 Đang kiểm tra tin nhắn...");

  if (!botActive) {
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = bot.messages[i].chat_id;
      String text = bot.messages[i].text;

      // 📌 Xử lý lệnh /start
      if (text == "/start" || text.startsWith("/start@")) {
        Serial.println("🔄 Nhận lệnh /start -> Bot hoạt động trở lại...");
        bot.sendMessage(chat_id, "✅ Bot đã hoạt động trở lại!", "Markdown");
        botActive = true;
      }
    }
    return; // Không xử lý các lệnh khác khi bot bị tắt
  }

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.println("Tin nhắn nhận được: " + text);

    // 📌 Kiểm tra lệnh /add <member>
    if (text.indexOf("/add ") == 0) {
      String member = text.substring(5); // Cắt phần sau "/add "

      // 🔗 Link mời nhóm (Thay bằng link nhóm thực tế)
      String inviteLink = "https://t.me/+1RvSYqR7j7Y4OTI1"; 

      // 📩 Gửi tin nhắn mời tham gia nhóm
      String msg = "@" + member + " hãy tham gia nhóm qua link sau: " + inviteLink;
      bot.sendMessage(chat_id, msg);
      Serial.println("✅ Đã gửi lời mời đến: " + member);
    }
    if (text == "/help" || text.startsWith("/help@")) {
        Serial.println("🔹 Nhận lệnh /help -> Gửi danh sách lệnh...");
        
        String helpMessage = "🤖 *Danh sách lệnh:*\n";
        helpMessage += "🔹 `/help` - Xem danh sách lệnh\n";
        helpMessage += "🔹 `/add <sdt>` - Mời thành viên vào nhóm\n";
        helpMessage += "🔹 `/status` - Kiểm tra trạng thái bot\n";
  
        bool sent = bot.sendMessage(chat_id, helpMessage, "Markdown");
        if (sent) {
            Serial.println("✅ Đã gửi tin nhắn /help thành công!");
        } else {
            Serial.println("❌ Lỗi: Không gửi được tin nhắn /help!");
        }
    }
    if (text == "/status" || text.startsWith("/status@")) {
        Serial.println("🔹 Nhận lệnh /status -> Kiểm tra trạng thái...");
  
        String statusMessage = "📡 *Trạng thái bot:*\n";
        statusMessage += "🔹 WiFi: " + String(WiFi.SSID()) + " (" + String(WiFi.localIP().toString()) + ")\n";
        statusMessage += "🔹 Kết nối Telegram: " + String(bot.last_message_received > 0 ? "✅ Hoạt động" : "❌ Không có phản hồi") + "\n";
  
        bool sent = bot.sendMessage(chat_id, statusMessage, "Markdown");
  
        Serial.println(sent ? "✅ Đã gửi trạng thái bot!" : "❌ Lỗi gửi tin nhắn /status!");
    }
    //Xử lý lệnh /shutdown
    if (text == "/shutdown" || text.startsWith("/shutdown@")) {
        Serial.println("🚨 Nhận lệnh /shutdown -> Ngừng xử lý lệnh...");
        bot.sendMessage(chat_id, "🚨 Bot đã tắt. Gửi `/start` để bật lại!", "Markdown");
        botActive = false;
      }
  }
}

void loop() {
  static uint count_ = 0;

  if (motionDetected) {
    ++count_;
    Serial.print(count_); Serial.println(". 🔔 Chuyển động phát hiện!");
    String msg = StringFormat("%u => Motion detected!", count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.println("✅ Đã gửi thông báo chuyển động đến Telegram");
    motionDetected = false;
  }

  // 📥 Kiểm tra tin nhắn từ Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    Serial.println("📩 Có tin nhắn mới!");
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  delay(1000); // ⏳ Đợi 1 giây trước khi kiểm tra tiếp
}
