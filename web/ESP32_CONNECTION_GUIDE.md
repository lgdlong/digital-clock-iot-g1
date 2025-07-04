# 🕒 Hướng dẫn kết nối ESP32 với Web Application

## 📋 Tổng quan

Ứng dụng web này có thể kết nối trực tiếp với ESP32 để hiển thị thời gian real-time thông qua MQTT broker.

## 🔧 Cấu hình ESP32

### 1. MQTT Settings trong ESP32
```cpp
// Cấu hình trong main.cpp
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;  // TCP port cho ESP32
const char* timePublishTopic = "clock/time";
const char* resetSubscribeTopic = "clock/reset";
```

### 2. WiFi Connection
- ESP32 phải kết nối WiFi để giao tiếp với MQTT broker
- Sử dụng WiFiManager để cấu hình WiFi
- Hotspot mặc định: `IOT_NHOM1` / `12345678`

## 🌐 Web Application APIs

### ESP32 Time API
```
GET /api/esp32-time
```
- Lấy thời gian hiện tại từ ESP32
- Timeout: 10 giây
- Response: JSON với time, timestamp, source

### ESP32 Status API
```
GET /api/esp32-status
```
- Kiểm tra trạng thái kết nối ESP32
- Test MQTT broker connection
- Response: JSON với connection status, latency

## 🔄 Cách thức hoạt động

### 1. MQTT Communication Flow
```
Web App ---> MQTT Broker ---> ESP32
         (WebSocket)      (TCP)
         port 8000        port 1883
```

### 2. Time Synchronization
1. Web app gửi `"reset"` tới topic `"clock/reset"`
2. ESP32 nhận lệnh và publish thời gian tới `"clock/time"`
3. Web app nhận thời gian và hiển thị

### 3. Topics sử dụng
- `clock/time`: ESP32 publish thời gian (format: HH:MM:SS)
- `clock/reset`: Web app gửi lệnh để trigger time update

## 🧪 Test kết nối

### 1. Kiểm tra ESP32
```bash
# Monitor Serial của ESP32
# Phải thấy logs:
✓ WiFi connected: 192.168.x.x
✓ MQTT connected
📤 Time published: 14:30:25
```

### 2. Test từ Web App

#### A. Sử dụng giao diện web
1. Truy cập: https://digital-clock-iot-g1.vercel.app/
2. Kiểm tra component "Thời gian ESP32"
3. Nhấn nút "🔄" để đồng bộ
4. Xem trạng thái kết nối

#### B. Test API trực tiếp
```bash
# Test status
curl https://digital-clock-iot-g1.vercel.app/api/esp32-status

# Test get time
curl https://digital-clock-iot-g1.vercel.app/api/esp32-time
```

### 3. Debug Information
Trong development mode, web hiển thị debug info:
- MQTT Connection Status
- Current Time
- Last Update Time
- Connection Errors

## 🚨 Troubleshooting

### Vấn đề thường gặp

#### 1. ESP32 không kết nối MQTT
```
Nguyên nhân: WiFi không ổn định
Giải pháp: Kiểm tra WiFi, restart ESP32
```

#### 2. Web không nhận được time
```
Nguyên nhân: ESP32 không online hoặc MQTT broker issue
Giải pháp: Check ESP32 serial logs, test API status
```

#### 3. Timeout errors
```
Nguyên nhân: ESP32 không phản hồi
Giải pháp: Gửi lệnh reset từ Serial hoặc restart ESP32
```

### Commands để debug

#### ESP32 Serial Commands
```
help     - Show available commands
status   - Show device status  
reset    - Factory reset
```

#### Test MQTT bằng tools khác
```bash
# Subscribe to time topic
mosquitto_sub -h broker.hivemq.com -t "clock/time"

# Send reset command
mosquitto_pub -h broker.hivemq.com -t "clock/reset" -m "reset"
```

## 📊 Monitoring

### Web App Features
- ✅ Real-time time display từ ESP32
- ✅ Connection status indicators
- ✅ Auto-reconnect mechanism
- ✅ Error notifications
- ✅ Manual refresh capability
- ✅ Latency monitoring

### ESP32 Features  
- ✅ MQTT auto-reconnect
- ✅ WiFi status monitoring
- ✅ Serial command interface
- ✅ Time publishing on demand

## 🔐 Security Notes

- Sử dụng public MQTT broker (broker.hivemq.com)
- Không authentication required
- Topics công khai - chỉ dùng cho testing
- Production nên dùng private broker với authentication

## 📱 Mobile Support

Web app responsive và hoạt động trên mobile:
- Touch-friendly buttons
- Auto-refresh time display
- Mobile-optimized MQTT WebSocket connection

---

## 🎯 Quick Start

1. **Chuẩn bị ESP32:**
   - Upload code `src/main.cpp`
   - Kết nối WiFi
   - Kiểm tra Serial logs

2. **Test Web:**
   - Mở https://digital-clock-iot-g1.vercel.app/
   - Kiểm tra "Thời gian ESP32" component
   - Nhấn refresh để test

3. **Verify:**
   - Thời gian hiển thị từ ESP32
   - Status "Kết nối" hiển thị
   - No error messages

**Thành công khi:** Web hiển thị thời gian real-time từ ESP32 với status "Kết nối" 🎉 