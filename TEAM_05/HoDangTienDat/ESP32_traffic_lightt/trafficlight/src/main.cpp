#include <Arduino.h>
#include <TM1637Display.h>

#define RED_PIN    5    // Đỏ 
#define GREEN_PIN  16   // Xanh
#define YELLOW_PIN 17   // Vàng 

// Chân module TM1637
#define CLK_PIN 15
#define DIO_PIN 2

TM1637Display display(CLK_PIN, DIO_PIN);

const unsigned long durations[3] = {
  4000,  // RED: 4 giây
  5000,  // GREEN: 5 giây
  2000   // YELLOW: 2 giây
};

// Các trạng thái: 0 = RED, 1 = GREEN, 2 = YELLOW
int state = 0;
unsigned long previousMillis = 0;
int transitionCount = 0;
unsigned long cumulativeTime = 0;

// Biến lưu trạng thái số giây hiển thị lần trước (để giảm cập nhật màn hình)
int lastDisplayedSec = -1;

String stateName(int s) {
  switch (s) {
    case 0: return "RED";
    case 1: return "GREEN";
    case 2: return "YELLOW";
    default: return "";
  }
}

void setState(int newState, unsigned long lastDuration) {
  cumulativeTime += lastDuration;
  transitionCount++;

  Serial.print(millis() / 1000); // In thời gian thực
  Serial.print(" s - ");
  Serial.print(transitionCount);
  Serial.print(". ");
  Serial.print(stateName(state));
  Serial.print(" ==> ");
  Serial.print(stateName(newState));
  Serial.print(" (");
  Serial.print(cumulativeTime / 1000);
  Serial.println(" s)");

  state = newState;
  previousMillis = millis();

  switch (state) {
    case 0: // RED
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(YELLOW_PIN, LOW);
      break;
    case 1: // GREEN
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(YELLOW_PIN, LOW);
      break;
    case 2: // YELLOW
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(YELLOW_PIN, HIGH);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  
  display.setBrightness(0x0f);
  display.showNumberDec(0, true);
  
  previousMillis = millis();

  Serial.println("Traffic light simulation started.");
  Serial.print("0. NONE ==> ");
  Serial.print(stateName(state));
  Serial.println(" (0 s)");
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long interval = durations[state];
  unsigned long elapsed = currentMillis - previousMillis;

  int remainingSec = (interval - elapsed + 999) / 1000;
  if (remainingSec < 0) remainingSec = 0;

  if (remainingSec != lastDisplayedSec) {  // Chỉ cập nhật màn hình khi giá trị thay đổi
    display.showNumberDec(remainingSec, true);
    lastDisplayedSec = remainingSec;
  }

  if (elapsed >= interval) {
    int nextState = (state + 1) % 3;
    setState(nextState, interval);
  }
}
