// Blynk authentication
#define BLYNK_TEMPLATE_ID "TMPL6uZv-JegF"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "BPRnO3B4iFlsVNrFRKtygQwcjFFhXNS1"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>



// Khai báo chân LED
#define gPIN 15  // LED xanh
#define yPIN 2   // LED vàng
#define rPIN 5   // LED đỏ

// Khai báo chân cảm biến DHT11
#define dhtPIN 4    // Đổi từ GPIO16 sang GPIO4 (D2 trên NodeMCU)
#define dhtTYPE DHT11

// Khai báo chân OLED
#define OLED_SDA 13
#define OLED_SCL 12

// Thông tin WiFi
#define WIFI_SSID "3567"
#define WIFI_PASS "12345678"

// Khởi tạo đối tượng
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
DHT dht(dhtPIN, dhtTYPE);
BlynkTimer timer;

// Initialize Telegram BOT
#define BOTtoken "7261282557:AAGoLbHh6Z6xmp-b8KwPVN89zIXYuMK2VpY"  // your Bot Token (Get from Botfather)

// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "-4710463542" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Biến lưu trạng thái
bool yellowBlink = false;
unsigned long startTime;
float fHumidity = 0.0;
float fTemperature = 0.0;
unsigned long lastAlertTime = 0;
const unsigned long alertInterval = 5 * 60 * 1000; // 5 phút
bool trafficLightsEnabled = true; // Mặc định đèn giao thông hoạt động



// Xử lý nút trên Blynk (V0 bật/tắt LED vàng)
BLYNK_WRITE(V3) {
  yellowBlink = param.asInt();
}

// Khai báo trước các hàm
void ThreeLedBlink();
void updateDHT();
void sendUptimeToBlynk();
void checkWiFiConnection();

void setup() {
  Serial.begin(115200);

  // Cấu hình chân đầu ra
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  // Khởi động OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "HN Thành CNPM");
  oled.sendBuffer();

  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  checkWiFiConnection();

  // Khởi động Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  dht.begin();
  startTime = millis();

  // Thiết lập timer
  timer.setInterval(1000L, ThreeLedBlink);
  timer.setInterval(2000L, updateDHT);
  timer.setInterval(5000L, sendUptimeToBlynk);
}

void checkAndSendAlert() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastAlertTime < alertInterval) {
    return; // Chỉ gửi cảnh báo mỗi 5 phút
  }

  String alertMessage = "";

  // Kiểm tra ngưỡng nhiệt độ
  if (fTemperature < 10) {
    alertMessage += "⚠️ Cảnh báo: Nhiệt độ quá thấp (<10°C) - Nguy cơ hạ thân nhiệt!\n";
  } else if (fTemperature >= 10 && fTemperature < 15) {
    alertMessage += "⚠️ Cảnh báo: Nhiệt độ thấp (10-15°C) - Nguy cơ mắc bệnh hô hấp!\n";
  } else if (fTemperature >= 30 && fTemperature <= 35) {
    alertMessage += "⚠️ Cảnh báo: Nhiệt độ cao (30-35°C) - Cơ thể mất nước, mệt mỏi!\n";
  } else if (fTemperature > 35 && fTemperature <= 40) {
    alertMessage += "⚠️ Cảnh báo: Nhiệt độ nguy hiểm (>35°C) - Nguy cơ say nắng, chuột rút!\n";
  } else if (fTemperature > 40) {
    alertMessage += "🚨 Nguy hiểm: Nhiệt độ cực cao (>40°C) - Nguy cơ suy nội tạng, đột quỵ nhiệt!\n";
  }

  // Kiểm tra ngưỡng độ ẩm
  if (fHumidity < 30) {
    alertMessage += "⚠️ Cảnh báo: Độ ẩm thấp (<30%) - Da khô, kích ứng mắt, tăng nguy cơ bệnh hô hấp!\n";
  } else if (fHumidity > 70 && fHumidity <= 80) {
    alertMessage += "⚠️ Cảnh báo: Độ ẩm cao (>70%) - Tăng nguy cơ nấm mốc, vi khuẩn phát triển!\n";
  } else if (fHumidity > 80) {
    alertMessage += "🚨 Nguy hiểm: Độ ẩm cực cao (>80%) - Cảm giác oi bức, khó thở, nguy cơ sốc nhiệt!\n";
  }

  // Nếu có cảnh báo, gửi lên Telegram
  if (alertMessage.length() > 0) {
    alertMessage = "📢 [CẢNH BÁO]\n" + alertMessage;
    bot.sendMessage(GROUP_ID, alertMessage, "");
    Serial.println("✅ Đã gửi cảnh báo đến Telegram!");
    lastAlertTime = currentTime; // Cập nhật thời gian gửi cảnh báo cuối cùng
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    Serial.println("📩 Tin nhắn mới từ Telegram: " + text);

    if (text == "/traffic_off") {
      trafficLightsEnabled = false;
      digitalWrite(gPIN, LOW);
      digitalWrite(yPIN, LOW);
      digitalWrite(rPIN, LOW);
      bot.sendMessage(chat_id, "🚦 Đèn giao thông đã tắt!", "");
    } 
    else if (text == "/traffic_on") {
      trafficLightsEnabled = true;
      bot.sendMessage(chat_id, "🚦 Đèn giao thông hoạt động trở lại!", "");
    }
  }
}

