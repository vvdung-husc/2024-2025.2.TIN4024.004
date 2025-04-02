#include <WiFi.h>

// Thông tin mạng WiFi
const char *ssid = "Ten_WiFi";     // Tên WiFi
const char *password = "Mat_khau"; // Mật khẩu WiFi

void setup()
{
  Serial.begin(115200); // Bật Serial Monitor để kiểm tra kết nối WiFi
  WiFi.begin(ssid, password);

  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi đã kết nối!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP()); // Hiển thị địa chỉ IP của ESP32
}

void loop()
{
  // Chương trình chính sẽ được viết ở đây
}