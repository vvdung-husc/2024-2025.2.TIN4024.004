#include <Arduino.h>
#include <TM1637Display.h>

// Định nghĩa các chân kết nối
#define LED_GREEN 25      // Chân kết nối đèn xanh
#define LED_YELLOW 26     // Chân kết nối đèn vàng
#define LED_RED 27        // Chân kết nối đèn đỏ
#define LED_BLUE 21       // Chân kết nối đèn xanh dương (LED phụ)
#define BTN_DISPLAY 23    // Chân kết nối nút nhấn
#define LIGHT_SENSOR 13   // Chân kết nối cảm biến ánh sáng
#define CLK 18            // Chân CLK của màn hình TM1637
#define DIO 19            // Chân DIO của màn hình TM1637

// Ngưỡng ánh sáng để chuyển đổi giữa chế độ ngày và đêm
#define LIGHT_THRESHOLD 1000 

// Khởi tạo đối tượng màn hình TM1637
TM1637Display display(CLK, DIO);

// Định nghĩa các trạng thái của đèn giao thông
enum TrafficState { GREEN, YELLOW, RED };
TrafficState currentState = GREEN; // Trạng thái hiện tại, mặc định là đèn xanh

// Thời gian đếm ngược cho từng trạng thái đèn
const int greenTime = 5;  // Thời gian đèn xanh
const int yellowTime = 3; // Thời gian đèn vàng
const int redTime = 5;    // Thời gian đèn đỏ

int countdown;            // Biến đếm ngược thời gian
bool displayEnabled = true; // Biến kiểm soát việc hiển thị màn hình
bool lastButtonState = HIGH; // Trạng thái trước đó của nút nhấn
bool isNightMode = false; // Biến kiểm soát chế độ đêm (true: đêm, false: ngày)

// Hàm điều khiển đèn giao thông dựa trên trạng thái hiện tại
void setLights(TrafficState state) {
    if (isNightMode) {
        // Chế độ đêm: chỉ đèn vàng sáng
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        digitalWrite(LED_RED, LOW);
    } else {
        // Chế độ ngày: đèn hoạt động theo trạng thái
        digitalWrite(LED_GREEN, state == GREEN);
        digitalWrite(LED_YELLOW, state == YELLOW);
        digitalWrite(LED_RED, state == RED);
    }
}

// Hàm hiển thị số lên màn hình TM1637
void displayNumber(int num) {
    if (isNightMode) {
        // Chế độ đêm: hiển thị "0.0"
        display.showNumberDecEx(0, 0b01000000, true, 2, 2); // "0.0" với dấu chấm ở giữa
    } else {
        // Chế độ ngày: hiển thị số đếm ngược hoặc tắt màn hình
        if (displayEnabled) {
            display.showNumberDecEx(num, 0, true, 2, 2); // Hiển thị số
        } else {
            display.clear(); // Tắt màn hình
        }
    }
    digitalWrite(LED_BLUE, displayEnabled); // Điều khiển LED xanh dương theo trạng thái hiển thị
}

// Hàm kiểm tra trạng thái nút nhấn
void checkButton() {
    bool currentButtonState = digitalRead(BTN_DISPLAY); // Đọc trạng thái nút nhấn
    if (lastButtonState == HIGH && currentButtonState == LOW) {
        // Nếu nút được nhấn (chuyển từ HIGH sang LOW)
        displayEnabled = !displayEnabled; // Đảo trạng thái hiển thị màn hình
        displayNumber(countdown); // Cập nhật màn hình
    }
    lastButtonState = currentButtonState; // Lưu trạng thái hiện tại của nút
}

// Hàm kiểm tra cảm biến ánh sáng
void checkLightSensor() {
    int lightValue = analogRead(LIGHT_SENSOR); // Đọc giá trị từ cảm biến ánh sáng
    bool previousNightMode = isNightMode; // Lưu trạng thái chế độ đêm trước đó
    isNightMode = (lightValue < LIGHT_THRESHOLD); // Cập nhật chế độ đêm (true nếu Lux < 1000)
    
    if (previousNightMode != isNightMode) {
        // Nếu chế độ đêm thay đổi, cập nhật đèn và màn hình
        setLights(currentState);
        displayNumber(countdown);
    }
}

// Hàm thiết lập ban đầu
void setup() {
    // Thiết lập chế độ cho các chân
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(BTN_DISPLAY, INPUT_PULLUP); // Nút nhấn sử dụng điện trở kéo lên
    pinMode(LIGHT_SENSOR, INPUT); // Cảm biến ánh sáng
    
    display.setBrightness(7); // Đặt độ sáng màn hình
    display.clear(); // Xóa màn hình ban đầu
    
    setLights(GREEN); // Khởi tạo đèn xanh
    countdown = greenTime; // Đặt thời gian đếm ngược ban đầu
    displayNumber(countdown); // Hiển thị thời gian ban đầu
}

// Hàm lặp chính
void loop() {
    static unsigned long lastUpdate = 0; // Biến lưu thời gian cập nhật cuối cùng
    unsigned long currentMillis = millis(); // Lấy thời gian hiện tại

    checkButton(); // Kiểm tra nút nhấn
    checkLightSensor(); // Kiểm tra cảm biến ánh sáng

    // Cập nhật mỗi giây
    if (currentMillis - lastUpdate >= 1000) {
        lastUpdate = currentMillis; // Cập nhật thời gian
        
        if (isNightMode) {
            // Chế độ đêm: đặt thời gian đếm ngược là thời gian đèn vàng
            countdown = yellowTime;
            displayNumber(0); // Hiển thị "0.0"
        } else {
            // Chế độ ngày: xử lý đếm ngược và chuyển đổi trạng thái đèn
            if (countdown == 0) {
                // Khi đếm ngược kết thúc, chuyển sang trạng thái tiếp theo
                switch (currentState) {
                    case GREEN:
                        currentState = YELLOW;
                        countdown = yellowTime;
                        break;
                    case YELLOW:
                        currentState = RED;
                        countdown = redTime;
                        break;
                    case RED:
                        currentState = GREEN;
                        countdown = greenTime;
                        break;
                }
            } else {
                countdown--; // Giảm thời gian đếm ngược
            }
        }
        
        displayNumber(countdown); // Cập nhật màn hình
        
        if (!isNightMode && (countdown == greenTime || countdown == yellowTime || countdown == redTime)) {
            // Nếu không phải chế độ đêm và thời gian đếm ngược bằng thời gian của một trạng thái
            setLights(currentState); // Cập nhật đèn
        }
    }
}