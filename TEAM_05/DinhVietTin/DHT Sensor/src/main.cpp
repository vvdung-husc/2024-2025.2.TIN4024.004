#include <Arduino.h>       // Thư viện chuẩn của Arduino
#include <TM1637Display.h> // Thư viện điều khiển màn hình 7 đoạn TM1637

#define BLYNK_TEMPLATE_ID "TMPL6w6iNbExx"                   // ID template của Blynk
#define BLYNK_TEMPLATE_NAME "DHT Sensor"                    // Tên template trên Blynk
#define BLYNK_AUTH_TOKEN "QthBhZPFQut3Q1INIamtXRVgZ9WB8jNH" // Mã token để kết nối với Blynk

#include <WiFi.h>             // Thư viện WiFi cho ESP32
#include <WiFiClient.h>       // Thư viện hỗ trợ client WiFi
#include <BlynkSimpleEsp32.h> // Thư viện Blynk để giao tiếp với Blynk cloud
#include <DHT.h>              // Thư viện cảm biến nhiệt độ và độ ẩm DHT

char ssid[] = "Wokwi-GUEST"; // Tên mạng WiFi
char pass[] = "";            // Mật khẩu mạng WiFi (để trống cho Wokwi)

#define btnBLED 23    // Chân kết nối nút nhấn bật/tắt đèn LED
#define pinBLED 21    // Chân điều khiển đèn LED
#define CLK 18        // Chân Clock của màn hình TM1637
#define DIO 19        // Chân Data của màn hình TM1637
#define DHTPIN 16     // Chân cảm biến DHT22
#define DHTTYPE DHT22 // Loại cảm biến DHT đang sử dụng

TM1637Display display(CLK, DIO); // Khởi tạo màn hình TM1637
DHT dht(DHTPIN, DHTTYPE);        // Khởi tạo cảm biến DHT

ulong currentMiliseconds = 0; // Biến lưu thời gian hiện tại
bool blueButtonON = true;     // Trạng thái đèn LED

// Khai báo các hàm sử dụng trong chương trình
bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();
void readDHT();

void setup()
{
  Serial.begin(115200);           // Khởi động Serial Monitor
  pinMode(pinBLED, OUTPUT);       // Đặt chân đèn LED là OUTPUT
  pinMode(btnBLED, INPUT_PULLUP); // Đặt chân nút nhấn là INPUT_PULLUP
  display.setBrightness(0x0f);    // Đặt độ sáng tối đa cho màn hình TM1637
  dht.begin();                    // Khởi động cảm biến DHT

  Serial.print("Connecting to ");
  Serial.println(ssid);                      // Hiển thị thông tin kết nối WiFi
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Kết nối với Blynk
  Serial.println("WiFi connected");          // Thông báo đã kết nối WiFi

  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW); // Bật/tắt đèn LED theo trạng thái ban đầu
  Blynk.virtualWrite(V1, blueButtonON);             // Gửi trạng thái LED lên Blynk
  Serial.println("== START ==>");                   // In thông báo bắt đầu chương trình
}

void loop()
{
  Blynk.run();                   // Duy trì kết nối với Blynk
  currentMiliseconds = millis(); // Lấy thời gian hiện tại
  uptimeBlynk();                 // Cập nhật số giây chạy lên màn hình TM1637
  updateBlueButton();            // Xử lý nút nhấn để bật/tắt đèn LED
  readDHT();                     // Đọc dữ liệu từ cảm biến DHT
}

// Hàm kiểm tra nếu đủ thời gian trễ thì trả về true
bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond) // Kiểm tra nếu chưa đủ thời gian chờ
    return false;                                // Trả về false để bỏ qua
  ulTimer = currentMiliseconds;                  // Cập nhật lại thời gian chờ
  return true;                                   // Trả về true để thực hiện công việc tiếp theo
}

// Hàm xử lý bật/tắt đèn LED khi nhấn nút
void updateBlueButton()
{
  static ulong lastTime = 0;   // Biến lưu thời gian lần nhấn trước
  static int lastValue = HIGH; // Biến lưu trạng thái trước đó của nút nhấn
  if (!IsReady(lastTime, 50))  // Kiểm tra chống dội phím (debounce)
    return;
  int v = digitalRead(btnBLED); // Đọc trạng thái nút nhấn
  if (v == lastValue)           // Nếu không thay đổi trạng thái thì bỏ qua
    return;
  lastValue = v; // Cập nhật trạng thái mới của nút nhấn
  if (v == LOW)  // Nếu nút nhấn chưa nhả ra thì không làm gì
    return;

  blueButtonON = !blueButtonON;                                      // Đảo trạng thái đèn LED
  Serial.println(blueButtonON ? "Blue Light ON" : "Blue Light OFF"); // Hiển thị trạng thái đèn trên Serial
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);                  // Bật/tắt đèn LED
  Blynk.virtualWrite(V1, blueButtonON);                              // Cập nhật trạng thái đèn lên Blynk

  if (!blueButtonON)
    display.clear(); // Nếu tắt đèn thì xóa hiển thị trên màn hình
}

// Hàm cập nhật số giây chạy lên Blynk và màn hình TM1637
void uptimeBlynk()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) // Kiểm tra nếu đã đủ 1 giây
    return;
  ulong value = lastTime / 1000; // Chuyển millis thành giây
  Blynk.virtualWrite(V0, value); // Gửi số giây chạy lên Blynk

  if (blueButtonON)
    display.showNumberDec(value); // Nếu đèn bật, hiển thị số giây trên màn hình
  else
    display.clear(); // Nếu đèn tắt, xóa màn hình
}

// Hàm đọc dữ liệu từ cảm biến DHT
void readDHT()
{
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 500)) // Kiểm tra nếu đã đủ 0.5 giây
    return;
  float temp = dht.readTemperature(); // Đọc nhiệt độ
  float hum = dht.readHumidity();     // Đọc độ ẩm
  if (isnan(temp) || isnan(hum))      // Kiểm tra nếu dữ liệu lỗi
  {
    Serial.println("Failed to read from DHT sensor!"); // In lỗi
    return;
  }
  Blynk.virtualWrite(V2, temp); // Gửi nhiệt độ lên Blynk
  Blynk.virtualWrite(V3, hum);  // Gửi độ ẩm lên Blynk
}

// Hàm xử lý bật/tắt đèn từ Blynk
BLYNK_WRITE(V1)
{
  blueButtonON = param.asInt(); // Nhận trạng thái từ Blynk
  Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");
  digitalWrite(pinBLED, blueButtonON ? HIGH : LOW); // Bật/tắt đèn LED

  if (!blueButtonON)
    display.clear(); // Nếu tắt đèn thì xóa hiển thị trên màn hình
}
