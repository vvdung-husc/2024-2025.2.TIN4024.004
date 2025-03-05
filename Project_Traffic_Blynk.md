
  
# [Project - Traffic - Blynk](https://wokwi.com/projects/424566065569042433)
- ## Yêu cầu thực hiện theo nhóm (2 đến 5 thành viên mỗi nhóm)
	- Tạo dự án có tên **ESP32_Traffic_Blynk** tại thư mục của nhóm mình
	
		> ví dụ: ..\TEAM_X\ESP32_Traffic_Blynk
	- Thời gian đến hết ngày **16/03/2025**
	
		> Sau thời gian này các commit thay đổi nội dung của dự án sẽ không được tính.
---
![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/ESP32_Traffic_Blynk.png)

## Thông tin BLYNK trong mã nguồn
>- Mỗi thành viên của nhóm điền 3 mã BLYNK tương ứng của mình trong mã nguồn
>- Mỗi thành viên đều phải xây dựng Web và Mobile trong BLYNK tương ứng với 3 mã BLINK ở trên
```cpp
/* Fill in information from Blynk Device Info here */

//NGUYEN VAN A
// #define BLYNK_TEMPLATE_ID "TMPL64YL86543"
// #define BLYNK_TEMPLATE_NAME "ESP32 TRAFFIC"
// #define BLYNK_AUTH_TOKEN "S9-UuqRP6ItPoUGPZYbtSWknol03FF-0"

//TRAN THI B
// #define BLYNK_TEMPLATE_ID "TMPL64YL85678"
// #define BLYNK_TEMPLATE_NAME "ESP32 PROJECT 2"
// #define BLYNK_AUTH_TOKEN "S9-UuqRP6ItPoUGPZYbtSWknol03FF-1"

//VO VIET DUNG
#define  BLYNK_TEMPLATE_ID  "TMPL64YL8fJrk"
#define  BLYNK_TEMPLATE_NAME  "ESP32 LED TM1637"
#define  BLYNK_AUTH_TOKEN  "KD23vHiUV3LzwzOito2j40o12cKvzzMi"
```

---

- ## Các thiết bị trong dự án
 
	1\.  **LED - Đèn LED**
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/LED.png)
		
	[Tham khảo - https://docs.wokwi.com/parts/wokwi-led](https://docs.wokwi.com/parts/wokwi-led)
		
		
	2\.  **Resistor - Điện trở**
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/Resistor.png)
	
	[Tham khảo - https://docs.wokwi.com/parts/wokwi-resistor](https://docs.wokwi.com/parts/wokwi-resistor)


	3\.  **PushButton - Nút nhấn**

	> Bật tắt thiển thị thời gian đếm ngược của đèn
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/PushButton.png)
	
	[Tham khảo - https://docs.wokwi.com/parts/wokwi-pushbutton](https://docs.wokwi.com/parts/wokwi-pushbutton)
	
	
	4\.  **TM1637 - Bảng hiển thị đèn LED**

	> Bảng hiển thị thời gian
		
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/TM1637.png)

	[Tham khảo - https://docs.wokwi.com/parts/wokwi-tm1637-7segment](https://docs.wokwi.com/parts/wokwi-tm1637-7segment)

	***Thư viện sử dụng***
	> **TM1637** by Avishay - Arduino library for TM1637

	```cpp
	#include <TM1637Display.h>

	#define CLK 2
	#define DIO 3
			  
	TM1637Display display(CLK, DIO);	
	```

	5\.  **LDR - Cảm biến quang điện trở**

	> Dùng để bật/tắt đèn đường, đèn trong nhà dựa trên cường độ ánh sáng môi trường.
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/LDR.png)

	[Tham khảo - https://docs.wokwi.com/parts/wokwi-photoresistor-sensor](https://docs.wokwi.com/parts/wokwi-photoresistor-sensor)

	
	6\.  **DHT- Cảm biến nhiệt độ và độ ẩm**
		
	> Dùng để đọc nhiệt độ và độ ẩm hiện tại.
  		
  	***Thư viện sử dụng***
	> **DHT sensor library** by Adafruit

	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/DHT.png)


	[Tham khảo - https://docs.wokwi.com/parts/wokwi-dht22](https://docs.wokwi.com/parts/wokwi-dht22)

	
	7\.  **ESP32 Board - NodeMCU-32s**
				![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/esp32_board.png)


	>### Sơ đồ chân - board-esp32-devkit-c-v4
	
	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/esp32-devkitC-v4-pinout.png)
---
