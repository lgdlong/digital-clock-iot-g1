
# Key Features – Digital Clock IoT

1. **Thời gian thực** (Đã có Wokwi)
  
*  - [x] Hiển thị giờ/phút/giây. (Wokwi)
*  - [x] Lấy thời gian Internet và backup bằng module RTC **DS1307** nếu mất điện. (Wokwi)
2. **Đo & hiển thị nhiệt độ phòng** (Đang xử lý)
  
*  - [ ] Đọc dữ liệu từ cảm biến **LM35**.
*  - [ ] Hiển thị nhiệt độ lên màn hình **LCD 16x2 I2C**.
3. **Lấy & hiển thị thông tin thời tiết ngoài trời** (Đang xử lý):
  
*  - [ ] Kết nối Internet để lấy dữ liệu thời tiết (API OpenWeatherMap hoặc tương đương).
*  - [ ] Hiển thị thông tin: nhiệt độ, độ ẩm, trạng thái thời tiết.
4. **Báo thức thông minh** (Check sau)
  
*  - [ ] Đặt và cấu hình thời gian báo thức.
*  - [ ] Kích hoạt buzzer khi đến giờ báo thức.
*  - [ ] Có nút nhấn vật lý để tắt/chỉnh báo thức.
5. **Giao diện web cấu hình** (Check sau)

* - [ ] ESP32 chạy local web server.
*  - [ ] Cấu hình báo thức, thời gian, tên thiết bị, vị trí dự báo thời tiết qua giao diện web.
*  - [ ] Lưu cấu hình vào EEPROM hoặc SPIFFS.