#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>
/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6lYwGWEqx"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "HwY9Ws1btOsgiJyMzkzmCs2p2-jFjC4H"
// Phải để trước khai báo sử dụng thư viện Blynk

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// char ssid[] = "CNTT-MMT";  //Tên mạng WiFi
// char pass[] = "13572468";             //Mật khẩu mạng WiFi

#define btnBLED  23 //Chân kết nối nút bấm
#define pinBLED  21 //Chân kết nối đèn xxanh

#define DHTPIN 16      // Chân kết nối cảm biến DHT22
#define DHTTYPE DHT22  // Loại cảm biến

#define CLK 18  //Chân kết nối CLK của TM1637
#define DIO 19  //Chân kết nối DIO của TM1637

DHT dht(DHTPIN, DHTTYPE);

//Khởi tạo mà hình TM1637
TM1637Display display(CLK, DIO);


//Biến toàn cục
ulong currentMiliseconds = 0; //Thời gian hiện tại - miliseconds 
bool blueButtonON = true;     //Trạng thái của nút bấm ON -> đèn Xanh sáng và hiển thị LED TM1637
ulong lastMillis = 0;

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void updateBlueButton();
void uptimeBlynk();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);
  
  dht.begin();
  display.setBrightness(0x0f);
  display.showNumberDec(0, false);
  // Start the WiFi connection
  Serial.print("Connecting to ");Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi

  Serial.println();
  Serial.println("WiFi connected");

  
  digitalWrite(pinBLED, blueButtonON? HIGH : LOW);  
  Blynk.virtualWrite(V1, blueButtonON); //Đồng bộ trạng thái trạng thái của đèn với Blynk
  
  Serial.println("== START ==>");
}

void loop() {  
  Blynk.run();  //Chạy Blynk để cập nhật trạng thái từ Blynk Cloud

  if (millis() - lastMillis >= 2000) { // Cập nhật mỗi 2 giây
    lastMillis = millis();

    float temperature = dht.readTemperature(); // Đọc nhiệt độ (°C)
    float humidity = dht.readHumidity();       // Đọc độ ẩm (%)

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Lỗi đọc cảm biến DHT22!");
      return;
    }

    Serial.print("Nhiệt độ: "); Serial.print(temperature); Serial.print("°C, ");
    Serial.print("Độ ẩm: "); Serial.print(humidity); Serial.println("%");

    display.showNumberDec((int)temperature, false); // Hiển thị nhiệt độ lên TM1637

    Blynk.virtualWrite(V2, temperature); // Gửi nhiệt độ lên Blynk (V2)
    Blynk.virtualWrite(V3, humidity);    // Gửi độ ẩm lên Blynk (V3)
  }

  currentMiliseconds = millis();
  uptimeBlynk();
  updateBlueButton();
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
  if (!IsReady(lastTime, 50)) return; //Hạn chế bấm nút quá nhanh - 50ms mỗi lần bấm
  int v = digitalRead(btnBLED);
  if (v == lastValue) return;
  lastValue = v;
  if (v == LOW) return;

  if (!blueButtonON){
    Serial.println("Blue Light ON");
    digitalWrite(pinBLED, HIGH);
    blueButtonON = true;
    Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
  }
  else {
    Serial.println("Blue Light OFF");
    digitalWrite(pinBLED, LOW);    
    blueButtonON = false;
    Blynk.virtualWrite(V1, blueButtonON);//Gửi giá trị lên chân ảo V1 trên ứng dụng Blynk.
    display.clear();
  }    
}

void uptimeBlynk(){
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; //Kiểm tra và cập nhật lastTime sau mỗi 1 giây
  ulong value = lastTime / 1000;
  Blynk.virtualWrite(V0, value);  //Gửi giá trị lên chân ảo V0 trên ứng dụng Blynk.
  if (blueButtonON){
    display.showNumberDec(value);
  }
}

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
    display.clear(); 
  }
}