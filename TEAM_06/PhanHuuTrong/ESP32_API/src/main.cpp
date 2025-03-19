#define BLYNK_TEMPLATE_ID "TMPL6j37JFNQC"
#define BLYNK_TEMPLATE_NAME "ESP32 Blynk LED"
#define BLYNK_AUTH_TOKEN "72PWXm4riLyYaEzU-8e5-wh5WJh7OxiE"

#include <WiFi.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// API lấy dữ liệu thời tiết từ OpenWeatherMap (Hà Nội)
const char* weatherApiUrl = "http://api.openweathermap.org/data/2.5/weather?q=HaNoi&appid=d28bc8d69fd4fb15eab6f9ee1cacee0e&units=metric";

void setup() {
    Serial.begin(115200);
    
    // Kết nối WiFi
    WiFi.begin(ssid, password);
    Serial.print("Đang kết nối WiFi...");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi đã kết nối!");
    Serial.print("Địa chỉ IP: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;  // Tạo đối tượng HTTPClient
        http.begin(weatherApiUrl); // Bắt đầu kết nối API
        int httpResponseCode = http.GET(); // Gửi yêu cầu GET
        
        if (httpResponseCode == 200) {
            String payload = http.getString(); // Lấy dữ liệu JSON
            Serial.println("Dữ liệu nhận được từ API:");
            Serial.println(payload); // In toàn bộ JSON ra terminal

            // Phân tích JSON
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                float temperature = doc["main"]["temp"]; // Lấy nhiệt độ
                const char* weather = doc["weather"][0]["description"]; // Mô tả thời tiết
                float humidity = doc["main"]["humidity"]; // Độ ẩm
                float windSpeed = doc["wind"]["speed"]; // Tốc độ gió
                float lon = doc["coord"]["lon"]; // Kinh độ
                float lat = doc["coord"]["lat"]; // Vĩ độ

                // Hiển thị dữ liệu trên Serial Monitor
                Serial.println("\n========== DỮ LIỆU THỜI TIẾT ==========");
                Serial.print("  Nhiệt độ: "); Serial.print(temperature); Serial.println("°C");
                Serial.print("  Mô tả: "); Serial.println(weather);
                Serial.print("  Độ ẩm: "); Serial.print(humidity); Serial.println("%");
                Serial.print("  Tốc độ gió: "); Serial.print(windSpeed); Serial.println(" m/s");
                Serial.print("  Vị trí: "); Serial.print(lat); Serial.print(", "); Serial.println(lon);
                Serial.println("=======================================\n");
            } else {
                Serial.println("❌ Lỗi khi phân tích JSON!");
            }
        } else {
            Serial.print("❌ Lỗi HTTP: ");
            Serial.println(httpResponseCode);
        }
        
        http.end(); // Đóng kết nối HTTP
    } else {
        Serial.println("⚠️  WiFi chưa kết nối!");
    }
    
    delay(60000); // Chờ 60 giây trước khi gửi yêu cầu tiếp theo
}