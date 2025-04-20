#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi credentials (for PlatformIO)
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// Telegram settings
const char *telegramBotToken = "7690656750:AAGUkvNpbRxwjA2hNM5lC2ClSDe-BXCLVs0";
const char *telegramChatID = "6328731895";

// Pin definitions
#define TRIG_PIN 12   // GPIO 12 (HC-SR04 Trig)
#define ECHO_PIN 14   // GPIO 14 (HC-SR04 Echo)
#define DHT_PIN 13    // GPIO 13 (DHT22)
#define LED_PIN 2     // GPIO 2 (LED)
#define BUZZER_PIN 23 // GPIO 23 (Buzzer)

// DHT22 setup
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// LCD I2C setup (địa chỉ I2C thường là 0x27, nếu không hoạt động thì thử 0x3F)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Thresholds
const float WATER_LEVEL_THRESHOLD = 70.0; // Ngưỡng mực nước (cm), giả sử bể cao 100 cm
const float HUMIDITY_THRESHOLD = 90.0;    // Ngưỡng độ ẩm (%)

// Variables
bool warningSent = false;         // Trạng thái gửi cảnh báo qua Telegram
unsigned long lastUpdateTime = 0; // Thời gian cập nhật cuối cùng
const float TANK_HEIGHT = 100.0;  // Chiều cao bể (cm), giả định

void setup()
{
  // Khởi tạo Serial để debug
  Serial.begin(115200);

  // Khởi tạo các chân
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Khởi tạo DHT22
  dht.begin();

  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  delay(2000);
  lcd.clear();

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

// Hàm gửi tin nhắn qua Telegram
void sendTelegramMessage(String message)
{
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(telegramBotToken) +
               "/sendMessage?chat_id=" + String(telegramChatID) +
               "&text=" + message;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    Serial.println("Telegram message sent: " + message);
  }
  else
  {
    Serial.println("Error sending Telegram message");
  }
  http.end();
}

void loop()
{
  // Đọc dữ liệu từ cảm biến HC-SR04
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;                        // Khoảng cách (cm)
  float waterLevel = TANK_HEIGHT - distance;                    // Mực nước (cm)
  float waterLevelPercent = (waterLevel / TANK_HEIGHT) * 100.0; // Phần trăm

  // Đảm bảo giá trị hợp lý
  if (waterLevelPercent < 0)
    waterLevelPercent = 0;
  if (waterLevelPercent > 100)
    waterLevelPercent = 100;

  // Đọc dữ liệu từ DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Kiểm tra lỗi đọc từ DHT22
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT22!");
    humidity = 0;
    temperature = 0;
  }

  // Hiển thị dữ liệu trên LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water: ");
  lcd.print(waterLevelPercent, 1);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C H:");
  lcd.print(humidity, 1);
  lcd.print("%");

  // In dữ liệu ra Serial để debug
  Serial.print("Water Level: ");
  Serial.print(waterLevelPercent);
  Serial.println("%");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // Kiểm tra ngưỡng và kích hoạt cảnh báo
  bool isFloodRisk = false;
  if (waterLevelPercent > WATER_LEVEL_THRESHOLD)
  {
    // Mực nước vượt ngưỡng
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    isFloodRisk = true;
  }
  else if (humidity > HUMIDITY_THRESHOLD)
  {
    // Độ ẩm cao
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Nhấp nháy LED
    digitalWrite(BUZZER_PIN, HIGH);
    isFloodRisk = true;
  }
  else
  {
    // Không có nguy cơ ngập
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    warningSent = false; // Reset trạng thái gửi cảnh báo
  }

  // Gửi thông báo qua Telegram nếu có nguy cơ ngập
  if (isFloodRisk && !warningSent)
  {
    sendTelegramMessage("Cảnh báo ngập nước! Mực nước: " + String(waterLevelPercent) +
                        "%, Độ ẩm: " + String(humidity) + "%");
    warningSent = true;
  }

  delay(1000); // Cập nhật mỗi 1 giây
}
