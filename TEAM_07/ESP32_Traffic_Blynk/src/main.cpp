#include <Arduino.h>
#include <TM1637Display.h> // Thư viện điều khiển TM1637

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6v_xsCQM5"
#define BLYNK_TEMPLATE_NAME "ESP3 LED TM1637"
#define BLYNK_AUTH_TOKEN "Z-8JgCxE9v9KvwKIAB9dUrwQpEOyY2Rr"
// Phải để trước khai báo sử dụng thư viện Blynk

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

#define DHTPIN 16      // Chân GPIO kết nối cảm biến DHT22
#define DHTTYPE DHT22  // Loại cảm biến
DHT dht(DHTPIN, DHTTYPE);

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi


#define btnBLED  23 //Chân kết nối nút bấm
#define pinBLED  21 //Chân kết nối đèn xxanh

#define CLK 18  //Chân kết nối CLK của TM1637
#define DIO 19  //Chân kết nối DIO của TM1637


//Khởi tạo mà hình TM1637
TM1637Display display(CLK, DIO);
BlynkTimer timer;  // Khởi tạo đối tượng timer

// Pin LED
#define rLED  27
#define yLED  26
#define gLED  25

//Pin - Cảm biến quang điện trở
#define ldrPIN 33

// Thời gian đèn (ms)
uint rTIME = 5000;   // 5 giây
uint yTIME = 3000;   // 3 giây
uint gTIME = 10000;   // 10 giây

//Biến toàn cục

// Biến thời gian
ulong currentMiliseconds = 0; //thời gian hiện tại
ulong nextTimeTotal = 0; //biến tổng - chỉ để hiển thị
ulong ledTimeStart = 0;
int currentLED = 0; //đèn led hiện tại đang sángsáng
int tmCounter = rTIME / 1000; // Biến đếm ngược trên bảng
ulong counterTime = 0; //Thời gian bắt đầu đếm ngược
bool blueButtonON = true;     //Trạng thái của nút bấm ON -> đèn Xanh sáng và hiển thị LED TM1637

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();

void sendSensorData() {
  float temperature = dht.readTemperature();  // Đọc nhiệt độ
  float humidity = dht.readHumidity();       // Đọc độ ẩm

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Lỗi đọc cảm biến!");
    return;
  }

  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.print("°C - Độ ẩm: ");
  Serial.print(humidity);
  Serial.println("%");

  Blynk.virtualWrite(V2, temperature);  // Gửi nhiệt độ lên Blynk (V2)
  Blynk.virtualWrite(V3, humidity);     // Gửi độ ẩm lên Blynk (V3)
}
void setup() {
  Serial.begin(115200);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  currentLED = rLED;
  display.setBrightness(0x0f);
  dht.begin();
  
  // Start the WiFi connection
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi

  Serial.println();
  Serial.println("WiFi connected");

  Serial.println("== START ==>");  
  Serial.print("1. RED    => GREEN  "); Serial.print(rTIME / 1000); Serial.println(" (s)");
  digitalWrite(pinBLED, blueButtonON? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); //Đồng bộ trạng thái trạng thái của đèn với Blynk
  
  // Gửi dữ liệu cảm biến mỗi 2 giây
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  currentMiliseconds = millis();
  NonBlocking_Traffic_Light();
  NonBlocking_Traffic_Light_TM1637();
}

// Kiểm tra thời gian không chặn (non-blocking)
bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Xử lý đèn giao thông
void NonBlocking_Traffic_Light() {
  switch (currentLED) {
    case rLED: // Đèn đỏ
      if (IsReady(ledTimeStart, rTIME)) {
        digitalWrite(rLED, LOW);
        digitalWrite(gLED, HIGH);
        currentLED = gLED;
        tmCounter = gTIME / 1000; // Chuyển sang đếm ngược đèn xanh
        Serial.print("2. GREEN  => YELLOW "); Serial.print(gTIME / 1000); Serial.println(" (s)");
      } 
      break;

    case gLED: // Đèn xanh
      if (IsReady(ledTimeStart, gTIME)) {        
        digitalWrite(gLED, LOW);
        digitalWrite(yLED, HIGH);
        currentLED = yLED;
        tmCounter = yTIME / 1000; // Chuyển sang đếm ngược đèn vàng
        Serial.print("3. YELLOW => RED    "); Serial.print(yTIME / 1000); Serial.println(" (s)");        
      }
      break;

    case yLED: // Đèn vàng
      if (IsReady(ledTimeStart, yTIME)) {        
        digitalWrite(yLED, LOW);
        digitalWrite(rLED, HIGH);
        currentLED = rLED;
        tmCounter = rTIME / 1000; // Chuyển về đếm ngược đèn đỏ
        Serial.print("1. RED    => GREEN  "); Serial.print(rTIME / 1000); Serial.println(" (s)");        
      }
      break;
  }  
}

// Hiển thị thời gian còn lại trên TM1637
void NonBlocking_Traffic_Light_TM1637() {
  static ulong lastUpdate = 0;

  if (millis() - lastUpdate >= 1000) { // Cập nhật mỗi giây
    lastUpdate = millis();
    tmCounter--; // Giảm thời gian còn lại

    if (tmCounter < 0) { 
      // Khi hết thời gian, chuyển sang đếm ngược của đèn tiếp theo
      if (currentLED == rLED) tmCounter = gTIME / 1000;
      else if (currentLED == gLED) tmCounter = yTIME / 1000;
      else if (currentLED == yLED) tmCounter = rTIME / 1000;
    }

    // Hiển thị số giây còn lại
    display.showNumberDec(tmCounter, true);
  }
}
