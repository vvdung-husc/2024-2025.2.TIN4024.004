#include <Arduino.h>


#define LED_RED 23  // Chọn chân GPIO 2 để điều khiển LED
#define LED_YELLOW 19
#define LED_GREEN 13

ulong currentMiliseconds = 3; // thời gian hiện tại ms
ulong ledStart = 0;           // thời gian on/off LED
ulong ledYellowStart = 0; 
ulong ledGreenStart = 0; 

//void Use_Blocking();
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void Use_Non_Blocking();

void setup(){
  Serial.begin(115200);     //Khởi tạo Serial
  pinMode(LED_RED, OUTPUT); // Cấu hình chân LED_PIN là đầu ra
  pinMode(LED_YELLOW, OUTPUT); 
  pinMode(LED_GREEN, OUTPUT); 
}

void loop() {
  currentMiliseconds = millis();  //nhận thời gian hiện tại tính bằng msms
  //Use_Blocking();         //blocking
  Use_Non_Blocking();       //Non-blocking

  // Serial.print("Timer :");
  // Serial.println(currentMiliseconds);

}

// void Use_Blocking()
// {
//   digitalWrite(LED_RED, HIGH); // Bật LED
//   Serial.println("LED -> ON");
//   delay(1000);               // Đợi 1 giây
//   digitalWrite(LED_RED, LOW); // Tắt LED
//   Serial.println("LED -> OFF");
//   delay(1000); // Đợi 1 giây

  
// }

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

void Use_Non_Blocking()
{
  static bool isLED_RED_ON = false;
  static bool isLED_YELLOW_ON = false;
  static bool isLED_GREEN_ON = false;

  // Điều khiển LED đỏ nhấp nháy mỗi 1000ms
  if (IsReady(ledStart, 1000))
  {
    if (isLED_RED_ON)
    {
      digitalWrite(LED_RED, LOW);
      Serial.println("NonBlocking LED_RED -> OFF");
    }
    else
    {
      digitalWrite(LED_RED, HIGH);
      Serial.println("NonBlocking LED_RED -> ON");
    }
    isLED_RED_ON = !isLED_RED_ON;
  }

  // Điều khiển LED vàng nhấp nháy mỗi 3000ms
  if (IsReady(ledYellowStart, 1500))
  {
    if (isLED_YELLOW_ON)
    {
      digitalWrite(LED_YELLOW, LOW);
      Serial.println("NonBlocking LED_YELLOW -> OFF");
    }
    else
    {
      digitalWrite(LED_YELLOW, HIGH);
      Serial.println("NonBlocking LED_YELLOW -> ON");
    }
    isLED_YELLOW_ON = !isLED_YELLOW_ON;
  }

  // Điều khiển LED xanh nhấp nháy mỗi 500ms
  if (IsReady(ledGreenStart, 2000))
  {
    if (isLED_GREEN_ON)
    {
      digitalWrite(LED_GREEN, LOW);
      Serial.println("NonBlocking LED_YELLOW -> OFF");
    }
    else
    {
      digitalWrite(LED_GREEN, HIGH);
      Serial.println("NonBlocking LED_YELLOW -> ON");
    }
    isLED_GREEN_ON = !isLED_GREEN_ON;
  }
}
