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
*/
#include <Arduino.h>

// Định nghĩa chân LED cho đèn đỏ, đèn vàng và đèn xanh
const int RED_LED_PIN = 23;
const int YELLOW_LED_PIN = 22;
const int GREEN_LED_PIN = 21;

// Thời gian mỗi đèn sáng (millisecond)
const unsigned long redInterval = 5000; // 5 giây
const unsigned long yellowInterval = 3000; // 3 giây
const unsigned long greenInterval = 7000; // 7 giây

// Biến để lưu trữ thời gian lần cuối cùng LED thay đổi trạng thái
unsigned long previousMillis = 0;
// Biến để theo dõi trạng thái của LED hiện tại
int currentLED = 0;

void setup() {
  // Thiết lập chân LED là OUTPUT
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  
  // Khởi động serial monitor để in ra thông tin
  Serial.begin(9600);
}

void loop() {
  // Lấy thời gian hiện tại
  unsigned long currentMillis = millis();

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

    // In ra trạng thái hiện tại, màu đèn sẽ sáng và thời gian hiện tại
    if (currentLED == 0) {
      Serial.print("GREEN => RED : ");
      Serial.print(currentMillis);
      Serial.println("ms");
      digitalWrite(RED_LED_PIN, HIGH);
      currentLED = 1;
    } else if (currentLED == 1) {
      Serial.print("RED => YELLOW : ");
      Serial.print(currentMillis);
      Serial.println("ms");
      digitalWrite(YELLOW_LED_PIN, HIGH);
      currentLED = 2;
    } else if (currentLED == 2) {
      Serial.print("YELLOW => GREEN : ");
      Serial.print(currentMillis);
      Serial.println("ms");
      digitalWrite(GREEN_LED_PIN, HIGH);
      currentLED = 0;
    }
  }
}
