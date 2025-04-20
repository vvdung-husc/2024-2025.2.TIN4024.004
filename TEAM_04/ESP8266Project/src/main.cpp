

// ==== Blynk Credentials ====
//  Nguyễn Đình Ngọc Bảo 
//  #define BLYNK_TEMPLATE_ID "TMPL6fjFNdL5O"
//  #define BLYNK_TEMPLATE_NAME "ESP8266Project"
//  #define BLYNK_AUTH_TOKEN "EqapOpB088ZJ2obRlqC0FFqewej-al3E"

// //Nguyễn Hữu Phước
// #define BLYNK_TEMPLATE_ID "TMPL6SgBJeLiw"
// #define BLYNK_TEMPLATE_NAME "ESP8266 Project"
// #define BLYNK_AUTH_TOKEN "4XAvaz1jYQUtEY4FCXNTpjJ0fnIBUXcz"

// Phan Bá Dân
#define BLYNK_TEMPLATE_ID "TMPL6-GCFqGVh"
#define BLYNK_TEMPLATE_NAME "ESP8266 Project"
#define BLYNK_AUTH_TOKEN "ble4eJLaOdlVPWzk_1iXPuGvkU_S4Uby"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
#include <Arduino.h>
#include "utils.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#define gPIN 15
#define yPIN 2
#define rPIN 5

#define dhtPIN 16
#define dhtTYPE DHT11

#define OLED_SDA 13
#define OLED_SCL 12

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

DHT dht(dhtPIN, dhtTYPE);

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

bool nutNhan = false;

bool WelcomeDisplayTimeout(uint tgCho = 5000) {
    static ulong thoiGianTruoc = 0;
    static bool hoanTat = false;
    if (hoanTat) return true;
    if (!IsReady(thoiGianTruoc, tgCho)) return false;
    hoanTat = true;
    return hoanTat;
}

void setup() {
    Serial.begin(115200);
    pinMode(gPIN, OUTPUT);
    pinMode(yPIN, OUTPUT);
    pinMode(rPIN, OUTPUT);

    digitalWrite(gPIN, LOW);
    digitalWrite(yPIN, LOW);
    digitalWrite(rPIN, LOW);

    dht.begin();
    Wire.begin(OLED_SDA, OLED_SCL);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    Serial.println(WiFi.status() == WL_CONNECTED ? "✅ WiFi kết nối thành công!" : "❌ Kết nối WiFi thất bại!");

    Blynk.virtualWrite(V3, nutNhan);
    oled.begin();
    oled.clearBuffer();

    oled.setFont(u8g2_font_unifont_t_vietnamese1);
    oled.drawUTF8(0, 14, "Trường Đại học Khoa học");
    oled.drawUTF8(0, 28, "Khoa Công Nghệ Thông Tin");
    oled.drawUTF8(0, 42, "Lập trình IoT Nhóm 4");

    oled.sendBuffer();
    randomSeed(analogRead(0));
}
void ThreeLedBlink() {
    static ulong thoiGianTruoc = 0;
    static int denHienTai = 0;
    static const int danhSachDen[3] = {gPIN, yPIN, rPIN};

    if (!IsReady(thoiGianTruoc, 1000)) return;
    int denTruoc = (denHienTai + 2) % 3;
    digitalWrite(danhSachDen[denTruoc], LOW);
    digitalWrite(danhSachDen[denHienTai], HIGH);
    denHienTai = (denHienTai + 1) % 3;
}

float doAm = 0.0;
float nhietDo = 0.0;

void UpdateDHT() {
    static ulong thoiGianTruoc = 0;
    if (!IsReady(thoiGianTruoc, 2000)) return;

    float doAmMoi = random(0, 10001) / 100.0;
    float nhietDoMoi = random(0, 8001) / 100.0;

    if (isnan(doAmMoi) || isnan(nhietDoMoi)) {
        Serial.println("Lỗi đọc cảm biến DHT!");
        return;
    }

    bool canVe = false;

    if (nhietDo != nhietDoMoi) {
        canVe = true;
        nhietDo = nhietDoMoi;
        Serial.printf("Nhiệt độ: %.2f °C\n", nhietDo);
    }

    if (doAm != doAmMoi) {
        canVe = true;
        doAm = doAmMoi;
        Serial.printf("Độ ẩm: %.2f%%\n", doAm);
    }

    if (canVe) {
        oled.clearBuffer();
        oled.setFont(u8g2_font_unifont_t_vietnamese2);

        String chuoiNhietDo = StringFormat("Nhiệt độ: %.2f °C", nhietDo);
        oled.drawUTF8(0, 14, chuoiNhietDo.c_str());

        String chuoiDoAm = StringFormat("Độ ẩm: %.2f%%", doAm);
        oled.drawUTF8(0, 42, chuoiDoAm.c_str());

        oled.sendBuffer();
    }

    Blynk.virtualWrite(V1, nhietDo);
    Blynk.virtualWrite(V2, doAm);
}
void DrawCounter() {
    static uint boDem = 0;
    static ulong thoiGianTruoc = 0;
    if (!IsReady(thoiGianTruoc, 2000)) return;

    oled.clearBuffer();
    oled.setFont(u8g2_font_logisoso32_tf);
    oled.setCursor(30, 40);
    oled.print(boDem);
    oled.sendBuffer();

    boDem++;
}

void YellowLight() {
    static bool trangThaiDenVang = false;
    static ulong thoiGianTruoc = 0;
    if (IsReady(thoiGianTruoc, 500)) {
        trangThaiDenVang = !trangThaiDenVang;
        digitalWrite(yPIN, trangThaiDenVang);
    }

    digitalWrite(gPIN, LOW);
    digitalWrite(rPIN, LOW);
}

void guiThoiGianLenBlynk() {
  static ulong thoiGianTruoc = 0;
    if (IsReady(thoiGianTruoc, 1000)) {
        ulong giaTri = millis() / 1000; // Đọc thời gian hoạt động từ millis() và chia cho 1000 để tính thời gian giây
        Blynk.virtualWrite(V0, giaTri);  // Gửi thời gian lên Blynk
    }
}

BLYNK_WRITE(V3) {
  nutNhan = param.asInt();
}

void loop() {
  if (!WelcomeDisplayTimeout()) return;
  ThreeLedBlink();
  UpdateDHT();
  guiThoiGianLenBlynk();
  if (nutNhan) {
      YellowLight();
  }
  
}