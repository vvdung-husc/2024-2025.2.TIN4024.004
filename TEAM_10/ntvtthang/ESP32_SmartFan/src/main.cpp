// Thêm các thư viện cần thiết
#define BLYNK_TEMPLATE_ID "TMPL6Vl3Q9yWk"
#define BLYNK_TEMPLATE_NAME "ESP32SmartFan"
#define BLYNK_AUTH_TOKEN "CST80Y2RQx1F3kD8MkFuIFrHTGLqiZRz"   // Thay bằng Auth Token từ email

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Thông tin WiFi
char ssid[] = "Wokwi-GUEST"; // Tên mạng WiFi
char pass[] = "";            // Mật khẩu mạng WiFi

// Định nghĩa chân và loại cảm biến
#define DHTPIN 4          // Chân GPIO 4 kết nối với DHT22
#define DHTTYPE DHT22     // Loại cảm biến
#define RELAY_PIN 5       // Chân GPIO 5 kết nối với Relay

DHT dht(DHTPIN, DHTTYPE);

// Biến lưu trạng thái quạt
int fanState = 0;

void setup() {
  // Khởi tạo Serial để debug
  Serial.begin(115200);

  // Khởi tạo cảm biến DHT
  dht.begin();

  // Cấu hình chân Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Tắt Relay ban đầu

  // Kết nối với Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connecting to WiFi and Blynk...");
}

// Hàm nhận lệnh từ Blynk để điều khiển quạt
BLYNK_WRITE(V0) {
  fanState = param.asInt(); // Nhận giá trị từ Switch (0 hoặc 1)
  digitalWrite(RELAY_PIN, fanState); // Bật/tắt Relay
  Serial.print("Fan State: ");
  Serial.println(fanState ? "ON" : "OFF");
}

void loop() {
  // Chạy Blynk
  Blynk.run();

  // Đọc nhiệt độ từ DHT22
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Gửi nhiệt độ lên Blynk
  Blynk.virtualWrite(V1, temp);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" °C");

  // Điều khiển quạt tự động dựa trên nhiệt độ (nếu cần)
  if (temp > 30 && fanState == 0) {
    fanState = 1;
    digitalWrite(RELAY_PIN, HIGH);
    Blynk.virtualWrite(V0, 1); // Cập nhật trạng thái trên Blynk
    Serial.println("Fan ON (Auto)");
  } else if (temp < 25 && fanState == 1) {
    fanState = 0;
    digitalWrite(RELAY_PIN, LOW);
    Blynk.virtualWrite(V0, 0); // Cập nhật trạng thái trên Blynk
    Serial.println("Fan OFF (Auto)");
  }

  delay(2000); // Đọc mỗi 2 giây
}