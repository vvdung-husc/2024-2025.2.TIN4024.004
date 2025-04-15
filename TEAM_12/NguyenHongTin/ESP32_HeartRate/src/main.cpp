#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <LiquidCrystal_I2C.h>

#define REPORTING_PERIOD_MS 1000

PulseOximeter pox;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ LCD 0x27, 16 cột, 2 dòng

uint32_t tsLastReport = 0;

void onBeatDetected()
{
    Serial.println("Phát hiện nhịp tim!");
}

void setup()
{
    Serial.begin(115200);
    lcd.init();  // Khởi tạo LCD
    lcd.backlight();

    if (!pox.begin()) {
        Serial.println("Lỗi cảm biến nhịp tim!");
        lcd.print("Loi cam bien nhip tim!");
    } else {
        Serial.println("Cảm biến OK!");
        lcd.print("Cam bien OK!");
    }

    pox.setOnBeatDetectedCallback(onBeatDetected);
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}

void loop()
{
    pox.update();

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Nhip tim: ");
        lcd.print(pox.getHeartRate());
        lcd.print(" bpm");

        lcd.setCursor(0, 1);
        lcd.print("Oxy: ");
        lcd.print(pox.getSpO2());
        lcd.print("%");

        tsLastReport = millis();
    }
}
