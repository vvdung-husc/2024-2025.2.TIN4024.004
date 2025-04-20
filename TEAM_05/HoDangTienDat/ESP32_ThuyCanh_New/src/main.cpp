#define BLYNK_TEMPLATE_ID "TMPL6uis_oez2"
#define BLYNK_TEMPLATE_NAME "ESP32ThuyCanh"
#define BLYNK_AUTH_TOKEN "1-5VyH7lLgamJ3DEU_RPwXfcs_i7AG0R"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// WiFi cho Wokwi
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Cảm biến DHT22
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Pin thiết bị
#define LED_GROW_LIGHT 17
#define LED_PUMP 4
#define LED_PH_ALERT 15
#define LED_LIGHT_ALERT 0
#define LED_TEMP_ALERT 2
#define BTN_GROW_LIGHT 19
#define BTN_PUMP 18
#define LDR_PIN 34
#define PH_PIN 35

// Trạng thái thiết bị
bool growLightOn = false;
bool pumpOn = false;

// Blynk virtual pins
#define VPIN_TEMP V0
#define VPIN_LIGHT V1
#define VPIN_PH V2
#define VPIN_ALERT_LIGHT V4
#define VPIN_ALERT_PH V5
#define VPIN_GROW V6
#define VPIN_PUMP V7
#define VPIN_ALERT_TEMP V8

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  dht.begin();

  pinMode(LED_GROW_LIGHT, OUTPUT);
  pinMode(LED_PUMP, OUTPUT);
  pinMode(LED_PH_ALERT, OUTPUT);
  pinMode(LED_LIGHT_ALERT, OUTPUT);
  pinMode(LED_TEMP_ALERT, OUTPUT);
  pinMode(BTN_GROW_LIGHT, INPUT);
  pinMode(BTN_PUMP, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Không tìm thấy OLED");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  Blynk.run();

  float temp = dht.readTemperature();
  int lightValue = analogRead(LDR_PIN);
  int phRaw = analogRead(PH_PIN);
  float ph = map(phRaw, 0, 4095, 0, 14);

  display.clearDisplay();
  display.setCursor(0, 0);

  if (isnan(temp)) {
    Serial.println("Không đọc được nhiệt độ!");
    display.println("Temp error!");
  } else {
    display.print("Nhiet do: ");
    display.print(temp);
    display.println(" C");
    Blynk.virtualWrite(VPIN_TEMP, temp);

    // Cảnh báo nhiệt độ
    bool alertTemp = temp > 35;
    digitalWrite(LED_TEMP_ALERT, alertTemp);
    Blynk.virtualWrite(VPIN_ALERT_TEMP, alertTemp);
  }

  display.print("Anh sang: ");
  display.println(lightValue);
  display.print("pH: ");
  display.println(ph, 1);
  display.display();

  Blynk.virtualWrite(VPIN_LIGHT, lightValue);
  Blynk.virtualWrite(VPIN_PH, ph);

  // Cảnh báo ánh sáng và pH
  bool alertLight = lightValue < 1000;
  bool alertPH = (ph < 5.5 || ph > 7.5);
  digitalWrite(LED_LIGHT_ALERT, alertLight);
  digitalWrite(LED_PH_ALERT, alertPH);
  Blynk.virtualWrite(VPIN_ALERT_LIGHT, alertLight);
  Blynk.virtualWrite(VPIN_ALERT_PH, alertPH);

  // Nút bấm vật lý
  if (digitalRead(BTN_GROW_LIGHT) == HIGH) {
    growLightOn = !growLightOn;
    digitalWrite(LED_GROW_LIGHT, growLightOn);
    Blynk.virtualWrite(VPIN_GROW, growLightOn);
    delay(300);
  }

  if (digitalRead(BTN_PUMP) == HIGH) {
    pumpOn = !pumpOn;
    digitalWrite(LED_PUMP, pumpOn);
    Blynk.virtualWrite(VPIN_PUMP, pumpOn);
    delay(300);
  }

  delay(500);
}

// Blynk điều khiển từ xa
BLYNK_WRITE(VPIN_GROW) {
  growLightOn = param.asInt();
  digitalWrite(LED_GROW_LIGHT, growLightOn);
}

BLYNK_WRITE(VPIN_PUMP) {
  pumpOn = param.asInt();
  digitalWrite(LED_PUMP, pumpOn);
}
