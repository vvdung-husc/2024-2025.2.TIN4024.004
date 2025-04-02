#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Ticker.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>

// Khai báo các define
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define RED_LED D5
#define YELLOW_LED D6
#define GREEN_LED D7

#define BLYNK_TEMPLATE_ID "TMPL6dnv6jqMP"
#define BLYNK_TEMPLATE_NAME "ESP8266 Blynk Telegram"
#define BLYNK_FIRMWARE_VERSION "1.0.0"

// Khai báo biến toàn cục
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
char auth[] = "PVSrkog9yqc_F7CtlpAuuMH7aJYxBA0Z";
char ssid[] = "Wokwi-GUEST";
char pass[] = " ";
String telegramToken = "7869624586:AAHbkuMDIIbX8wodUUvGXXh9X1AYr3sXjLU";
int64_t chatId = -4771876874;

WiFiClientSecure client;
UniversalTelegramBot bot(telegramToken, client);
Ticker timer;
bool blinkMode = false;
bool trafficOn = true;
unsigned long uptime = 0;

// Prototype các hàm
void handleTelegramMessage(String message);
void updateUptime();
void checkSensor();
void checkHealthHazard(float temp, float hum);
void trafficLight();

void setup()
{
  Serial.begin(115200);

  // Khởi tạo OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  // Khởi tạo LED
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Kết nối WiFi và Blynk
  Blynk.begin(auth, ssid, pass);

  // Cấu hình Telegram
  client.setInsecure(); // Bỏ qua chứng chỉ SSL

  // Thiết lập timer
  timer.attach(5, checkSensor);       // Kiểm tra cảm biến mỗi 5s
  timer.attach(60, updateUptime);     // Cập nhật thời gian hoạt động mỗi phút
  timer.attach_ms(300, trafficLight); // Điều khiển đèn giao thông
}

void loop()
{
  Blynk.run();

  // Kiểm tra tin nhắn Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages)
  {
    for (int i = 0; i < numNewMessages; i++)
    {
      handleTelegramMessage(bot.messages[i].text);
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void handleTelegramMessage(String message)
{
  if (message == "/traffic_off")
  {
    trafficOn = false;
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    bot.sendMessage(String(chatId), "Đã tắt đèn giao thông", "");
  }
  else if (message == "/traffic_on")
  {
    trafficOn = true;
    bot.sendMessage(String(chatId), "Đã bật đèn giao thông", "");
  }
}

void updateUptime()
{
  uptime++;
  Blynk.virtualWrite(V5, uptime); // Gửi thời gian hoạt động lên Blynk
}

void checkSensor()
{
  // Tạo giá trị ngẫu nhiên cho nhiệt độ và độ ẩm
  float temperature = random(-400, 800) / 10.0;
  float humidity = random(0, 1000) / 10.0;

  // Hiển thị lên OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Nhiet do: " + String(temperature) + " C");
  display.println("Do am: " + String(humidity) + " %");
  display.display();

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);

  // Kiểm tra và gửi cảnh báo
  checkHealthHazard(temperature, humidity);
}

void checkHealthHazard(float temp, float hum)
{
  String warning = "";

  // Kiểm tra nhiệt độ
  if (temp < 10)
  {
    warning += "Cảnh báo: Nhiệt độ thấp (" + String(temp) + "°C) - Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch.\n";
  }
  else if (temp > 35 && temp <= 40)
  {
    warning += "Cảnh báo: Nhiệt độ cao (" + String(temp) + "°C) - Nguy cơ sốc nhiệt, chuột rút, say nắng.\n";
  }
  else if (temp > 40)
  {
    warning += "CẢNH BÁO KHẨN CẤP: Nhiệt độ rất cao (" + String(temp) + "°C) - Có thể gây suy nội tạng, đột quỵ nhiệt.\n";
  }

  // Kiểm tra độ ẩm
  if (hum < 30)
  {
    warning += "Cảnh báo: Độ ẩm thấp (" + String(hum) + "%) - Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh hô hấp.\n";
  }
  else if (hum > 70 && hum <= 80)
  {
    warning += "Cảnh báo: Độ ẩm cao (" + String(hum) + "%) - Tăng nguy cơ nấm mốc, vi khuẩn phát triển.\n";
  }
  else if (hum > 80)
  {
    warning += "CẢNH BÁO KHẨN CẤP: Độ ẩm rất cao (" + String(hum) + "%) - Khó thở, tăng nguy cơ sốc nhiệt.\n";
  }

  if (warning != "")
  {
    bot.sendMessage(String(chatId), warning, "");
  }
}

void trafficLight()
{
  if (!trafficOn)
    return;

  static int state = 0;
  static unsigned long lastChange = 0;

  if (blinkMode)
  {
    // Chế độ nhấp nháy đèn vàng
    if (millis() - lastChange >= 500)
    {
      digitalWrite(YELLOW_LED, !digitalRead(YELLOW_LED));
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      lastChange = millis();
    }
  }
  else
  {
    // Chế độ đèn giao thông bình thường
    switch (state)
    {
    case 0: // Đỏ
      digitalWrite(RED_LED, HIGH);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      if (millis() - lastChange >= 5000)
      {
        state = 1;
        lastChange = millis();
      }
      break;
    case 1: // Xanh
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      if (millis() - lastChange >= 5000)
      {
        state = 2;
        lastChange = millis();
      }
      break;
    case 2: // Vàng
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      if (millis() - lastChange >= 2000)
      {
        state = 0;
        lastChange = millis();
      }
      break;
    }
  }
}

BLYNK_WRITE(V3)
{
  blinkMode = param.asInt();
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V3); // Đồng bộ trạng thái switch từ Blynk
}