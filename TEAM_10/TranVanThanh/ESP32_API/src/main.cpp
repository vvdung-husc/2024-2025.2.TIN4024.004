#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// BLYNK CREDENTIALS
#define BLYNK_TEMPLATE_ID "TMPL6WQLYM4vE"
#define BLYNK_TEMPLATE_NAME "ESP32 API"
#define BLYNK_AUTH_TOKEN "f_anI_owNl3_ctATip2QyQI38resT7a2"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WIFI CREDENTIALS
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// OpenWeatherMap API Key (Thay key của bạn vào đây)
#define OPENWEATHERMAP_KEY "db7c89ee3508861b25e0e153b208cbe4"

struct IP4_Info
{
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;
ulong currentMillis = 0;

// Kiểm tra thời gian giữa các lần gửi dữ liệu
bool IsReady(ulong &ulTimer, uint32_t ms)
{
  if (currentMillis - ulTimer < ms)
    return false;
  ulTimer = currentMillis;
  return true;
}

// Format chuỗi
String StringFormat(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  char buff[256];
  vsnprintf(buff, sizeof(buff), fmt, args);
  va_end(args);
  return String(buff);
}

// Phân tích chuỗi từ API IP
void parseGeoInfo(String payload, IP4_Info &ipInfo)
{
  String values[7];
  int index = 0;

  while (payload.length() > 0 && index < 7)
  {
    int delimiterIndex = payload.indexOf('|');
    if (delimiterIndex == -1)
    {
      values[index++] = payload;
      break;
    }
    values[index++] = payload.substring(0, delimiterIndex);
    payload = payload.substring(delimiterIndex + 1);
  }

  ipInfo.ip4 = values[0];
  ipInfo.latitude = values[6];
  ipInfo.longitude = values[5];

  Serial.printf("IPv4: %s\n", ipInfo.ip4.c_str());
  Serial.printf("Latitude: %s\n", ipInfo.latitude.c_str());
  Serial.printf("Longitude: %s\n", ipInfo.longitude.c_str());
}

// Lấy IP, vĩ độ, kinh độ từ API
void getAPI()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("getAPI() Error: WiFi not connected");
    return;
  }

  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200)
  {
    String response = http.getString();
    Serial.println(response);
    parseGeoInfo(response, ip4Info);
  }
  else
  {
    Serial.printf("getAPI() Error: HTTP Code %d\n", httpResponseCode);
  }

  http.end();
}

// Lấy nhiệt độ từ OpenWeatherMap
void updateTemp()
{
  static ulong lastTime = 0;
  static float lastTemp = 0.0;

  if (!IsReady(lastTime, 10000))
    return;
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("updateTemp() Error: WiFi not connected");
    return;
  }

  String urlWeather = StringFormat(
      "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
      ip4Info.latitude.c_str(), ip4Info.longitude.c_str(), OPENWEATHERMAP_KEY);

  HTTPClient http;
  http.begin(urlWeather);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200)
  {
    String response = http.getString();
    Serial.println(response);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);

    if (!error)
    {
      float temp = doc["main"]["temp"];
      if (temp != lastTemp)
      {
        lastTemp = temp;
        Serial.printf("Temperature: %.2f°C\n", temp);
        Blynk.virtualWrite(V4, temp); // Cập nhật nhiệt độ lên Blynk (V4)
      }
    }
    else
    {
      Serial.println("JSON Parsing Failed!");
    }
  }
  else
  {
    Serial.printf("updateTemp() Error: HTTP Code %d\n", httpResponseCode);
  }

  http.end();
}

// Gửi dữ liệu IP, tọa độ lên Blynk
void onceCalled()
{
  static bool done = false;
  if (done)
    return;
  done = true;

  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, ip4Info.latitude);
  Blynk.virtualWrite(V3, ip4Info.longitude);

  String link = StringFormat("https://www.google.com/maps/place/%s,%s",
                             ip4Info.latitude.c_str(), ip4Info.longitude.c_str());
  Blynk.virtualWrite(V5, link);
}

// Cập nhật thời gian chạy
void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return;

  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to WiFi: %s", WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

void loop()
{
  Blynk.run();

  currentMillis = millis();
  onceCalled();
  updateTemp();
  uptimeBlynk();
}