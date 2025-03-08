#include <Arduino.h>
#include <TM1637Display.h>
#include <EEPROM.h>

// Chân LED giao thông
#define R_PIN 27
#define Y_PIN 26
#define G_PIN 25
#define IND_PIN 21  // LED chỉ thị

// Chân TM1637
#define CLK_PIN 18
#define DIO_PIN 19

// Chân nút nhấn
#define BTN_PIN 23

// Cảm biến ánh sáng (LDR)
#define LDR_PIN 13
#define LDR_THRESH 1000  // Ngưỡng ánh sáng

// Thời gian đèn (ms)
const uint rTime = 5000;  
const uint yTime = 3000;  
const uint gTime = 7000;  

// Biến trạng thái
bool dispOn = true;  // Trạng thái hiển thị TM1637
ulong lightStart = 0;
int curLight = R_PIN;
int secs = rTime / 1000;

// Đối tượng TM1637
TM1637Display disp(CLK_PIN, DIO_PIN);

// Timer hardware
hw_timer_t *timer = NULL;

// Hàm kiểm tra thời gian không chặn
bool isDone(ulong &timer, uint32_t ms) {
  if (millis() - timer < ms) return false;
  timer = millis();
  return true;
}

// Đọc cảm biến ánh sáng với bộ lọc trung bình
int readLDR() {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(LDR_PIN);
    delay(10);
  }
  return sum / 10;
}

// Cập nhật đếm ngược bằng Timer
void IRAM_ATTR onTimer() {
  if (secs > 0) secs--;
}

// Điều khiển đèn giao thông
void runTraffic() {
  int ldr = readLDR();

  if (ldr < LDR_THRESH) { // Chế độ ban đêm
    digitalWrite(R_PIN, LOW);
    digitalWrite(G_PIN, LOW);
    digitalWrite(Y_PIN, HIGH);
    disp.clear();
    return;
  }

  switch (curLight) {
    case R_PIN:
      if (isDone(lightStart, rTime)) {
        digitalWrite(R_PIN, LOW);
        digitalWrite(G_PIN, HIGH);
        curLight = G_PIN;
        secs = gTime / 1000;
      }
      break;

    case G_PIN:
      if (isDone(lightStart, gTime)) {
        digitalWrite(G_PIN, LOW);
        digitalWrite(Y_PIN, HIGH);
        curLight = Y_PIN;
        secs = yTime / 1000;
      }
      break;

    case Y_PIN:
      if (isDone(lightStart, yTime)) {
        digitalWrite(Y_PIN, LOW);
        digitalWrite(R_PIN, HIGH);
        curLight = R_PIN;
        secs = rTime / 1000;
      }
      break;
  }
}

// Cập nhật hiển thị TM1637
void updateDisplay() {
  if (dispOn) {
    char buf[3];
    sprintf(buf, "%02d", secs);
    disp.showNumberDecEx(atoi(buf), 0x00, true, 2, 2);
  } else {
    disp.clear();
  }
}

// Xử lý nút nhấn
void handleButton() {
  if (digitalRead(BTN_PIN) == LOW) {
    delay(50);
    if (digitalRead(BTN_PIN) == LOW) {
      dispOn = !dispOn;
      digitalWrite(IND_PIN, dispOn ? HIGH : LOW);
      
      EEPROM.write(0, dispOn);
      EEPROM.commit();

      Serial.println(dispOn ? "TM1637 ON" : "TM1637 OFF");
      while (digitalRead(BTN_PIN) == LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(1);

  pinMode(R_PIN, OUTPUT);
  pinMode(Y_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(IND_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  digitalWrite(Y_PIN, LOW);
  digitalWrite(G_PIN, LOW);
  digitalWrite(R_PIN, HIGH);
  digitalWrite(IND_PIN, HIGH);

  disp.setBrightness(7);
  disp.showNumberDecEx(secs, 0x00, true, 2, 2);

  dispOn = EEPROM.read(0);
  digitalWrite(IND_PIN, dispOn ? HIGH : LOW);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);

  Serial.println("== START ==> Traffic Light System");
}

void loop() {
  runTraffic();
  updateDisplay();
  handleButton();
}
