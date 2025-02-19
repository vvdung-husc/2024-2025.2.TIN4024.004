/*
#include <Arduino.h>

void setup() {
Serial.begin(115200);
Serial.println("Cai j day");
}

void loop() {
  static int i=0;
  ulong t=millis();//Khai bao t kieu long hoi ham milis tra ve thoi gian ms tu khu ESP32 bat dau chay thu
  Serial.println(t);
  Serial.println("Loop time: ");
  Serial.println(++i);//Tu so 1 thay vi 0 nhu i++
  delay(1000);
}
*/#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa chân LED cho đèn đỏ, đèn vàng và đèn xanh
const int RED_LED_PIN = 23;
const int YELLOW_LED_PIN = 22;
const int GREEN_LED_PIN = 21;

// Định nghĩa chân cho màn hình 7 đoạn
const int CLK_PIN = 16; // Chân CLK của TM1637
const int DIO_PIN = 17; // Chân DIO của TM1637

// Thời gian mỗi đèn sáng (millisecond)
const unsigned long redInterval = 5000;   // 5 giây
const unsigned long yellowInterval = 3000; // 3 giây
const unsigned long greenInterval = 7000;  // 7 giây

// Biến để lưu trữ thời gian lần cuối cùng LED thay đổi trạng thái
unsigned long previousMillis = 0;
// Biến để theo dõi trạng thái của LED hiện tại
int currentLED = 2; // Bắt đầu với đèn xanh (2)

// Biến để lưu trữ thời gian đếm ngược
unsigned long countdownTime = greenInterval; // Bắt đầu với thời gian đếm ngược cho đèn xanh

// Đối tượng TM1637Display để điều khiển màn hình 7 đoạn
TM1637Display display(CLK_PIN, DIO_PIN);

void setup() {
  // Thiết lập chân LED là OUTPUT
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  
  // Khởi động serial monitor để in ra thông tin
  Serial.begin(9600);

  // Thiết lập độ sáng cho màn hình 7 đoạn
  display.setBrightness(0x0f);

  // Bật đèn xanh ban đầu
  digitalWrite(GREEN_LED_PIN, HIGH);
}

void loop() {
  // Lấy thời gian hiện tại
  unsigned long currentMillis = millis();

  // Tính toán thời gian đếm ngược và hiển thị trên màn hình 7 đoạn
  int timeRemaining = (countdownTime - (currentMillis - previousMillis)) / 1000;
  display.showNumberDecEx(timeRemaining, 0b01000000, true);

  // Kiểm tra nếu thời gian đã qua là nhiều hơn hoặc bằng khoảng thời gian cho đèn hiện tại
  if ((currentLED == 0 && currentMillis - previousMillis >= redInterval) ||
      (currentLED == 1 && currentMillis - previousMillis >= yellowInterval) ||
      (currentLED == 2 && currentMillis - previousMillis >= greenInterval)) {
    // Lưu lại thời gian hiện tại
    previousMillis = currentMillis;

    // Tắt tất cả các đèn LED
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);

    // In ra trạng thái hiện tại và màu đèn sẽ sáng
    if (currentLED == 0) {
      Serial.print("YELLOW => RED : ");
      Serial.print(currentMillis);
      Serial.println("ms");
      digitalWrite(RED_LED_PIN, HIGH);
      currentLED = 2;
      countdownTime = greenInterval;
    } else if (currentLED == 1) {
      Serial.print("GREEN => YELLOW : ");
      Serial.print(currentMillis);
      Serial.println("ms");
      digitalWrite(YELLOW_LED_PIN, HIGH);
      currentLED = 0;
      countdownTime = redInterval;
    } else if (currentLED == 2) {
      Serial.print("YELLOW => GREEN : ");
      Serial.print(currentMillis);
      Serial.println("ms");
      digitalWrite(GREEN_LED_PIN, HIGH);
      currentLED = 1;
      countdownTime = yellowInterval;
    }
  }
}
