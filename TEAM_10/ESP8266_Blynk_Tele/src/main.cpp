#include <Arduino.h>
#include <utils.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <UniversalTelegramBot.h>

//Nguyễn Trần Viết Thắng
#define BLYNK_TEMPLATE_ID "TMPL61bYE-l65"
#define BLYNK_TEMPLATE_NAME "ESP8266BlynkTele"
#define BLYNK_AUTH_TOKEN "5KUjzDBJEUFuI-cDkeTE-A3ziMAvZMQE"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST"; // Tên mạng WiFi
char pass[] = "";            // Mật khẩu mạng WiFi

#define gPIN 15
#define yPIN 2
#define rPIN 5
#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
bool blinkMode = false;  // Controlled by V1 switch
bool trafficEnabled = true;

//Nguyễn Trần Viết Thắng
const char* botToken = "7958454010:AAGYkVfTHc-FgAg-YqOMgIo5CR-DRgvMB-I";
const char* chatID = "-4717614518";

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

ulong uptimeSeconds = 0;

void sendTelegramAlert(String message) {
  bot.sendMessage(chatID, message, "Markdown");
}

void setup() {
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  // Khởi tạo OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();

  // Start the WiFi connection
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Kết nối đến mạng WiFi
  Serial.println();
  Serial.println("WiFi connected");

  client.setInsecure();

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT-NHOM10");
  oled.sendBuffer();
}

// V1 controls blinking mode
BLYNK_WRITE(V1) {
  blinkMode = param.asInt();
}

void ThreeLedBlink() {
  static ulong lastTimer = 0;
  static int currentLed = 0;
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (millis() - lastTimer < 1000 || !trafficEnabled) return;
  lastTimer = millis();

  if (blinkMode) {
    // Blinking mode: only yellow LED blinks
    digitalWrite(gPIN, LOW);
    digitalWrite(rPIN, LOW);
    digitalWrite(yPIN, !digitalRead(yPIN));
  } else {
    // Normal traffic light mode
    digitalWrite(ledPin[(currentLed + 2) % 3], LOW);
    digitalWrite(ledPin[currentLed], HIGH);
    currentLed = (currentLed + 1) % 3;
  }
}

void updateDHT() {
  static ulong lastTimer = 0;
  static ulong lastAlert = 0;
  if (millis() - lastTimer < 2000) return;
  lastTimer = millis();

  float nhietDo = random(-400, 800) / 10.0;  // -40.0 to 80.0°C
  float doAm = random(0, 1000) / 10.0;       // 0.0 to 100.0%

  Serial.printf("Temperature: %.2f °C, Humidity: %.2f %%\n", nhietDo, doAm);
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);
  oled.setCursor(0, 20);
  oled.printf("Nhiet do: %.2f °C", nhietDo);
  oled.setCursor(0, 40);
  oled.printf("Do am: %.2f %%", doAm);
  oled.sendBuffer();

  // Send to Blynk virtual pins
  Blynk.virtualWrite(V2, nhietDo);  // Temperature to V2
  Blynk.virtualWrite(V3, doAm);     // Humidity to V3

  // Telegram alerts every 5 minutes
  if (millis() - lastAlert > 300000) {
    String message = "";
    if (nhietDo < 10) message += "⚠️ Nguy cơ hạ thân nhiệt!\n";
    else if (nhietDo > 35) message += "⚠️ Nguy cơ sốc nhiệt!\n";
    else if (nhietDo > 40) message += "⚠️ Cực kỳ nguy hiểm!\n";

    if (doAm < 30) message += "⚠️ Độ ẩm thấp, nguy cơ bệnh hô hấp!\n";
    else if (doAm > 70) message += "⚠️ Độ ẩm cao, nguy cơ nấm mốc!\n";
    else if (doAm > 80) message += "⚠️ Nguy cơ sốc nhiệt do độ ẩm!\n";

    if (message != "") {
      message = "🚨 Cảnh báo!\n" + message +
                "Nhiệt độ: " + String(nhietDo) + "°C\n" +
                "Độ ẩm: " + String(doAm) + "%";
      sendTelegramAlert(message);
      lastAlert = millis();
    }
  }
}

void checkTelegram() {
  int messageCount = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < messageCount; i++) {
    String text = bot.messages[i].text;
    if (text == "/traffic_off") {
      trafficEnabled = false;
      digitalWrite(gPIN, LOW);
      digitalWrite(yPIN, LOW);
      digitalWrite(rPIN, LOW);
      sendTelegramAlert("🚦 Đèn giao thông đã tắt!");
    } else if (text == "/traffic_on") {
      trafficEnabled = true;
      sendTelegramAlert("🚦 Đèn giao thông hoạt động trở lại!");
    }
  }
}

void updateUptime() {
  static ulong lastUpdate = 0;
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    uptimeSeconds++;

    int hours = uptimeSeconds / 3600;
    int minutes = (uptimeSeconds % 3600) / 60;
    int seconds = uptimeSeconds % 60;

    char uptimeStr[20];
    sprintf(uptimeStr, "%02d:%02d:%02d", hours, minutes, seconds);
    
    Serial.printf("Thời gian hoạt động: %s\n", uptimeStr);
    
    // Send uptime to V0
    Blynk.virtualWrite(V0, uptimeSeconds);
  }
}

void loop() {
  Blynk.run();
  ThreeLedBlink();
  updateDHT();
  checkTelegram();
  updateUptime();
}
