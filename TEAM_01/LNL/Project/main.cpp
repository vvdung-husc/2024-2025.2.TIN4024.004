const int trigPin = 5;
const int echoPin = 18;
const int ledPin = 16;
const int buzzerPin = 4;
const int dangerLevel = 10; // Ngưỡng cảnh báo (cm)

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(ledPin, HIGH);  // LED bật
  digitalWrite(buzzerPin, LOW);  // Buzzer tắt
  
  Serial.begin(115200);
}

void loop() {
  long duration, distance;

  // Đo khoảng cách
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;  // Tính khoảng cách (cm)
  
  Serial.print("Khoang cach: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Kiểm tra giá trị khoảng cách hợp lệ
  if (distance < 2 || distance > 400) { // Kiểm tra giá trị hợp lệ cho HC-SR04
    Serial.println("Gia tri khoang cach khong hop le!");
    return; // Thoát khỏi vòng lặp nếu khoảng cách không hợp lệ
  }

  // Kiểm tra trạng thái cảnh báo và điều khiển LED
  if (distance <= dangerLevel) {
    digitalWrite(ledPin, LOW);  // LED tắt
    digitalWrite(buzzerPin, HIGH);  // Buzzer bật
    Serial.println("Nguy hiem! LED tat va bao dong!");
    sendToThingSpeak(distance, 1);  // Gửi dữ liệu với cảnh báo
  } else {
    digitalWrite(ledPin, HIGH);  // LED bật
    digitalWrite(buzzerPin, LOW);  // Buzzer tắt
    Serial.println("An toan! LED sang!");
    sendToThingSpeak(distance, 0);  // Gửi dữ liệu với trạng thái an toàn
  }

  delay(10000);  // Đo lại sau 10 giây
}

void sendToThingSpeak(long distance, int alertStatus) {
  Serial.print("Goi du lieu len ThingSpeak: ");
  Serial.print("Khoang cach: "); 
  Serial.print(distance);
  Serial.print(" cm, Trang thai bao dong: "); 
  Serial.println(alertStatus);
}
