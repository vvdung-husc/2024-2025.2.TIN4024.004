#include <Arduino.h>
#include <TM1637Display.h> // Thư viện điều khiển TM1637

// TRAN HOAI DUC
#define BLYNK_TEMPLATE_ID "TMPL6v_xsCQM5"
#define BLYNK_TEMPLATE_NAME "ESP3 LED TM1637"
#define BLYNK_AUTH_TOKEN "Z-8JgCxE9v9KvwKIAB9dUrwQpEOyY2Rr"

// NGUYEN QUANG HUY
#define BLYNK_TEMPLATE_ID "TMPL6fc0FgBst"
#define BLYNK_TEMPLATE_NAME "ESP32 Traffic Blynk"
#define BLYNK_AUTH_TOKEN "S_uZAuR4Lh0Yy4rxzZTi2p3QaSZ10jX6"

// HO PHUOC THAI
//  #define BLYNK_TEMPLATE_ID "TMPL65LJA8v6C"
//  #define BLYNK_TEMPLATE_NAME "TrafficBlynk"
//  #define BLYNK_AUTH_TOKEN "4S1HvPEtHMoVDwcMLDlX92kNawqRBed4"

// NGUYEN DINH QUAN
#define BLYNK_TEMPLATE_ID "TMPL6doIf-emq"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "WDUBaVmcVnEYcgG4ijgys67IltCGmU6x"

// NGUYEN DUC KHANH
#define BLYNK_TEMPLATE_ID "TMPL69Hubiw5q"
#define BLYNK_TEMPLATE_NAME "nduckhanh1"
#define BLYNK_AUTH_TOKEN "S4MKiJT7JdJf6qTBsK75WjLOB4QjDd4f"

// Phải để trước khai báo sử dụng thư viện Blynk

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

#define DHTPIN 16     // Chân GPIO kết nối cảm biến DHT22
#define DHTTYPE DHT22 // Loại cảm biến
DHT dht(DHTPIN, DHTTYPE);

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST"; // Tên mạng WiFi
char pass[] = "";            // Mật khẩu mạng WiFi

#define btnBLED 23 // Chân kết nối nút bấm
#define pinBLED 21 // Chân kết nối đèn xxanh

#define CLK 18 // Chân kết nối CLK của TM1637
#define DIO 19 // Chân kết nối DIO của TM1637

// Khởi tạo mà hình TM1637
TM1637Display display(CLK, DIO);
BlynkTimer timer; // Khởi tạo đối tượng timer

// Pin LED
#define rLED 27
#define yLED 26
#define gLED 25

// Pin - Cảm biến quang điện trở
#define ldrPIN 33
#define LDR_THRESHOLD 1000 // Ngưỡng ánh sáng để phát hiện trời tối
bool nightMode = false;    // Biến kiểm tra có bật chế độ đèn vàng nhấp nháy không

// Thời gian đèn (ms)
uint rTIME = 5000;  // 5 giây
uint yTIME = 3000;  // 3 giây
uint gTIME = 10000; // 10 giây

// Biến toàn cục

// Biến thời gian
ulong currentMiliseconds = 0; // thời gian hiện tại
ulong nextTimeTotal = 0;      // biến tổng - chỉ để hiển thị
ulong ledTimeStart = 0;
int currentLED = 0;           // đèn led hiện tại đang sángsáng
int tmCounter = rTIME / 1000; // Biến đếm ngược trên bảng
ulong counterTime = 0;        // Thời gian bắt đầu đếm ngược
bool blueButtonON = true;     // Trạng thái của nút bấm ON -> đèn Xanh sáng và hiển thị LED TM1637
int ldrThreshold = 1000;      // Giá trị mặc định, có thể điều chỉnh từ Blynk

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void NonBlocking_Traffic_Light();
void NonBlocking_Traffic_Light_TM1637();
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();

