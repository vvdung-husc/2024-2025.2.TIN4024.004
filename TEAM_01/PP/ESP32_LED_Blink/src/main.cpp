#include<Arduino.h>
#include <TM1637Display.h>

#define RED_LED 18
#define GREEN_LED 17
#define YELLOW_LED 2
#define CLK 15
#define DIO 0

unsigned long previousMillis = 0;
const long interval = 3000;
int state = 0;
int countdown = 3;

TM1637Display display(CLK, DIO);

void updateLights(int state);
void displayCountdown(int timeLeft);

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    Serial.begin(115200);
    display.setBrightness(0x0f);
    displayCountdown(countdown);
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
        previousMillis = currentMillis;
        countdown--;
        if (countdown == 0) {
            state = (state + 1) % 3;
            updateLights(state);
            countdown = 3;
        }
        displayCountdown(countdown);
    }
}

void updateLights(int state) {
    switch (state) {
        case 0:
            digitalWrite(RED_LED, HIGH);
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(YELLOW_LED, LOW);
            break;
        case 1:
            digitalWrite(RED_LED, LOW);
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(YELLOW_LED, LOW);
            break;
        case 2:
            digitalWrite(RED_LED, LOW);
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(YELLOW_LED, HIGH);
            break;
    }
}

void displayCountdown(int timeLeft) {
    Serial.print("Time left: ");
    Serial.print(timeLeft);
    Serial.println(" seconds");
    display.showNumberDec(timeLeft);
}