#include <Arduino.h>
#define LED_PIN 23  // Chọn chân GPIO 2 để điều khiển LED

ulong currentMiliseconds = 0; // thời gian hiện tại ms
ulong ledStart = 0;           // thời gian on/off LED

void Use_Blocking();
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void Use_Non_Blocking();

void setup() {
  Serial.begin(115200);     //Khởi tạo Serial
  pinMode(LED_PIN, OUTPUT); // Cấu hình chân LED_PIN là đầu ra
}

void loop() {
  currentMiliseconds = millis();  //nhận thời gian hiện tại tính bằng msms
  //Use_Blocking();         //blocking
  Use_Non_Blocking();       //Non-blocking

  Serial.print("Timer :");
  Serial.println(currentMiliseconds);

}

void Use_Blocking()
{
  digitalWrite(LED_PIN, HIGH); // Bật LED
  Serial.println("LED -> ON");
  delay(1000);               // Đợi 1 giây
  digitalWrite(LED_PIN, LOW); // Tắt LED
  Serial.println("LED -> OFF");
  delay(1000); // Đợi 1 giây
}

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

void Use_Non_Blocking()
{
  static bool isLED_ON = false;
  if (!IsReady(ledStart, 1000))
    return;

  if (!isLED_ON)
  {
    digitalWrite(LED_PIN, HIGH); // Bật LED
    Serial.println("NonBlocking LED -> ON");
  }
  else
  {
    digitalWrite(LED_PIN, LOW); // Tắt LED
    Serial.println("NonBlocking LED -> OFF");
  }
  isLED_ON = !isLED_ON;
}