#include <Arduino.h>
#include <TM1637Display.h>

#define RED_PIN    5    //đỏ 
#define GREEN_PIN  16   // xanh
#define YELLOW_PIN 17   // vàng 

//chân module TM1637
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

// Thời điểm bắt đầu
unsigned long previousMillis = 0;

//đếm số lần chuyển trạng thái
int transitionCount = 0;

// Tổng thời gian tích lũy của các trạng thái đã hoàn thành (ms)
unsigned long cumulativeTime = 0;

// Hàm trả về tên của trạng thái
String stateName(int s) {
  switch (s) {
    case 0: return "RED";
    case 1: return "GREEN";
    case 2: return "YELLOW";
    default: return "";
  }
}

// Hàm cập nhật trạng thái mới và in thông báo chuyển đổi ngay lập tức
void setState(int newState, unsigned long lastDuration) {
  // Cập nhật tổng thời gian (tính cho trạng thái vừa hoàn thành)
  cumulativeTime += lastDuration;
  transitionCount++;

  // In thông báo chuyển trạng thái ngay lúc chuyển (không chờ đợi)
  // In theo định dạng: "<số thứ tự>. <TRẠNG THÁI HIỆN TẠI> ==> <TRẠNG THÁI MỚI> (<tổng thời gian> s)"
  Serial.print(transitionCount);
  Serial.print(". ");
  Serial.print(stateName(state));
  Serial.print(" ==> ");
  Serial.print(stateName(newState));
  Serial.print(" (");
  Serial.print(cumulativeTime / 1000);
  Serial.println(" s)");

  // Cập nhật trạng thái và thời điểm bắt đầu của trạng thái mới
  state = newState;
  previousMillis = millis();

  // Cập nhật LED theo trạng thái mới
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
  
  // Cấu hình chân LED là OUTPUT
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  
  // Khởi tạo ban đầu: bật đèn RED, tắt các đèn khác
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  
  // Khởi tạo module 7-segment
  display.setBrightness(0x0f);
  display.showNumberDec(0, true);
  
  previousMillis = millis();
  
  Serial.println("Traffic light simulation started.");
  // In thông báo ban đầu (trạng thái đầu tiên)
  Serial.print("0. NONE ==> ");
  Serial.print(stateName(state));
  Serial.print(" (0 s)");
  Serial.println();
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long interval = durations[state]; // Thời gian của trạng thái hiện tại

  // Tính thời gian đã trôi qua kể từ khi trạng thái hiện tại bắt đầu
  unsigned long elapsed = currentMillis - previousMillis;
  
  //hiển thị số giây còn lại của trạng thái hiện tại
  int remainingSec = (interval - elapsed + 999) / 1000;
  if (remainingSec < 0) remainingSec = 0;
  display.showNumberDec(remainingSec, true);
  
  // Nếu thời gian hiện tại của trạng thái đã hết
  if (elapsed >= interval) {
    int nextState = (state + 1) % 3;
    setState(nextState, interval);
  }
}
