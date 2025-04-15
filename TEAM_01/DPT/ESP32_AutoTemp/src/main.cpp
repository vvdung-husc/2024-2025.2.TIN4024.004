#define BLYNK_TEMPLATE_ID "TMPL6lCsC-LTG"
#define BLYNK_TEMPLATE_NAME "ESP32FinalTest"
#define BLYNK_AUTH_TOKEN "8KSvJabgCjWzmnXJPMR2HxW2fSP2qXoB"

#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Định nghĩa các tham số màn hình OLED
#define SCREEN_WIDTH 128    // Chiều rộng màn hình OLED
#define SCREEN_HEIGHT 64    // Chiều cao màn hình OLED
#define OLED_RESET -1       // Reset mặc định cho màn hình OLED (không dùng)

// Khởi tạo màn hình OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Định nghĩa chân cảm biến và thiết bị
const int DHT_PIN = 15;      // Chân cảm biến DHT22
const int DHT_TYPE = DHT22;  // Loại cảm biến DHT22

const int LED = 12;  // Chân kết nối đèn LED

// Định nghĩa chân nút bấm và relay
const int button_temp = 26;   // Nút bấm điều khiển nhiệt độ

const int button_Mode = 32;   // Nút bấm thay đổi chế độ

const int relay_temp = 23;    // Relay điều khiển theo nhiệt độ
const int relay_dis = 16;     // Relay điều khiển thiết bị khác

// Định nghĩa các ngưỡng giá trị
float tempThreshold = 32;     // Ngưỡng nhiệt độ


volatile bool relayTemp_State = false;  // Trạng thái relay điều khiển nhiệt độ
volatile bool Mode_State = false;       // Biến lưu trạng thái của chế độ

// Biến lưu thời gian debounce cho các nút điều khiển (tránh rung nút)
volatile unsigned long lastDebounceTimeTemp = 0;
volatile unsigned long lastDebounceTimeMode = 0;

const unsigned long debounceDelay = 200;  // Thời gian chống rung nút 200ms

unsigned long previousMillis = 0;         // Lưu thời gian lần cập nhật trước cho OLED
const long intervaloled = 100;             // Thời gian cập nhật OLED mỗi 100ms

DHT dht(DHT_PIN, DHT_TYPE);  // Khởi tạo đối tượng DHT với chân cảm biến và loại DHT

const char* ssid = "Wokwi-GUEST";           // Tên mạng WiFi
const char* password = "";                  // Mật khẩu WiFi (trong trường hợp này không cần)

WiFiClient espClient;         // Khởi tạo đối tượng WiFiClient cho ESP32

// Hàm kết nối WiFi
void connectWiFi() 
{
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);  // Bắt đầu kết nối WiFi với SSID và password
    while (WiFi.status() != WL_CONNECTED)  // Kiểm tra trạng thái kết nối
    {
      delay(500);
      Serial.print(".");  // In ra dấu "." trong khi chờ kết nối
    }
    Serial.println(" Connected!");  // Thông báo đã kết nối thành công
}

// Ngắt ngoài để điều khiển relay (nhiệt độ)
void IRAM_ATTR buttonTempInterrupt() {
  if ((millis() - lastDebounceTimeTemp) > debounceDelay && !Mode_State) {  // Kiểm tra chống rung và chế độ
    relayTemp_State = !relayTemp_State;  // Thay đổi trạng thái relay nhiệt độ
    digitalWrite(relay_temp, relayTemp_State ? HIGH : LOW);  // Bật/tắt relay
    lastDebounceTimeTemp = millis();  // Cập nhật thời gian debounce
  }
}

// Ngắt ngoài để điều khiển chế độ (Mode)
void IRAM_ATTR buttonModeInterrupt() {
  if ((millis() - lastDebounceTimeMode) > debounceDelay) {
    Mode_State = !Mode_State;  // Thay đổi trạng thái chế độ (AUTO/HAND)
    digitalWrite(LED, Mode_State ? HIGH : LOW);  // Bật/tắt LED báo hiệu chế độ
    lastDebounceTimeMode = millis();
  }
}

// Khởi tạo các cảm biến và thiết bị
void setup() {
  Serial.begin(115200);  // Khởi tạo cổng Serial
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);  // Kết nối Blynk

  // Cấu hình chân cảm biến và relay
  pinMode(relay_dis, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(button_temp, INPUT_PULLUP);   
  pinMode(button_Mode, INPUT_PULLUP);  
  pinMode(relay_temp, OUTPUT);
  dht.begin();  // Khởi động cảm biến DHT

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Khởi động màn hình OLED
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.display();

  // Kết nối WiFi
  connectWiFi();

  // Thiết lập ngắt ngoài cho các nút bấm
  attachInterrupt(digitalPinToInterrupt(button_temp), buttonTempInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_Mode), buttonModeInterrupt, FALLING);
}

