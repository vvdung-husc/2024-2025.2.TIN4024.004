#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

// Trần Văn Thanh
// #define BLYNK_TEMPLATE_ID "TMPL6VbNuRWM1"
// #define BLYNK_TEMPLATE_NAME "ESP32TrafficBlynk"
// #define BLYNK_AUTH_TOKEN "X2za5Tymj_cH63wGgnL_cNIpWFKXqH2d"

//Trần Văn Minh Nhật

// #define BLYNK_TEMPLATE_ID "TMPL6rKfVu6HP"
// #define BLYNK_TEMPLATE_NAME "ESMART"
// #define BLYNK_AUTH_TOKEN "NVohCxt35pdmPIROGthShHSki6xWgVdf"

// Nguyễn Trần Viết Thắng
//#define BLYNK_TEMPLATE_ID "TMPL6lYwGWEqx"
//#define BLYNK_TEMPLATE_NAME "ESMART"
//#define BLYNK_AUTH_TOKEN "HwY9Ws1btOsgiJyMzkzmCs2p2-jFjC4H"

//Đoàn Anh Tú
#define BLYNK_TEMPLATE_ID "TMPL6chk8bhgy"
#define BLYNK_TEMPLATE_NAME "DoanAnhTu"
#define BLYNK_AUTH_TOKEN "hMXh6gasWGZm9e26p6zIFIR4KseckmGu"

//Phạm Văn Trí
#define BLYNK_TEMPLATE_ID "TMPL6mS6n5bhv"
#define BLYNK_TEMPLATE_NAME "4"
#define BLYNK_AUTH_TOKEN "z2XNBK32elbzhvNlnPr1XvFu35ez7E-m"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

#define btnBLED  23 //Chân kết nối nút bấm
#define pinBLED  21 //Chân kết nối đèn xanh

// Pin - Các đèn LEDLED
#define rLED 27
#define yLED 26
#define gLED 25

// Pin - TM1637TM1637
#define CLK 18
#define DIO 19

#define DHTPIN 16 //Chân kết nối cảm biến DHT22
#define DHTTYPE DHT22 //Loại cảm biến

//Pin - Cảm biến quang điện trở
#define ldrPIN  33

//1000 ms = 1 seconds
uint rTIME = 5000;  //thời gian chờ đèn đỏ    5 giây
uint yTIME = 3000;  //thời gian chờ đèn vàng  3 giây
uint gTIME = 10000; //thời gian chờ đèn xanh 10 giây

ulong currentMiliseconds = 0; // miliseconds hiện tại
ulong ledTimeStart = 0;       // thời gian bắt đầu chạy LED
ulong nextTimeTotal = 0;      // biến tổng - chỉ để hiển thị
int currentLED = 0;           // đèn LED hiện tại đang sáng
int tmCounter = rTIME / 1000; // biến đếm ngược trên bảng
ulong counterTime = 0;        // thời gian bắt đầu đếm ngược

bool blueButtonON = true;     //Trạng thái của nút bấm ON -> đèn Xanh sáng và hiển thị LED TM1637
ulong ledOnTime = 0; //Biến theo dõi thời gian đèn sáng

int darkThreshold = 1000; // ngưỡng ánh sáng < 1000 => DARK

//Khởi tạo mà hình TM1637
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);


bool IsReady(ulong &ulTimer, uint32_t milisecond);

void updateBlueButton();
void uptimeBlynk();
void readDHTSensor();
void updateLedTime();

void NonBlocking_Traffic_Light_TM1637();
bool isDark();
void YellowLED_Blink();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  dht.begin(); //Khởi động cảm biến DHT22
  
  // Start the WiFi connection
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi

  Serial.println();
  Serial.println("WiFi connected");

  
  digitalWrite(pinBLED, blueButtonON? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); //Đồng bộ trạng thái trạng thái của đèn với Blynk

  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);

  pinMode(ldrPIN, INPUT);

  tmCounter = (rTIME / 1000) - 1;
  display.setBrightness(7);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  display.showNumberDec(tmCounter--, true, 2, 2);

  currentLED = rLED;
  nextTimeTotal += rTIME;
  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  ");
  Serial.print((nextTimeTotal / 1000) % 60);
  Serial.println(" (ms)");
}

void loop()
{
  // put your main code here, to run repeatedly:
  currentMiliseconds = millis();

  uptimeBlynk();
  updateBlueButton();
  readDHTSensor();
  //updateLedTime();

  if (isDark())
    YellowLED_Blink(); // Nếu trời tối => Nhấp nháy đèn vàng
  else
    NonBlocking_Traffic_Light_TM1637(); // Hiển thị đèn giao thông
}

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

