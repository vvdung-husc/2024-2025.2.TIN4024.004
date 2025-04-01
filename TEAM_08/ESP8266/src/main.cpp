// ======= BLYNK IOT =======
/* Le Quang Vu */
#define BLYNK_TEMPLATE_ID "TMPL6dJK4ixVG"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project"
#define BLYNK_AUTH_TOKEN "uYjkAWgSNUR1idW_e9I5aL7vmndYmonD"


/* Nguyen  Duc Nhuan 
#define BLYNK_TEMPLATE_ID "TMPL6fwjhHJe3"
#define BLYNK_TEMPLATE_NAME "ESP8266Nhuan"
#define BLYNK_AUTH_TOKEN "Tg4h_AKlRE_Pqw0znNiDUROtJ25p0r0z"
*/

/* Lê Văn Bùi
#define BLYNK_TEMPLATE_ID "TMPL6PnTyjtcJ"
#define BLYNK_TEMPLATE_NAME "New Template"
#define BLYNK_AUTH_TOKEN "2rojMKKtn5U2w7IXUhiFLICHfyLDjMtj"
*/

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

// ======= OLED + LED =======
#include <Wire.h>
#include <U8g2lib.h>
#include <Arduino.h>
#include "utils.h"

// ======= WiFi =======
char ssid[] = "K50";         // Thay WiFi bạn
char pass[] = "02022003";

// ======= PIN =======
#define gPIN 15
#define yPIN 2
#define rPIN 5
#define OLED_SDA 13
#define OLED_SCL 12

// ======= OLED INIT =======
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ======= Biến chế độ =======
enum Mode { MODE_BLINK_ALL, MODE_YELLOW_ONLY };
Mode currentMode = MODE_BLINK_ALL;

ulong systemStartTime = 0;
float t = 0.0, h = 0.0;

// ======= BLYNK SWITCH (V3) =======
BLYNK_WRITE(V3) {
  int value = param.asInt();
  currentMode = (value == 1) ? MODE_YELLOW_ONLY : MODE_BLINK_ALL;

  // Reset LED trạng thái
  digitalWrite(gPIN, LOW);
  digitalWrite(rPIN, LOW);
  digitalWrite(yPIN, LOW);

  Serial.print("Chuyen che do bang Blynk V3: ");
  Serial.println(currentMode == MODE_YELLOW_ONLY ? "Yellow Blink" : "Three LED Blink");

  // Hiển thị chế độ trên OLED
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Che do Blynk: ");
  oled.drawUTF8(0, 30, currentMode == MODE_YELLOW_ONLY ? "Den VANG" : "3 Den");
  oled.sendBuffer();
}

void setup() {
  Serial.begin(115200);

  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);

  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Ket noi Blynk...");
  oled.sendBuffer();

  randomSeed(analogRead(A0));

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  oled.clearBuffer();
  oled.drawUTF8(0, 14, "Da ket noi Blynk");
  oled.sendBuffer();

  systemStartTime = millis();
}

void ThreeLedBlink() {
  static ulong lastTimer = 0;
  static int currentLed = 0;  
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (!IsReady(lastTimer, 1000)) return;
  int prevLed = (currentLed + 2) % 3;
  digitalWrite(ledPin[prevLed], LOW);  
  digitalWrite(ledPin[currentLed], HIGH);  
  currentLed = (currentLed + 1) % 3;
}

void YellowBlink() {
  static ulong lastTimer = 0;
  static bool isOn = false;

  if (!IsReady(lastTimer, 500)) return;

  isOn = !isOn;
  digitalWrite(gPIN, LOW);
  digitalWrite(rPIN, LOW);
  digitalWrite(yPIN, isOn ? HIGH : LOW);
}

void updateDHT() {
  static ulong lastTimer = 0;  
  if (!IsReady(lastTimer, 2000)) return;

  t = random(200, 350) / 10.0;
  h = random(400, 800) / 10.0;

  Serial.printf("Temp: %.2f *C | Humi: %.2f %%\n", t, h);

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);

  String s = StringFormat("Nhiet do: %.2f °C", t);
  oled.drawUTF8(0, 14, s.c_str());  

  s = StringFormat("Do am: %.2f %%", h);
  oled.drawUTF8(0, 42, s.c_str());      

  oled.sendBuffer();

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
}

void sendUptimeToBlynk() {
  static ulong lastTimer = 0;
  if (!IsReady(lastTimer, 1000)) return;

  ulong uptimeSec = (millis() - systemStartTime) / 1000;
  int timeStr =  uptimeSec;
  Blynk.virtualWrite(V0, timeStr);
}

void loop() {
  Blynk.run();
  sendUptimeToBlynk();
  updateDHT();

  if (currentMode == MODE_BLINK_ALL) {
    ThreeLedBlink();
  } else if (currentMode == MODE_YELLOW_ONLY) {
    YellowBlink();
  }
}
