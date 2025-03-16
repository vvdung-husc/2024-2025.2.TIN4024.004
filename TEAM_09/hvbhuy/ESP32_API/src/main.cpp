#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Arduino_JSON.h>
#include <BlynkSimpleEsp32.h>

const char *ssid = "VNPTHUE";
const char *password = "033607427";
const char *blynkAuth = "YOUR_BLYNK_AUTH_TOKEN"; // Thay YOUR_BLYNK_AUTH_TOKEN bằng mã thật của bạn

String locationUrl = "http://ip4.iothings.vn?geo=1";
String weatherApiKey = "YOUR_OPENWEATHERMAP_API_KEY";

String httpGETRequest(const char *Url);

void setup()
{
  Serial.begin(9600);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Kết nối Blynk
  Blynk.begin(blynkAuth, ssid, password);
}

void loop()
{
  Blynk.run(); // Duy trì kết nối Blynk

  Serial.println("Fetching location data...");
  String locationJsonBuffer = httpGETRequest(locationUrl.c_str());
  Serial.println(locationJsonBuffer);

  JSONVar locationJson = JSON.parse(locationJsonBuffer);
  if (JSON.typeof_(locationJson) == "undefined")
  {
    Serial.println("Parsing failed");
    return;
  }

  String ip = locationJson["ip"];
  float latitude = atof(locationJson["lat"]);
  float longitude = atof(locationJson["lon"]);

  Serial.println();
  Serial.print("IPv4: ");
  Serial.println(ip);
  Serial.print("Latitude: ");
  Serial.println(latitude);
  Serial.print("Longitude: ");
  Serial.println(longitude);

  String googleMapsLink = "https://www.google.com/maps/search/?api=1&query=" + String(latitude) + "," + String(longitude);
  Serial.print("Google Maps Link: ");
  Serial.println(googleMapsLink);

  // Fetch weather data
  String weatherUrl = "https://api.openweathermap.org/data/2.5/weather?lat=" + String(latitude) + "&lon=" + String(longitude) + "&appid=" + weatherApiKey;
  Serial.println("Fetching weather data...");
  String weatherJsonBuffer = httpGETRequest(weatherUrl.c_str());
  Serial.println(weatherJsonBuffer);

  JSONVar weatherJson = JSON.parse(weatherJsonBuffer);
  if (JSON.typeof_(weatherJson) == "undefined")
  {
    Serial.println("Parsing weather data failed");
    return;
  }

  float temperature = (double)weatherJson["main"]["temp"] - 273.15;
  float humidity = (double)weatherJson["main"]["humidity"];

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V1, ip);
  Blynk.virtualWrite(V2, latitude);
  Blynk.virtualWrite(V3, longitude);
  Blynk.virtualWrite(V4, temperature);
  Blynk.virtualWrite(V5, humidity);
  Blynk.virtualWrite(V6, googleMapsLink);

  delay(10000);
}

String httpGETRequest(const char *Url)
{
  HTTPClient http;
  http.begin(Url);
  int responseCode = http.GET();
  String responseBody = "{}";
  if (responseCode > 0)
  {
    Serial.print("Response Code: ");
    Serial.println(responseCode);
    responseBody = http.getString();
  }
  else
  {
    Serial.print("Error Code: ");
    Serial.println(responseCode);
  }
  http.end();
  return responseBody;
}
