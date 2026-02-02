#pragma once
#include <Arduino.h>

class LED
{
public:
    LED();
    void setup(int pin);
    void set(bool bON);
private:
    int _pin;
};

class LDR
{
public:
    LDR();
    void setup(int pin, bool vcc5Volt = true); // VCC = 3.3V or 5V
    int getValue(); // Analog value 0-4095
    float readLux(int* analogValue = nullptr); // Return light intensity in lux

    static int DAY_THRESHOLD;
private:
    int _pin;
    int _value;
    bool _vcc5Volt;
};

class Trafic_Blink
{
public:
    Trafic_Blink();
    void setupPin(int pinRed, int pinYellow, int pinGreen);
    void setupWaitTime(uint32_t redWait = 5, uint32_t yellowWait = 3, uint32_t greenWait = 10); // seconds
    void run(LDR& ldrSensor); 
    const char *ledString(int pin);

private:
    bool _ledStatus;
    unsigned long _previousMillis;
    int _LEDs[3];
    int _idxLED;
    uint32_t _waitTime[3];
};



bool IsReady(unsigned long &ulTimer, uint32_t millisecond = 1000);

// Định dạng chuỗi %s,%d,...
String StringFormat(const char *fmt, ...);