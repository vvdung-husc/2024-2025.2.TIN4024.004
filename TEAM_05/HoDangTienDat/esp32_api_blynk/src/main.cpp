#define BLYNK_TEMPLATE_ID "TMPL64qPlY1Sk"
#define BLYNK_TEMPLATE_NAME "ESP32API"
#define BLYNK_AUTH_TOKEN "bVlGl1ZHG1GfL8M4sDh4yvcy5cJbQfu6"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>


//  API Key từ OpenWeatherMap
#define OPENWEATHERMAP_KEY "66d330a373653bf8683a5e68302a9ba0"

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

struct IP4_Info {
  String ip4;
  String latitude;
  String longtitude;
};

IP4_Info ip4Info;
String urlWeather;
ulong currentMiliseconds = 0;

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

String StringFormat(const char* fmt, ...) {
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return s;
}

void parseGeoInfo(String payload, IP4_Info& ipInfo) {
  String values[7];
  int index = 0;
  while (payload.length() > 0 && index < 7) {
    int delimiterIndex = payload.indexOf('|');
    if (delimiterIndex == -1) {
      values[index++] = payload;
      break;
    }
    values[index++] = payload.substring(0, delimiterIndex);
    payload = payload.substring(delimiterIndex + 1);
  }
  ipInfo.ip4 = values[0];
  ipInfo.latitude = values[6];
  ipInfo.longtitude = values[5];

  Serial.printf("IP: %s\nLat: %s\nLon: %s\n", ip4Info.ip4.c_str(), ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
}

void getAPI() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin("http://ip4.iothings.vn/?geo=1");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    parseGeoInfo(response, ip4Info);

    String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
    urlWeather = StringFormat("https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
                              ip4Info.latitude.c_str(), ip4Info.longtitude.c_str(), OPENWEATHERMAP_KEY);

    Serial.println(link);
    Serial.println(urlWeather);
  } else {
    Serial.println("Error calling IP API");
  }

  http.end();
}

void updateTemp() {
  static ulong lastTime = 0;
  static float lastTemp = 0.0;

  if (!IsReady(lastTime, 10000)) return;
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(urlWeather);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    DynamicJsonDocument doc(2048);
    auto error = deserializeJson(doc, response);
    if (!error) {
      float temp = doc["main"]["temp"];
      if (temp != lastTemp) {
        lastTemp = temp;
        Serial.printf("Temp: %.1f\n", temp);
        Blynk.virtualWrite(V3, temp);
      }
    }
  } else {
    Serial.println("Error calling weather API");
  }

  http.end();
}

void onceCalled() {
  static bool done = false;
  if (done) return;
  done = true;

  String link = StringFormat("https://www.google.com/maps/place/%s,%s", ip4Info.latitude.c_str(), ip4Info.longtitude.c_str());
  Blynk.virtualWrite(V1, ip4Info.ip4);
  Blynk.virtualWrite(V2, link);
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return;
  Blynk.virtualWrite(V0, millis() / 1000);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

void loop() {
  Blynk.run();
  currentMiliseconds = millis();
  onceCalled();
  updateTemp();
  uptimeBlynk();
}