void NonBlocking_Traffic_Light_TM1637()
{
  if (!blueButtonON) return; // Nếu đèn xanh dương tắt -> Không chạy hệ thống đèn giao thông

  bool bShowCounter = false;
  switch (currentLED)
  {
  case rLED: // Đèn đỏ: 5 giây
    if (IsReady(ledTimeStart, rTIME))
    {
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, HIGH);
      currentLED = gLED;
      nextTimeTotal += gTIME;
      tmCounter = (gTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
      Serial.print("2. GREEN  => YELLOW ");
      Serial.print((nextTimeTotal / 1000) % 60);
      Serial.println(" (ms)");
    }
    break;
  case gLED: // Đèn xanh: 7 giây
    if (IsReady(ledTimeStart, gTIME))
    {
      digitalWrite(gLED, LOW);
      digitalWrite(yLED, HIGH);
      currentLED = yLED;
      nextTimeTotal += yTIME;
      tmCounter = (yTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
      Serial.print("3. YELLOW => RED    ");
      Serial.print((nextTimeTotal / 1000) % 60);
      Serial.println(" (ms)");
    }
    break;

  case yLED: // Đèn vàng: 2 giây
    if (IsReady(ledTimeStart, yTIME))
    {
      digitalWrite(yLED, LOW);
      digitalWrite(rLED, HIGH);
      currentLED = rLED;
      nextTimeTotal += rTIME;
      tmCounter = (rTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
      Serial.print("1. RED    => GREEN  ");
      Serial.print((nextTimeTotal / 1000) % 60);
      Serial.println(" (ms)");
    }
    break;
  }
  if (!bShowCounter)
    bShowCounter = IsReady(counterTime, 1000);
  if (bShowCounter)
  {
    display.showNumberDec(tmCounter--, true, 2, 2);
  }
}

bool isDark()
{
  static ulong darkTimeStart = 0; // lưu thời gian của việc đọc cảm biến
  static uint16_t lastValue = 0;  // lưu giá trị gần nhất của cảm biến
  static bool bDark = false;      // true: value > darkThreshold

  if (!IsReady(darkTimeStart, 50))
    return bDark;                      // 50ms đọc cảm biến 1 lần
  uint16_t value = analogRead(ldrPIN); // đọc cảm biến theo chế đố tương tự
  if (value == lastValue)
    return bDark; // vẫn bằng giá trị củ

  if (value < darkThreshold)
  {
    if (!bDark && blueButtonON) // Chỉ cho phép nhấp nháy khi blueButtonON == true
    {
      digitalWrite(currentLED, LOW);
      Serial.print("DARK value: ");
      Serial.println(value);
      Blynk.virtualWrite(V4, value);
    }
    // bDark = true;
    bDark = blueButtonON; // Chỉ đặt `bDark = true` nếu `blueButtonON` bật
  }
  else
  {
    if (bDark)
    {
      digitalWrite(yLED, LOW);      
      Serial.print("LIGHT value: ");
      Serial.println(value);
    }
    bDark = false;
  }

  lastValue = value;
  return bDark;
}

void YellowLED_Blink()
{
  if (!blueButtonON) return; // Nếu đèn xanh dương tắt -> Không nhấp nháy đèn vàng

  static ulong yLedStart = 0;
  static bool isON = false;

  if (!IsReady(yLedStart, 1000))
    return;
  if (!isON)
    digitalWrite(yLED, HIGH);
  else
    digitalWrite(yLED, LOW);
  isON = !isON;
}

void updateBlueButton(){
  static ulong lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50)) return; //Hạn chế bấm nút quá nhanh - 50ms mỗi lần bấm

  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  blueButtonON = !blueButtonON; // Đảo trạng thái

  if (blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    
    if (currentLED == rLED) digitalWrite(rLED, HIGH);
    if (currentLED == yLED) digitalWrite(yLED, HIGH);
    if (currentLED == gLED) digitalWrite(gLED, HIGH);
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    
    // Tắt cả 3 đèn giao thông
    digitalWrite(rLED, LOW);
    digitalWrite(yLED, LOW);
    digitalWrite(gLED, LOW);

    display.clear();
  }    
  Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
}

void uptimeBlynk() {
  static ulong lastTime = 0;
  static ulong runTime = 0; // Biến lưu thời gian chạy của đèn

  if (!IsReady(lastTime, 1000)) return; // Cập nhật mỗi giây

  if (blueButtonON) {  
    runTime++; // Chỉ tăng thời gian khi đèn sáng
    Blynk.virtualWrite(V0, runTime);  // Gửi lên Blynk V0
  }
}

void readDHTSensor() {
  static ulong lastTime = 0;
  if (!blueButtonON) return;  // Nếu đèn tắt thì không đọc cảm biến
  if (!IsReady(lastTime, 2000)) return; // Cập nhật mỗi 2 giây

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Lỗi đọc cảm biến DHT!");
    return;
  }

  Serial.print("Nhiệt độ: "); Serial.print(temperature); Serial.println("°C");
  Serial.print("Độ ẩm: "); Serial.print(humidity); Serial.println("%");

  Blynk.virtualWrite(V2, temperature);
  Blynk.virtualWrite(V3, humidity);
}

// void updateLedTime() {
//   static ulong lastUpdate = 0;
//   if (!IsReady(lastUpdate, 1000)) return;

//   if (blueButtonON) {
//     ledOnTime++;
//     Blynk.virtualWrite(V4, ledOnTime);
//   }
// }

//được gọi mỗi khi có dữ liệu mới được gửi từ ứng dụng Blynk đến thiết bị.
BLYNK_WRITE(V1) { //virtual_pin định nghĩa trong ứng dụng Blynk
  // Xử lý dữ liệu nhận được từ ứng dụng Blynk
  blueButtonON = param.asInt();  // Lấy giá trị từ ứng dụng Blynk

  if (blueButtonON){
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  }
  else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);

    // Tắt cả 3 đèn giao thông
    digitalWrite(rLED, LOW);
    digitalWrite(yLED, LOW);
    digitalWrite(gLED, LOW);

    display.clear(); 
  }
}
