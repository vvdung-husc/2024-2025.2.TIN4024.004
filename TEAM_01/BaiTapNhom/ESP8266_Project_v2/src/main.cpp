#define BLYNK_TEMPLATE_ID "TMPL6edj9LqxJ"
#define BLYNK_TEMPLATE_NAME "TeamProject02"
#define BLYNK_AUTH_TOKEN "w1ceRxYKYOJ6IVG_54mUkTdrNtkVONOR"


#include <Arduino.h>
#include "utils.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>

#include <Wire.h>
#include <U8g2lib.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#include <BlynkSimpleEsp8266.h>

char ssid[] = "CNTT-MMT";  //Tên mạng WiFi
char pass[] = "13572468";  //Mật khẩu mạng WiFi

// 🔗 Token của Telegram Bot (lấy từ BotFather)
#define BOTtoken "7511280600:AAGH0_yT3YVbBhdzsufSaLprwsPPoCQvmUs"

// 🆔 ID của nhóm Telegram (Thường là số âm)
#define GROUP_ID "-1002679961618"

#define gPIN 15        // Đèn xanh
#define yPIN 2         // Đèn vàng
#define rPIN 5         // Đèn đỏ
#define switchPIN 14   // Công tắc chuyển chế độ nhấp nháy
#define motionSensor 12  // Cảm biến chuyển động (GPIO12 - D6)

#define OLED_SDA 13
#define OLED_SCL 12

//Telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

DHT dht(D0, DHT11);

bool yellowBlinkMode = false; // Biến trạng thái chế độ đèn vàng nhấp nháy

float temperature = 0.0;
float humidity = 0.0;

bool botActive = true;
bool motionDetected = false;

unsigned long lastAlertTime = 0;  // Lưu thời gian gửi cảnh báo
unsigned long alertInterval = 300000;  // 5 phút = 300000 ms (5 * 60 * 1000)


// Hàm tạo số ngẫu nhiên trong khoảng min-max
float randomFloat(float minVal, float maxVal) {
    return minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
}

//Khi phát hiện chuyển động
void IRAM_ATTR detectsMovement() {
    motionDetected = true;
}

// Cập nhật giá trị nhiệt độ & độ ẩm (giả lập)
void updateDHT() {
    static ulong lastTimer = 0;
    if (!IsReady(lastTimer, 2000)) return; // Cập nhật mỗi 2 giây

    temperature = randomFloat(-50.0, 100.0);
    humidity = randomFloat(0.0, 100.0);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Gửi dữ liệu lên Blynk
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);
}

// Hàm hiển thị thông tin lên OLED
void updateOLED() {
    oled.clearBuffer();
    oled.setFont(u8g2_font_unifont_t_vietnamese2);

    // Hiển thị nhiệt độ
    String tempString = String("Nhiet do: ") + String(temperature, 2) + " °C";
    oled.drawUTF8(0, 20, tempString.c_str());

    // Hiển thị độ ẩm
    String humString = String("Do am: ") + String(humidity, 2) + " %";
    oled.drawUTF8(0, 40, humString.c_str());

    oled.sendBuffer();
}

// Điều khiển đèn giao thông
void updateTrafficLight() {
    static ulong lastTimer = 0;
    if (!IsReady(lastTimer, 1000)) return; // Chuyển đèn mỗi giây

    if (yellowBlinkMode) {
        digitalWrite(gPIN, LOW);
        digitalWrite(rPIN, LOW);
        digitalWrite(yPIN, !digitalRead(yPIN)); // Nhấp nháy đèn vàng mỗi 500ms
    } else {
        static int currentLed = 0;
        static const int ledPins[3] = {gPIN, yPIN, rPIN};

        digitalWrite(ledPins[(currentLed + 2) % 3], LOW); // Tắt đèn trước đó
        digitalWrite(ledPins[currentLed], HIGH); // Bật đèn hiện tại
        currentLed = (currentLed + 1) % 3;
    }
}

// Kiểm tra công tắc nhấp nháy
void checkSwitch() {
    static bool lastSwitchState = HIGH;
    bool currentSwitchState = digitalRead(switchPIN);

    if (currentSwitchState == LOW && lastSwitchState == HIGH) { // Khi nhấn công tắc
        yellowBlinkMode = !yellowBlinkMode;
        digitalWrite(yPIN, LOW); // Đảm bảo tắt đèn vàng trước khi nhấp nháy
        Serial.println(yellowBlinkMode ? "Yellow light flashing mode ON" : " Traffic mode ON");
    }

    lastSwitchState = currentSwitchState;
}