// Hàm đọc nhiệt độ và độ ẩm và điều khiển Relay 
void readTemperatureAndControlLED(float &temperature) {
  temperature = dht.readTemperature();  // Đọc nhiệt độ từ cảm biến

  if (Mode_State) {  // Chỉ điều khiển trong chế độ AUTO
    if (temperature >= tempThreshold) {
      digitalWrite(relay_temp, HIGH); relayTemp_State = true;  // Bật relay nhiệt độ nếu vượt ngưỡng
    } else if (temperature < tempThreshold) {
      digitalWrite(relay_temp, LOW); relayTemp_State = false;  // Tắt relay nhiệt độ nếu dưới ngưỡng
    }
  }
}

// Hàm hiển thị dữ liệu lên OLED
void displayData(float temp) {
  display.clearDisplay(); // Xóa màn hình OLED
  display.setTextSize(0.8); // Thiết lập kích thước chữ
  display.setTextColor(WHITE); // Thiết lập màu chữ
  display.setCursor(0, 0); // Đặt vị trí con trỏ trên màn hình
  display.print("Temp: "); // In ra tiêu đề nhiệt độ
  display.print((int)temp); // In ra giá trị nhiệt độ
  display.print((char)247); // In ký hiệu độ C
  display.print("C");
  display.print("MODE: "); // In ra chế độ hoạt động
  display.println(Mode_State ? "AUTO" : "HAND"); // Hiển thị chế độ AUTO hoặc HAND
  display.print("relay_temp: "); // In ra trạng thái relay nhiệt độ
  display.println(relayTemp_State ? "ON" : "OFF"); // Hiển thị ON/OFF
  display.display(); // Cập nhật màn hình
}

// Hàm hiển thị trạng thái hệ thống và relay lên Serial
void displayStatusToSerial(float temp) {
  Serial.println("===== System Status ====="); // In tiêu đề trạng thái
  Serial.print("Temperature: "); // In ra tiêu đề nhiệt độ
  Serial.print(temp); // In ra giá trị nhiệt độ
  Serial.println(" °C"); // Kết thúc dòng
  Serial.print("Mode: "); // In ra chế độ hoạt động
  Serial.println(Mode_State ? "AUTO" : "HAND"); // Hiển thị chế độ
  Serial.print("Relay Temp: "); // In ra trạng thái relay nhiệt độ
  Serial.println(relayTemp_State ? "ON" : "OFF"); // Hiển thị ON/OFF
  Serial.println("=========================="); // In dòng ngăn cách
}
// Hàm gửi dữ liệu lên Blynk
void sendSensorDataToBlynk() {
  float temp = dht.readTemperature();
  Blynk.virtualWrite(V0, temp); // V0 là widget nhiệt độ trên app
  Blynk.virtualWrite(V1, Mode_State ? 1 : 0); // Cập nhật chế độ
  Blynk.virtualWrite(V2, relayTemp_State ? 1 : 0); // Cập nhật trạng thái relay
}
// Điều khiển chuyển đổi chế độ (AUTO/HAND) từ app Blynk
BLYNK_WRITE(V1) {
  int mode = param.asInt(); // Lấy giá trị từ app (0 hoặc 1)
  Mode_State = (mode == 1); // Nếu 1 thì AUTO, 0 là HAND
  digitalWrite(LED, Mode_State ? HIGH : LOW); // Cập nhật LED báo hiệu
}

// Điều khiển bật/tắt relay nhiệt độ từ app Blynk (chỉ hoạt động khi HAND)
BLYNK_WRITE(V2) {
  if (!Mode_State) { // Chỉ cho phép khi ở chế độ HAND
    int value = param.asInt(); // Lấy trạng thái nút từ app
    relayTemp_State = (value == 1);
    digitalWrite(relay_temp, relayTemp_State ? HIGH : LOW);
  }
}
unsigned long lastBlynkSend = 0;
const long intervalBlynk = 1000;

// Vòng lặp chính
void loop() {
  Blynk.run(); // Chạy Blynk
  
  unsigned long currentMillis = millis(); // Lấy thời gian hiện tại
          
  // Nếu đã đến lúc cập nhật (500ms trôi qua)
  if (currentMillis - previousMillis >= intervaloled) {
    previousMillis = currentMillis; // Cập nhật lại thời gian lần trước

    float temp; // Khai báo biến cho các cảm biến

    // Đọc cảm biến và điều khiển LED
    readTemperatureAndControlLED(temp); // Đọc nhiệt độ và độ ẩm

    // Hiển thị dữ liệu lên màn hình OLED
    displayData(temp); // Hiển thị dữ liệu trên OLED

    // Hiển thị trạng thái lên Serial
    displayStatusToSerial(temp); // Hiển thị trạng thái hệ thống trên Serial
  }
  
  // Blynk
  if (currentMillis - lastBlynkSend >= intervalBlynk) {
    lastBlynkSend = currentMillis;
    sendSensorDataToBlynk();
  }
}
