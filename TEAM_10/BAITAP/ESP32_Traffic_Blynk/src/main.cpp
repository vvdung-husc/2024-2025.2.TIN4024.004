#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6lYwGWEqx"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "GHwY9Ws1btOsgiJyMzkzmCs2p2-jFjC4H"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";  
char pass[] = "";             

#define btnBLED  23 
#define pinBLED  21 

#define DHTPIN 16      
#define DHTTYPE DHT22  

#define CLK 18  
#define DIO 19  

#define LIGHT_SENSOR_PIN 34  // Cảm biến ánh sáng quang trở (LDR)
#define WARNING_LED_PIN 5    // Đèn LED cảnh báo khi tối
int darkThreshold = 1000;    // Ngưỡng ánh sáng

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

ulong currentMiliseconds = 0; 
bool blueButtonON = true;     
ulong lastMillis = 0;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void checkLightThreshold();

void setup() {
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(WARNING_LED_PIN, OUTPUT);

  dht.begin();
  display.setBrightness(0x0f);
  display.showNumberDec(0, false);

  Serial.print("Connecting to "); Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); 

  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); 
  Blynk.virtualWrite(V4, darkThreshold); 

  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run();  

  if (millis() - lastMillis >= 2000) { 
    lastMillis = millis();

    float temperature = dht.readTemperature(); 
    float humidity = dht.readHumidity();       

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Lỗi đọc cảm biến DHT22!");
      return;
    }

    Serial.print("Nhiệt độ: "); Serial.print(temperature); Serial.print("°C, ");
    Serial.print("Độ ẩm: "); Serial.print(humidity); Serial.println("%");

    display.showNumberDec((int)temperature, false); 

    Blynk.virtualWrite(V2, temperature);
    Blynk.virtualWrite(V3, humidity);
  }

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  checkLightThreshold();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton() {
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return; 
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON) {
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);
  } else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);
    display.clear();
  }    
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; 
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);
  if (blueButtonON) {
    display.showNumberDec(value);
  }
}

void checkLightThreshold() {
  int lightValue = analogRead(LIGHT_SENSOR_PIN);
  Serial.print("Light Intensity: "); Serial.println(lightValue);

  Blynk.virtualWrite(V4, lightValue);

  if (lightValue < darkThreshold) {
    digitalWrite(WARNING_LED_PIN, HIGH);
    Serial.println("Warning: Too dark!");
  } else {
    digitalWrite(WARNING_LED_PIN, LOW);
  }
}

BLYNK_WRITE(V1) {
  blueButtonON = param.asInt();
  if (blueButtonON) {
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  } else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    display.clear();
  }
}

BLYNK_WRITE(V4) {
  darkThreshold = param.asInt();
  Serial.print("Updated darkThreshold: "); Serial.println(darkThreshold);
}
