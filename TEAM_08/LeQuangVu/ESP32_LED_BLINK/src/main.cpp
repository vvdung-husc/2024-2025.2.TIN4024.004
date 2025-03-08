#include <Arduino.h>

// Định nghĩa các chân LED
#define LED_RED 23   
#define LED_YELLOW 22  
#define LED_GREEN 21   

// Biến lưu thời gian
ulong currentMiliseconds = 0; 
ulong ledStart = 0; 

// Trạng thái đèn giao thông
enum TrafficLightState { RED, GREEN, YELLOW };
TrafficLightState currentState = RED;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void ControlTrafficLight();

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  currentMiliseconds = millis();
  ControlTrafficLight();
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

void ControlTrafficLight() {
  static uint32_t duration = 5000; // Thời gian chuyển đèn mặc định

  if (!IsReady(ledStart, duration)) 
    return;

  // Chuyển đổi trạng thái đèn giao thông
  switch (currentState) {
    case RED:
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, LOW);
      Serial.println("🔴 ĐÈN ĐỎ - DỪNG LẠI");
      duration = 5000; // Đỏ sáng 5 giây
      currentState = GREEN;
      break;

    case GREEN:
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_GREEN, HIGH);
      Serial.println("🟢 ĐÈN XANH - ĐI");
      duration = 4000; // Xanh sáng 4 giây
      currentState = YELLOW;
      break;

    case YELLOW:
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_YELLOW, HIGH);
      digitalWrite(LED_GREEN, LOW);
      Serial.println("🟡 ĐÈN VÀNG - CHUẨN BỊ DỪNG");
      duration = 2000; // Vàng sáng 2 giây
      currentState = RED;
      break;
  }
}
