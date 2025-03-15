#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// API để lấy địa chỉ IPv4, vĩ độ, kinh độ
const char* ipApiUrl = "http://ip4.iothings.vn/?geo=1";
const char* apiUrl = "https://api.openweathermap.org/data/2.5/weather?lat=16.466669&lon=107.599998&appid=346de7aa97821496540613715713caf5&units=metric&lang=vi";

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");

    // Gửi yêu cầu HTTP GET
    HTTPClient http;
    http.begin(apiUrl);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
        
        // Parse JSON
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, response);
        
        if (!error) {
            const char* ip = doc["ip"];
            float latitude = doc["latitude"];
            float longitude = doc["longitude"];

            Serial.println("\n=== IP address information ===");
            Serial.print("IPv4: "); Serial.println(ip);
            Serial.print("Latitude (Vi_do): "); Serial.println(latitude, 6);
            Serial.print("Longitude (Kinh_do): "); Serial.println(longitude, 6);

            // Tạo link Google Maps
            Serial.print("\n Google Maps: ");
            Serial.print("http://www.google.com/maps/place/16.4591267,107.5901477");
            Serial.print(latitude, 6);
            Serial.print(",");
            Serial.println(longitude, 6);
        } else {
            Serial.println("❌ Error parsing JSON!");
        }
    } else {
        Serial.print("❌ HTTP error, error code: ");
        Serial.println(httpResponseCode);
    }
    
    http.end();
}

void loop() {
    // Không cần vòng lặp, chương trình chỉ chạy 1 lần
}