void sendSensorData()
{
  float temperature = dht.readTemperature(); // Đọc nhiệt độ
  float humidity = dht.readHumidity();       // Đọc độ ẩm

  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("Lỗi đọc cảm biến!");
    return;
  }

  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.print("°C - Độ ẩm: ");
  Serial.print(humidity);
  Serial.println("%");

  Blynk.virtualWrite(V2, temperature); // Gửi nhiệt độ lên Blynk (V2)
  Blynk.virtualWrite(V3, humidity);    // Gửi độ ẩm lên Blynk (V3)
}

void checkLDR()
{
  static ulong lastCheck = 0;
  static int lastLDRValue = -1; // Lưu giá trị cũ của LDR

  if (!IsReady(lastCheck, 2000))
    return; // Kiểm tra mỗi 2 giây

  int ldrValue = analogRead(ldrPIN);

  if (ldrValue != lastLDRValue)
  { // Chỉ cập nhật khi có thay đổi
    lastLDRValue = ldrValue;
    Serial.print("Giá trị LDR: ");
    Serial.println(ldrValue);
    Blynk.virtualWrite(V4, ldrThreshold); // Cập nhật ngưỡng lên Blynk

    if (ldrValue < ldrThreshold)
    { // Nếu trời tối
      if (!nightMode)
      {
        Serial.println("BAN ĐÊM: Đèn vàng nhấp nháy");
        nightMode = true;
      }
    }
    else
    { // Nếu trời sáng
      if (nightMode)
      {
        Serial.println("BAN NGÀY: Đèn giao thông bình thường");
        nightMode = false;
      }
    }
  }
}

BLYNK_WRITE(V4)
{
  int newThreshold = param.asInt();
  if (newThreshold != ldrThreshold)
  { // Tránh cập nhật lặp lại vô hạn
    ldrThreshold = newThreshold;
    Serial.print("Cập nhật ngưỡng LDR từ Blynk: ");
    Serial.println(ldrThreshold);

    // Kiểm tra ngay lập tức trời sáng hay tối
    int ldrValue = analogRead(ldrPIN);
    Serial.print("Giá trị LDR hiện tại: ");
    Serial.println(ldrValue);

    if (ldrValue < ldrThreshold)
    { // Trời tối
      if (!nightMode)
      {
        Serial.println("BAN ĐÊM: Đèn vàng nhấp nháy");
        nightMode = true;
      }
    }
    else
    { // Trời sáng
      if (nightMode)
      {
        Serial.println("BAN NGÀY: Đèn giao thông bình thường");
        nightMode = false;
      }
    }
  }
}

