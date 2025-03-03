#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân kết nối
#define BUTTON_PIN 23
#define LED_BLUE   21
#define LED_GREEN  25
#define LED_YELLOW 26
#define LED_RED    27
#define LDR_PIN    13
#define CLK        18
#define DIO        19

// Khởi tạo đối tượng hiển thị TM1637
TM1637Display display(CLK, DIO);

// Biến toàn cục
int buttonState = 0;
int lastButtonState = HIGH;
bool displayOn = true;
int countdown = 5;               // Thời gian đếm ngược ban đầu (cho đèn xanh)
unsigned long previousMillis = 0;
const long interval = 1000;      // Thời gian cập nhật đếm ngược (1 giây)
unsigned long blinkMillis = 0;
const long blinkInterval = 500;  // Tốc độ nhấp nháy của đèn vàng
const long debounceDelay = 50;   // Thời gian chống rung nút
unsigned long lastDebounceTime = 0;
bool yellowBlinkState = false;   // Trạng thái nhấp nháy của đèn vàng
bool lightStateChanged = true;   // Theo dõi thay đổi trạng thái đèn giao thông

// Các trạng thái của đèn giao thông
enum TrafficLightState { GREEN, YELLOW, RED };
TrafficLightState lightState = GREEN;

// Các chế độ hệ thống dựa trên giá trị ánh sáng
enum SystemMode { NIGHT_MODE, WARNING_MODE, NORMAL_MODE };
SystemMode mode = NORMAL_MODE;

void setup() {
  // Cấu hình chân vào/ra
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  // Cấu hình hiển thị
  display.setBrightness(0x0F);
  display.showNumberDec(countdown);

  // Khởi động Serial để debug
  Serial.begin(115200);
  Serial.println("Hệ thống đèn giao thông khởi động...");
}

void loop() {
  unsigned long currentMillis = millis();

  // Đọc giá trị cảm biến ánh sáng (LDR) và chuyển đổi sang lux
  int ldrRaw = analogRead(LDR_PIN);
  float ldrLux = map(ldrRaw, 0, 4095, 0, 1000); // Giả định 0-4095 tương ứng 0-1000 lux

  // Xác định chế độ hoạt động dựa trên giá trị lux
  if (ldrLux < 10) {
    mode = NIGHT_MODE;
  } else if (ldrLux < 50) {
    mode = WARNING_MODE;
  } else {
    mode = NORMAL_MODE;
  }

  // Kiểm tra nút nhấn với debounce để bật/tắt hiển thị
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH && (currentMillis - lastDebounceTime) > debounceDelay) {
    displayOn = !displayOn;
    if (!displayOn) {
      display.clear();
    }
    lastDebounceTime = currentMillis;
  }
  lastButtonState = buttonState;

  // Xử lý theo từng chế độ
  switch(mode) {
    case NIGHT_MODE:
      // Chế độ ban đêm: chỉ bật LED xanh dương, tắt các LED khác và tắt hiển thị
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_BLUE, HIGH);
      if (displayOn) display.clear();
      Serial.println("Chế độ ban đêm: Bật đèn xanh dương (BLUE)");
      break;

    case WARNING_MODE:
      // Chế độ cảnh báo: LED xanh dương sáng luôn bật, LED vàng nhấp nháy, tắt LED xanh và đỏ
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_BLUE, HIGH);
      if (currentMillis - blinkMillis >= blinkInterval) {
        blinkMillis = currentMillis;
        yellowBlinkState = !yellowBlinkState;
        digitalWrite(LED_YELLOW, yellowBlinkState);
      }
      if (displayOn) display.clear();
      Serial.println("Chế độ cảnh báo: Đèn vàng nhấp nháy, đèn xanh dương bật");
      break;

    case NORMAL_MODE:
      // Chế độ giao thông bình thường
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        countdown--;

        // Khi đếm xong, chuyển đổi trạng thái đèn giao thông
        if (countdown < 0) {
          switch (lightState) {
            case GREEN:
              lightState = YELLOW;
              countdown = 3;
              lightStateChanged = true;
              break;
            case YELLOW:
              lightState = RED;
              countdown = 5;
              lightStateChanged = true;
              break;
            case RED:
              lightState = GREEN;
              countdown = 5;
              lightStateChanged = true;
              break;
          }
        }
        if (displayOn) {
          display.showNumberDec(countdown);
        }
      }

      // Cập nhật trạng thái LED nếu có sự thay đổi
      if (lightStateChanged) {
        switch (lightState) {
          case GREEN:
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_RED, LOW);
            Serial.println("Tín hiệu: XANH (GREEN)");
            break;
          case YELLOW:
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_YELLOW, HIGH);
            digitalWrite(LED_RED, LOW);
            Serial.println("Tín hiệu: VÀNG (YELLOW)");
            break;
          case RED:
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_YELLOW, LOW);
            digitalWrite(LED_RED, HIGH);
            Serial.println("Tín hiệu: ĐỎ (RED)");
            break;
        }
        lightStateChanged = false;
      }
      break;
  }

  // In giá trị lux và trạng thái đèn giao thông
  Serial.print("LDR Lux: ");
  Serial.print(ldrLux);
  Serial.print(" | Tín hiệu: ");
  switch (lightState) {
    case GREEN:
      Serial.println("XANH (GREEN)");
      break;
    case YELLOW:
      Serial.println("VÀNG (YELLOW)");
      break;
    case RED:
      Serial.println("ĐỎ (RED)");
      break;
  }
}
