
// Wokwi sử dụng mạng WiFi "Wokwi-GUEST" không cần mật khẩu cho việc chạy mô phỏng
char ssid[] = "Wokwi-GUEST";  //Tên mạng WiFi
char pass[] = "";             //Mật khẩu mạng WiFi

// char ssid[] = "CNTT-MMT";  //Tên mạng WiFi
// char pass[] = "13572468";             //Mật khẩu mạng WiFi

#define btnBLED  23 
#define pinBLED  21 
#define DHTPIN 16      
#define DHTTYPE DHT22  
#define CLK 18  
#define DIO 19  

#define RED_LED 27
#define YELLOW_LED 26
#define GREEN_LED 25
#define ldrPIN  32

DHT dht(DHTPIN, DHTTYPE);

//1000 ms = 1 seconds
uint rTIME = 5000;  //thời gian chờ đèn đỏ    5 giây
uint yTIME = 3000;  //thời gian chờ đèn vàng  3 giây
uint gTIME = 7000; //thời gian chờ đèn xanh 7 giây

ulong currentMiliseconds = 0; // miliseconds hiện tại
ulong ledTimeStart = 0;       // thời gian bắt đầu chạy LED
ulong nextTimeTotal = 0;      // biến tổng - chỉ để hiển thị
int currentLED = 0;           // đèn LED hiện tại đang sáng
int tmCounter = rTIME / 1000; // biến đếm ngược trên bảng
ulong counterTime = 0;        // thời gian bắt đầu đếm ngược
bool blueButtonON = true;     // trạng thái đèn xanh
int value =0;                 // giá trị cảm biến
bool useBlynkValue = false;   // sử dụng giá trị từ Blynk
bool tm1637ON = true;          // trạng thái hiển thị TM1637

int darkThreshold = 1000; // Ngưỡng đọc cảm biến

TM1637Display display(CLK, DIO); // Khởi tạo đối tượng hiển thị TM1637

bool IsReady(ulong &ulTimer, uint32_t milisecond); // Hàm kiểm tra thời gian
void NonBlocking_Traffic_Light_TM1637();         // Hàm hiển thị đèn giao thông
bool isDark();                                 // Hàm kiểm tra trời tối
void YellowLED_Blink();                      // Hàm nhấp nháy đèn vàng
void DoamBlynk();                          // Hàm đọc độ ẩm từ cảm biến DHT22
void NhietdoBlynk();                  // Hàm đọc nhiệt độ từ cảm biến DHT22
void updateBlueButton();          // Hàm cập nhật trạng thái nút bấm

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(ldrPIN, INPUT);
  pinMode(pinBLED, OUTPUT);

  tmCounter = (rTIME / 1000) - 1;
  display.setBrightness(7);

  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  display.showNumberDec(tmCounter--, true, 2, 2);

  currentLED = RED_LED;
  nextTimeTotal += rTIME;
  Serial.println("== START ==>");
  Serial.print("1. RED    => GREEN  ");
  Serial.print((nextTimeTotal / 1000) % 60);
  Serial.println(" (ms)");

    // Start the WiFi connection
    Serial.print("Connecting to ");Serial.println(ssid);
    Blynk.begin(BLYNK_AUTH_TOKEN,ssid, pass); //Kết nối đến mạng WiFi

    Serial.println();
    Serial.println("WiFi connected");

    
    digitalWrite(pinBLED, blueButtonON? HIGH : LOW);  
    Blynk.virtualWrite(V1, blueButtonON); //Đồng bộ trạng thái trạng thái của đèn với Blynk
    
    Serial.println("== START ==>");
}

void loop()
{
    Blynk.run();
  // put your main code here, to run repeatedly:
  currentMiliseconds = millis();
  DoamBlynk();
  NhietdoBlynk();
  updateBlueButton();
  if (isDark()){
    YellowLED_Blink();
    display.clear();
  }
  else{
    NonBlocking_Traffic_Light_TM1637(); 
  }
    
}

bool IsReady(ulong &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

void DoamBlynk(){
    float h = dht.readHumidity();
    Blynk.virtualWrite(V3, h);  
  }
  
void NhietdoBlynk(){
    float t = dht.readTemperature();
    Blynk.virtualWrite(V2, t); 
    
  }

  void updateBlueButton() {
    static ulong lastTime = 0;
    static int lastValue = HIGH;
    static ulong debounceTime = 0;

    if (!IsReady(lastTime, 200)) return;
    int v = digitalRead(btnBLED);
    if (v == lastValue) return; 
    lastValue = v;
    if (v == LOW) return;

    // Chống nhấn đúp liên tục
    if (!IsReady(debounceTime, 150)) return;

    // Đảo trạng thái nút
    blueButtonON = !blueButtonON;
    Serial.print("Trạng thái LED: ");
    Serial.println(blueButtonON ? "ON" : "OFF");

    // Bật/tắt đèn LED
    digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);
    Blynk.virtualWrite(V1, blueButtonON);

    // Điều khiển TM1637
    tm1637ON = blueButtonON;
    if (!tm1637ON) display.clear(); 
}


