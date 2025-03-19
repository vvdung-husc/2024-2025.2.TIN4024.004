#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

// Thông tin kết nối Blynk
#define BLYNK_TEMPLATE_ID "TMPL6P6WadasK"
#define BLYNK_DEVICE_NAME "ESP32GetAPI"
#define BLYNK_AUTH_TOKEN "Zg0YdGHsi7w4znd1eseZNbLdySTaaKrb"

// Thông tin WiFi
const char *ssid = "VNPTHUE";
const char *password = "0336074217";
char auth[] = BLYNK_AUTH_TOKEN;

// Hàm lấy vị trí hiện tại từ API ip4.iothings.vn
void getLocation()
{
  HTTPClient http;
  http.begin("http://ip4.iothings.vn?geo=1");

  int httpCode = http.GET();
  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("Received payload: " + payload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    String ipv4 = doc["ip"].as<String>(); // Lấy địa chỉ IP
    float latitude = doc["lat"];          // Lấy vĩ độ
    float longitude = doc["lon"];         // Lấy kinh độ

    Serial.println("IPv4: " + ipv4);
    Serial.println("Latitude: " + String(latitude));
    Serial.println("Longitude: " + String(longitude));

    String googleMapsLink = "https://www.google.com/maps?q=" + String(latitude) + "," + String(longitude);
    Serial.println("Google Maps: " + googleMapsLink);

    // Gửi dữ liệu vị trí lên Blynk
    Blynk.virtualWrite(V0, ipv4);
    Blynk.virtualWrite(V1, latitude);
    Blynk.virtualWrite(V2, longitude);
    Blynk.virtualWrite(V3, googleMapsLink);

    // Gọi hàm lấy thông tin thời tiết dựa trên vị trí
    getWeather(latitude, longitude);
  }
  else
  {
    Serial.println("Error getting location"); // Báo lỗi nếu không lấy được vị trí
  }

  http.end();
}

// Hàm lấy thông tin thời tiết từ OpenWeatherMap API dựa vào vĩ độ và kinh độ
void getWeather(float lat, float lon)
{
  HTTPClient http;
  String apiKey = "d97bf5a08376db3e053403c07878b03d"; // API key OpenWeatherMap
  String url = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(lat) + "&lon=" + String(lon) + "&appid=" + apiKey;

  http.begin(url);

  int httpCode = http.GET();
  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println("Weather Data: " + payload);

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    float temp = doc["main"]["temp"].as<float>() - 273.15;          // Chuyển đổi nhiệt độ từ Kelvin sang độ C
    String weather = doc["weather"][0]["description"].as<String>(); // Mô tả thời tiết

    Serial.println("Temperature: " + String(temp) + "°C");
    Serial.println("Weather: " + weather);

    // Gửi dữ liệu thời tiết lên Blynk
    Blynk.virtualWrite(V4, temp);
    Blynk.virtualWrite(V5, weather);
  }
  else
  {
    Serial.println("Error getting weather data"); // Báo lỗi nếu không lấy được thông tin thời tiết
  }

  http.end();
}

// Hàm setup() chạy một lần khi ESP32 khởi động
void setup()
{
  Serial.begin(115200); // Khởi tạo Serial để debug

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Gọi hàm lấy thông tin vị trí khi thiết bị khởi động
  getLocation();

  // Kết nối với Blynk
  Blynk.begin(auth, ssid, password);
}

// Hàm loop() chạy liên tục sau khi setup() kết thúc
void loop()
{
  Blynk.run(); // Chạy Blynk để cập nhật dữ liệu từ server
}