void checkHealthAlerts() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastAlertTime >= alertInterval) {
      lastAlertTime = currentMillis;

      // Kiểm tra ngưỡng nhiệt độ
      String tempAlert = "";
      if (temperature < 10.0) {
          tempAlert = "🔥 Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch.";
      } else if (temperature >= 10.0 && temperature <= 15.0) {
          tempAlert = "🔥 Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp.";
      } else if (temperature > 35.0) {
          tempAlert = "🔥 Nguy cơ sốc nhiệt, chuột rút, say nắng.";
      } else if (temperature > 40.0) {
          tempAlert = "🔥 Cực kỳ nguy hiểm, có thể gây suy nội tạng, đột quỵ nhiệt.";
      }

      // Kiểm tra ngưỡng độ ẩm
      String humAlert = "";
      if (humidity < 30.0) {
          humAlert = "💦 Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh về hô hấp (viêm họng, khô mũi).";
      } else if (humidity > 70.0) {
          humAlert = "💦 Tăng nguy cơ nấm mốc, vi khuẩn phát triển, gây bệnh về đường hô hấp.";
      } else if (humidity > 80.0) {
          humAlert = "💦 Cảm giác oi bức, khó thở, cơ thể khó thoát mồ hôi, tăng nguy cơ sốc nhiệt.";
      }

      // Nếu có cảnh báo, gửi tin nhắn Telegram
      if (tempAlert != "" || humAlert != "") {
          String message = "🚨 Cảnh báo sức khỏe:\n";
          if (tempAlert != "") {
              message += "🔥 Nhiệt độ: " + String(temperature) + " °C - " + tempAlert + "\n";
          }
          if (humAlert != "") {
              message += "💦 Độ ẩm: " + String(humidity) + " % - " + humAlert + "\n";
          }
          bot.sendMessage(GROUP_ID, message, "Markdown");
          Serial.println("✅ Đã gửi cảnh báo đến Telegram!");
      }
  }
}


// Xử lý tin nhắn từ Telegram
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
    //Gửi cảnh báo đến Telegram mỗi 5 phút một lần nếu gặp trường hợp nhiệt độ hoặc độ ẩm
    //có thể gây nguy hại cho sức khỏe.
    
    //Điều khiển đèn giao thông bằng cách gửi tin nhắn từ Telegram để tắt toàn bộ đèn
    //khi gửi /traffic_off và hoạt động trở lại khi gửi /traffic_on
    
    //Xử lý lệnh /shutdown
    if (text == "/shutdown" || text.startsWith("/shutdown@")) {
        Serial.println("🚨 Nhận lệnh /shutdown -> Ngừng xử lý lệnh...");
        bot.sendMessage(chat_id, "🚨 Bot đã tắt. Gửi `/start` để bật lại!", "Markdown");
        botActive = false;
      }else if (text == "/blink on") {
        yellowBlinkMode = true;
        Blynk.virtualWrite(V3, 1);
        bot.sendMessage(chat_id, "🔸 Chế độ đèn vàng nhấp nháy BẬT");
    } else if (text == "/blink off") {
        yellowBlinkMode = false;
        Blynk.virtualWrite(V3, 0);
        bot.sendMessage(chat_id, "🚦 Chế độ giao thông BẬT");
    }
  }
}
//Xử lý command bằng một hàm riêng
void processCommandFromBlynk(String command) {
    bot.sendMessage(GROUP_ID, "📩 Lệnh từ Blynk: " + command);
    Serial.println("Thực hiện lệnh từ Blynk: " + command);
    // Thêm các xử lý khác nếu cần
}
// 🔗 Nhận lệnh từ Blynk (Virtual Pin V0)
BLYNK_WRITE(V0) {
    String command = param.asStr(); // Nhận dữ liệu dạng chuỗi
    if (command.length() > 0) {
        processCommandFromBlynk(command); // Gọi hàm xử lý lệnh từ Blynk
    }
}
BLYNK_WRITE(V3) {
  int buttonState = param.asInt();  // Nhận giá trị từ Blynk
  yellowBlinkMode = (buttonState == 1);  // Nếu button = 1 → bật chế độ nhấp nháy
  digitalWrite(yPIN, LOW); // Tắt đèn vàng trước khi bật chế độ nhấp nháy
}

void setup() {
    Serial.begin(115200);

    pinMode(gPIN, OUTPUT);
    pinMode(yPIN, OUTPUT);
    pinMode(rPIN, OUTPUT);
    pinMode(switchPIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

    Serial.print("Connecting to ");Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi
    Serial.println("WiFi connected");
    client.setInsecure(); 

    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    digitalWrite(rPIN, LOW);

    Wire.begin(OLED_SDA, OLED_SCL);
    oled.begin();
    oled.clearBuffer();
    oled.sendBuffer();
    
    Serial.println("System boot complete!");
    bot.sendMessage(GROUP_ID, "🤖 IoT TEAM01 Bot đã khởi động!");
}

void loop() {
    Blynk.run();
    checkSwitch();
    updateTrafficLight();
    updateDHT();
    updateOLED();
    checkHealthAlerts();  // Kiểm tra và gửi cảnh báo sức khỏe
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