//được gọi mỗi khi có dữ liệu mới được gửi từ ứng dụng Blynk đến thiết bị.
BLYNK_WRITE(V1) {
    bool newState = param.asInt();  // Lấy trạng thái từ Blynk
    if (newState != blueButtonON) { // Chỉ cập nhật nếu có thay đổi
        blueButtonON = newState;
        Serial.println(blueButtonON ? "Blynk -> Blue Light ON" : "Blynk -> Blue Light OFF");

        // Cập nhật trạng thái LED thực tế
        digitalWrite(pinBLED, blueButtonON ? HIGH : LOW);

        // Đồng bộ với TM1637
        tm1637ON = blueButtonON;
        if (!tm1637ON) display.clear();
    }
}


void NonBlocking_Traffic_Light_TM1637()
{
  if (!tm1637ON) return;

  bool bShowCounter = false;
  switch (currentLED)
  {
  case RED_LED: // Đèn đỏ: 5 giây
    if (IsReady(ledTimeStart, rTIME))
    {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      currentLED = GREEN_LED;
      nextTimeTotal += gTIME;
      tmCounter = (gTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
      Serial.print("2. GREEN  => YELLOW ");
      Serial.print((nextTimeTotal / 1000) % 60);
      Serial.println(" (ms)");
    }
    break;
  case GREEN_LED: // Đèn xanh: 7 giây
    if (IsReady(ledTimeStart, gTIME))
    {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      currentLED = YELLOW_LED;
      nextTimeTotal += yTIME;
      tmCounter = (yTIME / 1000) - 1;
      bShowCounter = true;
      counterTime = currentMiliseconds;
      Serial.print("3. YELLOW => RED    ");
      Serial.print((nextTimeTotal / 1000) % 60);
      Serial.println(" (ms)");
    }
    break;

  case YELLOW_LED: // Đèn vàng: 2 giây
    if (IsReady(ledTimeStart, yTIME))
    {
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      currentLED = RED_LED;
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
    Blynk.virtualWrite(V0, tmCounter);
    display.showNumberDec(tmCounter--, true, 2, 2);
  }
}

bool isDark()
{
  static ulong darkTimeStart = 0; // lưu thời gian của việc đọc cảm biến
  static uint16_t lastValue = 0;  // lưu giá trị gần nhất của cảm biến
  static bool bDark = false;      // true: value > darkThreshold

  if (!IsReady(darkTimeStart, 50))
    return bDark;                      // 50ms đọc cảm biến 1 lầnlần
    
    if (!useBlynkValue) // Nếu không dùng giá trị từ Blynk, đọc cảm biến
    value = analogRead(ldrPIN); // đọc cảm biến theo chế đố tương tựtự
    
    
    Blynk.virtualWrite(V4, value);

  if (value == lastValue)
    return bDark; // vẫn bằng giá trị củcủ

  if (value < darkThreshold)
  {
    if (!bDark)
    {
      digitalWrite(currentLED, LOW);
      Serial.print("DARK  value: ");
      Serial.println(value);
    }
    bDark = true;
  }
  else
  {
    if (bDark)
    {
      digitalWrite(currentLED, LOW);
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
  static ulong yLedStart = 0;
  static bool isON = false;

  if (!IsReady(yLedStart, 1000))
    return;
  if (!isON)
    digitalWrite(YELLOW_LED, HIGH);
  else
    digitalWrite(YELLOW_LED, LOW);
  isON = !isON;
}

//được gọi mỗi khi có dữ liệu mới được gửi từ ứng dụng Blynk đến thiết bị.
BLYNK_WRITE(V4) { //virtual_pin định nghĩa trong ứng dụng Blynk
    // Xử lý dữ liệu nhận được từ ứng dụng Blynk
    int dark = param.asInt();
    if (dark > 0) // Nếu giá trị từ Blynk hợp lệ
    {
        value = dark;
        useBlynkValue = true; // Chuyển sang sử dụng giá trị từ Blynk
        Serial.print("New dark value from Blynk: ");
        Serial.println(value);
    }
    else
    {
        useBlynkValue = false; // Quay lại dùng cảm biến
    }
}