void blinkYellowLight()
{
  static ulong lastBlink = 0;
  static bool ledState = false;

  if (IsReady(lastBlink, 500))
  { // Nhấp nháy mỗi 500ms
    ledState = !ledState;
    digitalWrite(rLED, LOW);
    digitalWrite(gLED, LOW);
    digitalWrite(pinBLED, LOW);
    digitalWrite(yLED, ledState ? HIGH : LOW);
    display.clear(); // Xóa màn hình TM1637 khi ở chế độ nhấp nháy
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(rLED, OUTPUT);
  pinMode(yLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(pinBLED, OUTPUT); // Thêm khai báo này

  digitalWrite(yLED, LOW);
  digitalWrite(gLED, LOW);
  digitalWrite(rLED, HIGH);
  digitalWrite(pinBLED, LOW); // Đảm bảo đèn tắt ban đầu
  currentLED = rLED;
  display.setBrightness(0x0f);
  dht.begin();

  // Start the WiFi connection
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Kết nối đến mạng WiFi

  Serial.println();
  Serial.println("WiFi connected");

  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  ");
  Serial.print(rTIME / 1000);
  Serial.println(" (s)");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON); // Đồng bộ trạng thái trạng thái của đèn với Blynk
  // Gửi giá trị ngưỡng LDR lên Blynk sau khi kết nối
  Blynk.virtualWrite(V4, ldrThreshold);
  // Gửi dữ liệu cảm biến mỗi 2 giây
  timer.setInterval(2000L, sendSensorData);
}
void loop()
{
  Blynk.run();
  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();

  checkLDR(); // Kiểm tra cảm biến ánh sáng
  if (blueButtonON)
  {
    digitalWrite(pinBLED, HIGH); // Bật đèn
  }
  else
  {
    digitalWrite(pinBLED, LOW); // Tắt đèn
  }

  if (nightMode)
  {
    blinkYellowLight(); // Chạy chế độ đèn vàng nhấp nháy
  }
  else
  {
    NonBlocking_Traffic_Light(); // Chạy chế độ đèn giao thông bình thường
    NonBlocking_Traffic_Light_TM1637();
  }

  timer.run();
}

// Kiểm tra thời gian không chặn (non-blocking)
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

// Xử lý đèn giao thông
void NonBlocking_Traffic_Light()
{
  switch (currentLED)
  {
  case rLED: // Đèn đỏ
    if (IsReady(ledTimeStart, rTIME))
    {
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, HIGH);
      currentLED = gLED;
      tmCounter = gTIME / 1000; // Chuyển sang đếm ngược đèn xanh
      Serial.print("2. GREEN  => YELLOW ");
      Serial.print(gTIME / 1000);
      Serial.println(" (s)");
    }
    break;

  case gLED: // Đèn xanh
    if (IsReady(ledTimeStart, gTIME))
    {
      digitalWrite(gLED, LOW);
      digitalWrite(yLED, HIGH);
      currentLED = yLED;
      tmCounter = yTIME / 1000; // Chuyển sang đếm ngược đèn vàng
      Serial.print("3. YELLOW => RED    ");
      Serial.print(yTIME / 1000);
      Serial.println(" (s)");
    }
    break;

  case yLED: // Đèn vàng
    if (IsReady(ledTimeStart, yTIME))
    {
      digitalWrite(yLED, LOW);
      digitalWrite(rLED, HIGH);
      currentLED = rLED;
      tmCounter = rTIME / 1000; // Chuyển về đếm ngược đèn đỏ
      Serial.print("1. RED    => GREEN  ");
      Serial.print(rTIME / 1000);
      Serial.println(" (s)");
    }
    break;
  }
}

// 🔹 Hàm bật/tắt pinBLED cùng với TM1637
void updateBlueButton()
{
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50))
    return;
  int v = digitalRead(btnBLED);
  if (v == lastValue)
    return;
  lastValue = v;
  if (v == LOW)
    return;

  blueButtonON = !blueButtonON; // Đảo trạng thái đèn

  Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
  Blynk.virtualWrite(V1, blueButtonON); // Đồng bộ với Blynk

  if (!blueButtonON)
    display.clear();
}
void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000))
    return; // Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value); // Gửi giá trị lên chân ảo V0 trên ứng dụng Blynk.
}

// được gọi mỗi khi có dữ liệu mới được gửi từ ứng dụng Blynk đến thiết bị.
BLYNK_WRITE(V1)
{ // virtual_pin định nghĩa trong ứng dụng Blynk
  // Xử lý dữ liệu nhận được từ ứng dụng Blynk
  blueButtonON = param.asInt(); // Lấy giá trị từ ứng dụng Blynk
  if (blueButtonON)
  {
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  }
  else
  {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);
    display.clear();
  }
}
// Hiển thị thời gian trên TM1637 và đồng bộ với pinBLED
void NonBlocking_Traffic_Light_TM1637()
{
  static ulong lastUpdate = 0;
  if (!IsReady(lastUpdate, 1000))
    return;

  tmCounter--;
  if (tmCounter < 0)
  {
    if (currentLED == rLED)
      tmCounter = gTIME / 1000;
    else if (currentLED == gLED)
      tmCounter = yTIME / 1000;
    else if (currentLED == yLED)
      tmCounter = rTIME / 1000;
  }

  if (blueButtonON)
  {
    display.showNumberDec(tmCounter, false, 2, 2);
    digitalWrite(pinBLED, HIGH); // Bật đèn khi TM1637 hiển thị
  }
  else
  {
    display.clear();
    digitalWrite(pinBLED, LOW); // Tắt đèn khi TM1637 tắt
  }
}