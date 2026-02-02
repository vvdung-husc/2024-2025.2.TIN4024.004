#pragma once

#include "ultils.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

class MainApp {
public:
    MainApp();
    ~MainApp();
    void setup(int pinGreen, int pinYellow, int pinRed);
    void run(DHT& dht, Adafruit_SSD1306& oled);
private:
    LED ledGreen;
    LED ledYellow;
    LED ledRed;
    ulong ulTimer;

    LED* curLed;
};