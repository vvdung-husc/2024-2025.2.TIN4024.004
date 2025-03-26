
# Dự án - Đèn giao thông
- ## Yêu cầu thực hiện theo nhóm (2 đến 5 thành viên mỗi nhóm)
	- Tạo dự án có tên **ESP8266_Blynk_Telegram** tại thư mục của nhóm mình
	
		> ví dụ: ..\TEAM_X\ESP8266_Blynk_Telegram
	- Thời gian đến hết ngày **06/04/2025**
	
		> Sau thời gian này các commit thay đổi nội dung của dự án sẽ không được tính.
		
### Một số yêu cầu
- Sinh số ngẫu nhiên cho nhiệt độ (từ -40.0 đến 80.0), độ ẩm (từ 0.0% đến 100.0%)
- Hiển thị thông tin nhiệt độ và độ ẩm trên bảng OLED
- Hiển thị 3 đèn LED theo cơ chế đèn giao thông
- Thiết kế trên Blynk (Mobile) gồm các thông tin
	>+ Thời gian hoạt động
	>+ Nhiệt độ, độ ẩm
	>+ Một Switch để chuyển sang chế độ đèn vàng nhấp nháy  
- Ứng dụng Telegram
	>+ Gửi cảnh báo đến Telegram mỗi 5 phút một lần nếu gặp trường hợp nhiệt độ hoặc độ ẩm có thể gây nguy hại cho sức khỏe (___tham khảo bảng ở dưới___).
	>+ Điều khiển đèn giao thông bằng cách gửi tin nhắn từ Telegram để tắt toàn bộ đèn khi gửi ___/traffic_off___ và hoạt động trở lại khi gửi ___/traffic_on___
___
### 🔥 Ngưỡng nhiệt độ ảnh hưởng đến sức khỏe
| Nhiệt độ (°C) | Ảnh hưởng đến sức khỏe |
|--------------|---------------------|
| **< 10°C**  | Nguy cơ hạ thân nhiệt, tê cóng, giảm miễn dịch. |
| **10 - 15°C** | Cảm giác lạnh, tăng nguy cơ mắc bệnh đường hô hấp. |
| **25 - 30°C** | Nhiệt độ lý tưởng, ít ảnh hưởng đến sức khỏe. |
| **30 - 35°C** | Cơ thể bắt đầu có dấu hiệu mất nước, mệt mỏi. |
| **> 35°C**  | Nguy cơ sốc nhiệt, chuột rút, say nắng. |
| **> 40°C**  | Cực kỳ nguy hiểm, có thể gây suy nội tạng, đột quỵ nhiệt. |

---

### 💦 Ngưỡng độ ẩm ảnh hưởng đến sức khỏe
| Độ ẩm (%)   | Ảnh hưởng đến sức khỏe |
|------------|---------------------|
| **< 30%**  | Da khô, kích ứng mắt, tăng nguy cơ mắc bệnh về hô hấp (viêm họng, khô mũi). |
| **40 - 60%** | Mức lý tưởng, ít ảnh hưởng đến sức khỏe. |
| **> 70%**  | Tăng nguy cơ nấm mốc, vi khuẩn phát triển, gây bệnh về đường hô hấp. |
| **> 80%**  | Cảm giác oi bức, khó thở, cơ thể khó thoát mồ hôi, tăng nguy cơ sốc nhiệt. |
