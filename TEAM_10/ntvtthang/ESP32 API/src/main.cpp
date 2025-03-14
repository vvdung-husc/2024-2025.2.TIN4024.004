#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define BLYNK_TEMPLATE_ID "TMPL6WQLYM4vE"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "f_anI_owNl3_ctATip2QyQI38resT7a2"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";  // Tên mạng WiFi
char pass[] = "";             // Mật khẩu WiFi (Wokwi không cần)

#define VIRTUAL_IP     V1
#define VIRTUAL_LAT    V2
#define VIRTUAL_LON    V3
#define VIRTUAL_TEMP   V4
#define VIRTUAL_MAP    V5

const char* ipApiUrl = "http://ip-api.com/json";
const char* weatherApiKey = "caf8911642ee01fd32784c242a5fd9e1";

// 📌 Khai báo trước các hàm để tránh lỗi
void fetchLocationAndWeather();
void fetchWeather(double lat, double lon);

BlynkTimer timer;

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, pass);

    Serial.print("Đang kết nối WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi đã kết nối!");

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Gọi hàm cập nhật vị trí & thời tiết mỗi 5 phút
    timer.setInterval(300000L, fetchLocationAndWeather);
}

void loop() {
    Blynk.run();
    timer.run();
}

// 📌 Lấy vị trí từ API
void fetchLocationAndWeather() {
    Serial.println("📍 Đang lấy vị trí...");

    HTTPClient http;
    http.begin(ipApiUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
        String payload = http.getString();
        Serial.println("🌍 Dữ liệu nhận được: " + payload);

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        String ip = doc["query"];
        double lat = doc["lat"];
        double lon = doc["lon"];
        String latStr = String(lat, 6);
        String lonStr = String(lon, 6);

        Serial.printf("IP: %s, Lat: %s, Lon: %s\n", ip.c_str(), latStr.c_str(), lonStr.c_str());

        Blynk.virtualWrite(VIRTUAL_IP, ip);
        Blynk.virtualWrite(VIRTUAL_LAT, latStr);
        Blynk.virtualWrite(VIRTUAL_LON, lonStr);

        String googleMapsLink = "https://www.google.com/maps/place/" + latStr + "," + lonStr;
        Blynk.virtualWrite(VIRTUAL_MAP, googleMapsLink);

        fetchWeather(lat, lon);
    } else {
        Serial.println("⚠️ Lỗi khi lấy dữ liệu từ API vị trí");
    }

    http.end();
}

// 📌 Lấy thời tiết từ OpenWeatherMap API
void fetchWeather(double lat, double lon) {
    Serial.println("🌦 Đang lấy dữ liệu thời tiết...");

    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(lat, 6) + "&lon=" + String(lon, 6) + "&appid=" + weatherApiKey + "&units=metric";
    http.begin(url);
    
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
        String payload = http.getString();
        Serial.println("⛅ Dữ liệu thời tiết: " + payload);

        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);

        float temp = doc["main"]["temp"];
        Serial.printf("🌡 Nhiệt độ: %.2f°C\n", temp);

        Blynk.virtualWrite(VIRTUAL_TEMP, String(temp) + " °C");
    } else {
        Serial.println("⚠️ Lỗi khi lấy dữ liệu thời tiết");
    }

    http.end();
}
