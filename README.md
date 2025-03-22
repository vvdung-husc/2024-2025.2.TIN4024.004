

# 2024-2025.2.TIN4024.004

## Phát triển ứng dụng IoT

## WiFi Lab: CNTT-MMT/13572468

  

-  #### WOKWI - World's most advanced ESP32 simulator

	Đăng ký tài khoản tại [wokwi.com](https://wokwi.com/)
	>Dùng để thiết kế mạch IoT cho việc mô phỏng

-  #### Your code editor

	Cài đặt [Visual Studio Code](https://code.visualstudio.com/)
	>Lập trình cho mạch đã thiết kế, chạy mô phỏng và nạp vào vi mạch thực hành

-  #### Your Gateway to Embedded Software Development Excellence

	Cài đặt [PlatformIO Extension](https://platformio.org/) cho VSCode

	> Biên dịch mã nguồn c/c++ cho thiết bị vi mạch

-  #### Wokwi Embedded Simulator

	Cài đặt [Wokwi Simulator](https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode) cho VSCode
	>Chạy mô phỏng mạch kết hợp
  
-  #### Reference videos - Wokwi, PlatformIO, VS Code

	- [Mô phỏng các dự án IoT trong VS Code | Wokwi, PlatformIO, VSCode](https://www.youtube.com/watch?v=9pTZL934k2s)
	- [Bắt đầu lập trình Arduino, ESP32 với PlatformIO trên VSCode](https://www.youtube.com/watch?v=20eakkralUs)
---

#### [Đăng ký tài khoản Github, nhóm thực hành](https://docs.google.com/spreadsheets/d/15aRVi8PURtXOAVGbCgaXe8SelJ5_0oaF/edit?gid=109107554#gid=109107554)

---

#### [Tổng quan về sơ đồ chân ESP32 và Ngoại vi](https://khuenguyencreator.com/tong-quan-ve-so-do-chan-esp32-va-ngoai-vi/)

#### [Lập trình ESP32 từ A tới Z](https://khuenguyencreator.com/lap-trinh-esp32-tu-a-toi-z/)

#### [Lập trình ESP32 GPIO Digital Input và Digital Output](https://khuenguyencreator.com/lap-trinh-esp32-gpio-digital-input-va-digital-output/)

---
### - Esp8266 Project | Blynk
>Vào TEAM_00/vvdung để lấy file utils.h và utils.cpp

### Một số yêu cầu
- Sinh số ngẫu nhiên cho nhiệt độ (từ -40.0 đến 80.0), độ ẩm (từ 0.0% đến 100.0%)
- Hiển thị thông tin nhiệt độ và độ ẩm trên bảng OLED
- Hiển thị 3 đèn LED theo cơ chế đèn giao thông
- Thiết kế trên Blynk (Mobile) gồm các thông tin
	>+ Thời gian hoạt động
	>+ Nhiệt độ, độ ẩm
	>+ Một Switch để chuyển sang chế độ đèn vàng nhấp nháy  

---
### [- Esp8266 Project (***mã nguồn minh họa***)](https://wokwi.com/projects/425833805481407489)
>Vào TEAM_00/vvdung để lấy file utils.h và utils.cpp

#### [Tải và cài đặt trình điều khiển (CP210x Windows Drivers)](https://www.silabs.com/documents/public/software/CP210x_Windows_Drivers.zip)

#### Tạo dự án - ESP32_Project 
+ Điều khiển đèn nhấp nháy (GREEN PIN 15, YELLOW PIN 2, RED PIN 5)
+ Đo nhiệt độ	(PIN 16)
+ Hiển thị lên màn hình OLED (SH1106) (SDA PIN 13, SCL PIN 12)
  > Thư viện hổ trợ U8g2 by oliver

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/diagram_two.png)

#### NodeMCU v2 và v3
![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/NodeMCU-V2-vs-V3.jpg)

---
### - [Esp32 |  TELEGRAM BOT (***mã nguồn minh họa***)](https://wokwi.com/projects/425361659331202049)

#### Tham khảo

+ [ESP32 Telegram Bot](https://www.iotzone.vn/esp32/cach-dung-esp32-telegram-dieu-khien-den-led-voi-arduino-ide/)

#### Ứng dụng Telegram

>[Telegram Applications](https://telegram.org/apps)

>[Telegram APIs](https://core.telegram.org/api)

## => Yêu cầu thực hiện được đoạn mã nguồn minh họa để hiển thị trong ứng dụng Telegram

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/ESP32_Telegram_02.png)

---
### - [Esp32 | API HTTP GET (***mã nguồn minh họa***)](https://wokwi.com/projects/425209099504209921)

#### Tham khảo

+ [ESP32 HTTP Client phương thức Get](https://khuenguyencreator.com/lay-du-lieu-thoi-tiet-voi-esp32-http-client-phuong-thuc-get/)

#### Một số phần mềm

+ [CURL for Windows](https://curl.se/windows/)
+ [Postman - Kiểm thử API](https://www.postman.com/)

#### http://ip4.iothings.vn?geo=1
>Thông tin địa chỉ IPv4 đang sử dụng và vị trí địa lý (Geolocation - Latitude: Vĩ độ, Longitude: Kinh độ) 

#### http://www.google.com/maps/place/16.4591267,107.5901477
>Google Maps tại Latitude, Longitude

## => Hãy lập trình trong ESP32 sử dụng API HTTP GET để lấy được IPv4, Latitude, Longitude và đưa ra Link Google Maps (Hiển thị tại Terminal)

## => Đăng ký tài khoản https://openweathermap.org/, sử dụng API http://ip4.iothings.vn?geo=1 và API https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={API key} để thực hiện trên Blynk như hình sau
> Xử lý chuỗi và JSON trả về từ API

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/ESP32_API.png)

---
### - [Blynk | Traffic & DHT Sensor](https://wokwi.com/projects/424198235739151361)
+ ***Thư viện sử dụng***
	> **# DHT sensor library** by Avishay - Arduino library for DHT11, DHT22, etc Temp & Humidity Sensors
	
![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/traffic_blynk_1.png)
---
### - [Esp32 | Blynk & Wokwi](https://wokwi.com/projects/423790624312911873)

#### Tham khảo

+ [Sử dụng Blynk IOT với ESP32](https://dienthongminhesmart.com/lap-trinh-esp32/blynk-iot-va-esp32/)
+ ***Thư viện sử dụng***
	> **TM1637** by Avishay - Arduino library for TM1637
 
	> **Blynk** by Volodymyr Shymanskyy

+ [Sử dụng Blynk IOT trên ESP32 (nếu không xem được từ youtube)](https://it.iothings.vn/downloads/mp4/Blynk_IOT_ESP32_WEB.mp4)

+ [https://it.iothings.vn/downloads/mp4](https://it.iothings.vn/downloads/mp4/)

#### Nên sử dụng AI để tìm hiểu 


[https://chatgpt.com/](https://chatgpt.com/)

[https://chat.deepseek.com/](https://chat.deepseek.com/)

[https://grok.com/](https://grok.com/)

---

### - [Blink an LED on ESP32](https://wokwi.com/projects/305566932847821378)

#### Tham khảo

  

+ [Tạo một dự án lập trình ESP32 với PlatformIO](https://khuenguyencreator.com/huong-dan-cai-dat-platform-io-lap-trinh-esp32/#Huong_dan_su_dung_Platform_IO_lap_trinh_ESP32)

  

+ [Wokwi for VS Code](https://docs.wokwi.com/vscode/getting-started)

  
+ [Cài đặt thư viện cho PlatformIO](https://khuenguyencreator.com/huong-dan-cai-dat-platform-io-lap-trinh-esp32/#Cai_dat_thu_vien_cho_Platformio)
-------------------------------

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/diagram_one.png)

-------------------------------

![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/diagram_two.png)

-------------------------------