// Hàm kiểm tra kết nối WiFi
void checkWiFiConnection() {
  Serial.print("Đang kết nối WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi đã kết nối!");
  } else {
    Serial.println("\n❌ Không thể kết nối WiFi!");
  }

  client.setInsecure(); // Bỏ qua chứng chỉ SSL
  if (bot.sendMessage(GROUP_ID, "🤖 Bot đã kết nối thành công!", "")) {
    Serial.println("✅ Đã gửi thông báo đến Telegram!");
  } else {
    Serial.println("❌ Lỗi khi gửi tin nhắn Telegram!");
  }
}

// Điều khiển LED nhấp nháy
void ThreeLedBlink() {

  if (!trafficLightsEnabled) return; // Nếu đèn đang tắt, thoát khỏi hàm

  static int currentLed = 0;
  static unsigned long lastTimer = 0;

  if (yellowBlink) {
    if (millis() - lastTimer >= 500) {
      digitalWrite(gPIN, LOW);
      digitalWrite(rPIN, LOW);
      digitalWrite(yPIN, !digitalRead(yPIN)); // Nhấp nháy LED vàng
      lastTimer = millis();
    }
  } else {
    static const int ledPin[3] = {gPIN, yPIN, rPIN};
    if (millis() - lastTimer >= 1000) {
      digitalWrite(ledPin[(currentLed + 2) % 3], LOW);
      digitalWrite(ledPin[currentLed], HIGH);
      currentLed = (currentLed + 1) % 3;
      lastTimer = millis();
    }
  }
}

// Cập nhật dữ liệu từ cảm biến DHT11
void updateDHT() {
  //float h = dht.readHumidity();
  //float t = dht.readTemperature();
  float h = random(0,1001) / 10.0;
  float t = random(-400, 801) / 10.0; 

  if (isnan(h) || isnan(t)) {
    Serial.println("❌ Lỗi đọc dữ liệu từ cảm biến DHT!");
    return;
  }

  fTemperature = t;
  fHumidity = h;

  Serial.printf("🌡️ Nhiet do: %.1f°C | 💧 Do am: %.1f%%\n", fTemperature, fHumidity);

  // Hiển thị lên OLED
  oled.clearBuffer();
  oled.setCursor(0, 20);
  oled.printf("Nhiet do: %.1f°C", fTemperature);
  oled.setCursor(0, 40);
  oled.printf("Do am: %.1f%%", fHumidity);
  oled.sendBuffer();

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V1, fTemperature);
  Blynk.virtualWrite(V2, fHumidity);

  // Kiểm tra và gửi cảnh báo nếu cần
  checkAndSendAlert();
}

// Gửi thời gian chạy lên Blynk
void sendUptimeToBlynk() {
  Blynk.virtualWrite(V0, (int)((millis() - startTime) / 1000));
}


// Loop chính
void loop() {
  Blynk.run();
  timer.run();

  if (millis() % 1000 == 0) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages) {
      handleNewMessages(numNewMessages);
    }
  }
}