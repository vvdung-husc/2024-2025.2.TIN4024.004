

  
# Dự án - Đèn giao thông
- ## Yêu cầu thực hiện theo nhóm (2 đến 5 thành viên mỗi nhóm)
	- Tạo dự án có tên **ESP32_Traffic_Light** tại thư mục của nhóm mình
	
		> ví dụ: ..\TEAM_X\ESP32_Traffic_Light
	- Thời gian đến hết ngày **23/02/2025**
	
		> Sau thời gian này các commit thay đổi nội dung của dự án sẽ không được tính.
---
![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/esp32_trafic.png)

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

	6\.  **ESP32 Board - NodeMCU-32s**

	![](https://raw.githubusercontent.com/vvdung/storage/refs/heads/main/IOT/esp32_board.png)
		
---
