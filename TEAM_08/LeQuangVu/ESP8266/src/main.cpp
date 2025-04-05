#include <Arduino.h>
#include "utils.h"

#include <Wire.h>
#include <U8g2lib.h>

#define gPIN 15
#define yPIN 2
#define rPIN 5

#define OLED_SDA 13
#define OLED_SCL 12

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

bool WelcomeDisplayTimeout(uint msSleep = 5000){
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;    
  return bDone;
}

void setup() {
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);
  
  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);  // SDA, SCL

  oled.begin();
  oled.clearBuffer();
  
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Nhóm 8");  
  oled.drawUTF8(0, 28, "Nhuận óc chó ");
  oled.drawUTF8(0, 42, "Vũ đẹo trai");  

  oled.sendBuffer();

  randomSeed(analogRead(A0));  // Khởi tạo random
}

void ThreeLedBlink(){
  static ulong lastTimer = 0;
  static int currentLed = 0;  
  static const int ledPin[3] = {gPIN, yPIN, rPIN};

  if (!IsReady(lastTimer, 1000)) return;
  int prevLed = (currentLed + 2) % 3;
  digitalWrite(ledPin[prevLed], LOW);  
  digitalWrite(ledPin[currentLed], HIGH);  
  currentLed = (currentLed + 1) % 3;
}

void updateDHT(){
  static ulong lastTimer = 0;  
  if (!IsReady(lastTimer, 2000)) return;

  // Random nhiệt độ từ 20 đến 35 độ C
  float t = random(200, 350) / 10.0;

  // Random độ ẩm từ 40 đến 80%
  float h = random(400, 800) / 10.0;

  Serial.print("Temperature (random): ");
  Serial.print(t);
  Serial.println(" *C");

  Serial.print("Humidity (random): ");
  Serial.print(h);
  Serial.println(" %");

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese2);

  String s = StringFormat("Nhiet do: %.2f °C", t);
  oled.drawUTF8(0, 14, s.c_str());  

  s = StringFormat("Do am: %.2f %%", h);
  oled.drawUTF8(0, 42, s.c_str());      

  oled.sendBuffer();
}

void loop() {
  if (!WelcomeDisplayTimeout()) return;
  ThreeLedBlink();
  updateDHT();
}
