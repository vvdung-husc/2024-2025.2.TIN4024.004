#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>  // Include the DHT library

/* Fill in information from Blynk Device Info here */
 #define BLYNK_TEMPLATE_NAME "ESP32 Blynk LED"
#define BLYNK_AUTH_TOKEN "72PWXm4riLyYaEzU-8e5-wh5WJh7OxiE"
// Phải để trước khai báo sử dụng thư viện Blynk

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

#define btnBLED  23 //Chân kết nối nút bấm
#define pinBLED  21 //Chân kết nối đèn xanh

#define CLK 18  //Chân kết nối CLK của TM1637
#define DIO 19  //Chân kết nối DIO của TM1637

// DHT sensor settings
#define DHTPIN 16    // Chân kết nối DHT sensor (SDA in your Wokwi structure)
#define DHTTYPE DHT22   // Loại DHT sensor (DHT11, DHT22, v.v.)
DHT dht(DHTPIN, DHTTYPE);

// Biến toàn cục
ulong currentMiliseconds = 0; // Thời gian hiện tại - miliseconds 
bool blueButtonON = true;     // Trạng thái của nút bấm ON -> đèn Xanh sáng và hiển thị LED TM1637

// Khởi tạo mà hình TM1637
TM1637Display display(CLK, DIO);

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void updateDHTSensor(); // Hàm để cập nhật giá trị DHT sensor

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
    
  display.setBrightness(0x0f);
  
  // Start the WiFi connection
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); // Kết nối đến mạng WiFi

  Serial.println();
  Serial.println("WiFi connected");

  // Khởi tạo DHT sensor
  Serial.println("Initializing DHT sensor...");
  dht.begin();

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); // Đồng bộ trạng thái của đèn với Blynk
  
  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run();  // Chạy Blynk để cập nhật trạng thái từ Blynk Cloud

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
  updateDHTSensor(); // Cập nhật giá trị DHT sensor
}

// put function definitions here:
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateBlueButton(){
  static ulong lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return; // Hạn chế bấm nút quá nhanh - 50ms mỗi lần bấm
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON); // Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON); // Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
    display.clear();
  }    
}

void uptimeBlynk(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; // Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  // Gửi giá trị lên chân ảo V0 trên ứng dụng Blynk.
  if (blueButtonON){
    display.showNumberDec(value);
  }
}

void updateDHTSensor(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return; // Cập nhật sau mỗi 2 giây
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true); // Nhiệt độ Fahrenheit

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor! Check wiring and sensor type.");
    return;
  }

  Serial.print("Humidity: "); Serial.print(h);
  Serial.print(" %\tTemperature: "); Serial.print(t);
  Serial.print(" *C "); Serial.print(f); Serial.println(" *F");
  
  Blynk.virtualWrite(V2, h); // Gửi giá trị độ ẩm lên chân ảo V2 trên ứng dụng Blynk.
  Blynk.virtualWrite(V3, t); // Gửi giá trị nhiệt độ lên chân ảo V3 trên ứng dụng Blynk.
}

// Được gọi mỗi khi có dữ liệu mới được gửi từ ứng dụng Blynk đến thiết bị.
BLYNK_WRITE(V1) { // virtual_pin định nghĩa trong ứng dụng Blynk
  // Xử lý dữ liệu nhận được từ ứng dụng Blynk
  blueButtonON = param.asInt();  // Lấy giá trị từ ứng dụng Blynk
  if (blueButtonON){
    Serial.println("Blynk -> Blue Light ON");
    digitalWrite(pinBLED, HIGH);
  }
  else {
    Serial.println("Blynk -> Blue Light OFF");
    digitalWrite(pinBLED, LOW);   
    display.clear(); 
  }
}
