# Smart Clock v5.0 - Tài liệu kỹ thuật

## 📚 Mục lục

### 🎯 [1. Tổng quan cấp cao](#high-level-overview)
- [Trách nhiệm chính](#main-responsibilities)
- [Tính năng chính](#key-features)
- [Khả năng hệ thống](#system-capabilities)

### 🏗️ [2. Kiến trúc & Luồng chương trình](#architecture--program-flow)
- [Luồng khởi tạo](#initialization-flow-setup---lines-1450-1523)
- [Luồng lặp chính](#main-loop-flow-loop---lines-1525-1591)
- [Máy trạng thái Overview](#state-machine-lines-1578-1590)
- [Tích hợp hệ thống](#system-integration)

### 🔧 [3. Global Variables & Cấu trúc dữ liệu](#global-variables--data-structures)
- [Định nghĩa chân phần cứng](#hardware-pin-definitions-lines-57-61)
- [Đối tượng phần cứng](#hardware-objects-lines-63-67)
- [Biến trạng thái hệ thống](#system-state-variables-lines-69-80)
- [Cấu trúc cấu hình](#configuration-structures)
  - [Cấu hình thiết bị](#device-configuration-lines-82-88)
  - [Trạng thái phần cứng](#hardware-status-lines-91-101)
  - [Hệ thống báo thức](#alarm-system-lines-118-135)
  - [Bộ đếm ngược](#countdown-timer-lines-137-147)
  - [Cấu hình thời tiết](#weather-configuration-lines-149-156)
  - [Dữ liệu thời tiết](#weather-data-lines-158-167)
- [Biến hiển thị & cảm biến](#temperature--display-variables-lines-114-116-184-188)
- [Hằng số bố trí bộ nhớ](#memory-layout-constants-lines-190-194)

### 📖 [4. Tham chiếu hàm](#function-reference)
- [Hàm tiện ích](#utility-functions)
  - [checkFirstBoot()](#checkfirstboot-lines-200-212)
  - [clearAllData()](#clearalldata-lines-214-234)
  - [convertAdcToTemperature()](#convertadctotemperatureint-adcvalue-lines-236-255)
  - [factoryReset()](#factoryreset-lines-257-267)
- [Hàm phần cứng](#hardware-functions)
  - [initializeHardware()](#initializehardware-lines-272-340)
  - [readTemperature()](#readtemperature-lines-342-351)
- [Hàm hiển thị](#display-functions)
  - [updateLCDContent()](#updatelcdcontentstring-line1-string-line2-lines-356-370)
  - [displayClock()](#displayclock-lines-372-427)
  - [displayCountdown()](#displaycountdown-lines-429-455)
  - [displayMenu()](#displaymenu-lines-457-467)
- [Hệ thống báo thức & bộ đếm](#alarm--timer-system)
  - [triggerAlarm()](#triggeralarmint-index-lines-472-479)
  - [stopAlarm()](#stopalarm-lines-481-491)
  - [updateAlarmDisplay()](#updatealarmdisplay-lines-493-524)
- [Hệ thống xử lý nút nhấn](#button-handling-system)
  - [handleButton()](#handlebutton-lines-529-593)
  - [buttonInterrupt()](#buttoninterrupt-lines-600-619)
- [Giao diện web](#web-interface)
  - [generateWebInterface()](#generatewebinterface-lines-624-995)
- [Các endpoint máy chủ web](#web-server-endpoints)
  - [setupWebServer()](#setupwebserver-lines-1000-1213)
  - [API Endpoints List](#api-endpoints-reference)
- [Quản lý cấu hình](#configuration-management)
  - [loadConfiguration()](#loadconfiguration-lines-1218-1237)
  - [saveConfiguration()](#saveconfiguration-lines-1239-1244)
  - [saveAlarms()](#savealarms-lines-1246-1253)
  - [loadWeatherConfig()](#loadweatherconfig-lines-1255-1270)
  - [saveWeatherConfig()](#saveweatherconfig-lines-1272-1286)
- [Hàm mạng](#network-functions)
  - [setupWiFi()](#setupwifi-lines-1291-1311)
- [Hàm hệ thống chính](#main-system-functions)
  - [setup()](#setup-lines-1316-1388)
  - [loop()](#loop-lines-1390-1456)
- [Hàm thời tiết](#weather-functions)
  - [fetchWeatherData()](#fetchweatherdata-lines-1461-1555)
  - [checkAlarms()](#checkalarms-lines-1557-1574)

### ⚙️ [5. Cấu hình phần cứng](#hardware-configuration)
- [Phân công chân](#pin-assignments)
- [Linh kiện bên ngoài](#external-components)
- [Yêu cầu phần cứng](#hardware-requirements)
- [Sơ đồ kết nối](#connection-diagrams)

### 🌐 [6. Giao diện web](#web-interface)
- [Tổng quan tính năng](#features)
- [Thành phần giao diện](#ui-components)
- [Configuration Sections](#configuration-sections)
- [Cập nhật thời gian thực](#real-time-updates)
- [Hàm JavaScript](#javascript-functions)

### 🔄 [7. Máy trạng thái](#state-machine)
- [Định nghĩa trạng thái](#state-definitions)
- [Chuyển đổi trạng thái](#states-and-transitions)
- [Xử lý trạng thái](#state-handlers-lines-1578-1590)
- [Sơ đồ luồng trạng thái](#state-flow-diagram)

### ⚠️ [8. Các phần phức tạp](#complextricky-sections)
- [Button Interrupt System](#1-button-interrupt-system-lines-529-619)
- [Weather API with Error Recovery](#2-weather-api-with-error-recovery-lines-1461-1555)
- [LCD Display Mode Switching](#3-lcd-display-mode-switching-lines-372-427)
- [Timer with 5-Second Alarm](#4-timer-with-5-second-alarm-lines-1542-1568)
- [EEPROM vs Preferences Storage](#5-eeprom-vs-preferences-storage-lines-1218-1286)

### 💾 [9. Bố trí bộ nhớ EEPROM](#eeprom-memory-layout)
- [Bản đồ bộ nhớ](#memory-map)
- [Chi tiết lưu trữ](#storage-details)
- [Cấu trúc dữ liệu](#data-structures-in-memory)
- [Sao lưu & phục hồi](#backup--recovery)

### 🌍 [10. Tích hợp mạng & API](#network--api-integration)
- [Quản lý WiFi](#wifi-management)
- [OpenWeatherMap API](#openweathermap-api)
- [Web Server Configuration](#web-server)
- [Các cân nhắc bảo mật](#security-considerations)

---

## Tổng quan cấp cao

The `main1.cpp` file (1698 lines) implements a **comprehensive IoT Smart Clock system** for ESP32 microcontroller. This is a standalone device that combines multiple functionalities:

- **Digital Clock** with RTC (Real-Time Clock) support
- **Multi-alarm system** (up to 5 alarms with weekly scheduling)
- **Countdown timer** with 5-second alert notification
- **Weather information** via OpenWeatherMap API
- **Temperature monitoring** using LM35 sensor
- **Web-based configuration interface** (responsive HTML/CSS/JavaScript)
- **WiFi connectivity** with auto-configuration portal
- **LCD display** (16x2) with automatic mode switching
- **Hardware status monitoring** and error reporting

### Trách nhiệm chính:
1. **Real-time clock display** and time management
2. **Alarm scheduling and triggering** with audio/visual alerts
3. **Weather data fetching** and display
4. **User interface** via web browser and physical button
5. **Configuration persistence** using EEPROM and Preferences
6. **Network management** including WiFi setup and hotspot mode

### Tính năng chính:
- **Interrupt-driven button handling** for immediate alarm shutoff
- **Dual-mode WiFi** (Station + Access Point simultaneously)
- **Automatic LCD mode switching** between clock and weather display
- **Comprehensive error handling** with recovery mechanisms
- **Real-time web interface** with 2-second update intervals
- **Multi-language support** (Vietnamese with emoji icons)
- **Factory reset capability** via long button press or web interface

### Khả năng hệ thống:
- **Maximum 5 simultaneous alarms** with weekly scheduling
- **Weather updates** every 2 minutes (configurable 1-60 minutes)
- **Temperature monitoring** with min/max tracking
- **Countdown timers** up to 999 minutes
- **Web interface** accessible via any browser on the network
- **Non-volatile storage** for all configurations and alarms

---

## Kiến trúc & Luồng chương trình

### Luồng khởi tạo (setup() - Lines 1450-1523)
```
Serial Setup → Hardware Init → First Boot Check → Load Configs → 
WiFi Setup → Web Server → Weather Data → Máy trạng thái Start
```

**Detailed Sequence:**
1. **Serial Communication** (115200 baud) for debugging
2. **GPIO Configuration** (LED, Buzzer, Button with interrupt)
3. **Hardware Initialization** and validation testing
4. **First Boot Detection** and data clearing if needed
5. **Configuration Loading** from EEPROM and Preferences
6. **WiFi Setup** with auto-configuration portal
7. **Web Server Startup** with all endpoints
8. **Initial Weather Fetch** (if enabled and connected)
9. **Máy trạng thái Initialization** to STATE_NORMAL

### Luồng lặp chính (loop() - Lines 1525-1591)
```
Web Server Handle → Temperature Reading → Timer Alarm Check → 
Button Handling → Hardware Status → Weather Update → Máy trạng thái
```

**Loop Cycle (100ms):**
1. **Web Server Processing** - Handle HTTP requests
2. **Temperature Reading** - Every 5 seconds from LM35 sensor
3. **Timer Alarm Handling** - 5-second alert with fast blinking
4. **Button Processing** - Every 50ms with debouncing
5. **Trạng thái phần cứng Updates** - WiFi connection monitoring
6. **Dữ liệu thời tiết Updates** - Periodic API calls or simulation
7. **Máy trạng thái Execution** - Display and functionality control

### Máy trạng thái (Lines 1578-1590)
The system operates on a finite state machine with these states:
- `STATE_BOOT` - Initial startup
- `STATE_NORMAL` - Normal clock operation
- `STATE_ALARM` - Active alarm (audio/visual)
- `STATE_COUNTDOWN` - Timer countdown display
- `STATE_MENU` - Configuration menu
- `STATE_INFO` - Information display
- `STATE_ERROR` - Error state

### Tích hợp hệ thống
The system integrates multiple subsystems:
- **Hardware Layer** - GPIO, I2C, ADC, Interrupts
- **Network Layer** - WiFi, HTTP Server, API Client
- **Storage Layer** - EEPROM, NVS Preferences
- **Application Layer** - Clock, Alarms, Weather, Web UI
- **User Interface** - LCD Display, Web Browser, Physical Button

---

## Global Variables & Cấu trúc dữ liệu

### Định nghĩa chân phần cứng (Lines 57-61)
```cpp
#define LED_PIN 12        // Status LED
#define BUZZER_PIN 25     // Alarm buzzer
#define BUTTON_PIN 26     // User input button
#define NTC_PIN 34        // Temperature sensor (LM35)
```

### Đối tượng phần cứng (Lines 63-67)
```cpp
LiquidCrystal_I2C LCD(0x27, 16, 2);  // I2C LCD display
RTC_DS1307 rtc;                      // Real-time clock module
WebServer server(80);                // HTTP web server
WiFiManager wifiManager;             // WiFi configuration manager
Preferences preferences;             // ESP32 NVS storage
```

### Biến trạng thái hệ thống (Lines 69-80)
```cpp
SystemState currentState = STATE_BOOT;  // Current system state
unsigned long stateStartTime = 0;       // State transition timestamp
```

### Cấu trúc cấu hình

#### Cấu hình thiết bị (Lines 82-88)
```cpp
struct DeviceConfig {
  char deviceName[64] = "Smart Clock v5.0";
  char hotspotSSID[32] = "IOT_NHOM1";
  char hotspotPassword[32] = "12345678";
  bool configValid = false;
} config;
```

#### Trạng thái phần cứng (Lines 91-101)
```cpp
struct HardwareStatus {
  bool lcdOK = false;     // LCD display status
  bool rtcOK = false;     // RTC module status
  bool wifiOK = false;    // WiFi connection status
  bool tempOK = false;    // Temperature sensor status
  bool buzzerOK = false;  // Buzzer functionality
  bool ledOK = false;     // LED functionality
  String lastError = ""; // Last error message
} hw;
```

#### Hệ thống báo thức (Lines 118-135)
```cpp
struct Alarm {
  int hour = -1;                    // Alarm hour (0-23)
  int minute = -1;                  // Alarm minute (0-59)
  bool enabled = false;             // Alarm active status
  bool daysOfWeek[7] = {false};     // Weekly schedule
  char label[32] = "";              // Alarm description
};

#define MAX_ALARMS 5
Alarm alarms[MAX_ALARMS];          // Array of alarm objects
int alarmCount = 0;                // Current number of alarms
bool alarmActive = false;          // Currently alarming flag
int activeAlarmIndex = -1;         // Index of active alarm
```

#### Bộ đếm ngược (Lines 137-147)
```cpp
struct CountdownTimer {
  unsigned long duration = 0;        // Timer duration in seconds
  unsigned long startTime = 0;       // Timer start timestamp
  bool active = false;               // Timer running flag
  bool finished = false;             // Timer completion flag
  char label[32] = "Timer";          // Timer description
  bool alarmTriggered = false;       // 5-second alarm state
  unsigned long alarmStartTime = 0;  // Alarm start timestamp
} timer;
```

#### Cấu hình thời tiết (Lines 149-156)
```cpp
struct WeatherConfig {
  char apiKey[64] = "";              // OpenWeatherMap API key
  char cityName[32] = "Thu Duc";     // Target city name
  char countryCode[8] = "VN";        // Country code
  bool enabled = false;              // Weather fetching enabled
  int updateInterval = 120;          // Update interval in seconds
} weatherConfig;
```

#### Dữ liệu thời tiết (Lines 158-167)
```cpp
struct WeatherData {
  float temperature = 0.0;           // Current temperature
  int humidity = 0;                  // Humidity percentage
  String description = "N/A";       // Weather description
  String city = "Thu Duc";          // City name
  bool dataValid = false;            // Data validity flag
  unsigned long lastUpdate = 0;     // Last update timestamp
  int errorCount = 0;                // API error counter
} weather;
```

### Temperature & Display Variables (Lines 114-116, 184-188)
```cpp
float currentTemp = 25.0;           // Current temperature reading
float minTemp = 999.0;              // Minimum recorded temperature
float maxTemp = -999.0;             // Maximum recorded temperature

String currentLCDLine1 = "";        // LCD first line content
String currentLCDLine2 = "";        // LCD second line content
int lcdDisplayMode = 0;             // Display mode (0: Clock, 1: Weather)
unsigned long lastLCDModeChange = 0; // Mode switch timestamp
```

### Hằng số bố trí bộ nhớ (Lines 190-194)
```cpp
#define EEPROM_SIZE 1024
#define CONFIG_ADDR 0      // Device config storage address
#define ALARM_ADDR 400     // Alarms storage address  
#define TIMER_ADDR 800     // Timer storage address
```

---

## Tham chiếu hàm

### Hàm tiện ích

#### `checkFirstBoot()` (Lines 200-212)
- **Purpose**: Detects first-time device startup
- **Returns**: `bool` - true if first boot, false otherwise
- **Side Effects**: Sets firstBoot flag to false in preferences
- **Usage**: Used in setup() to initialize default configurations

#### `clearAllData()` (Lines 214-234)
- **Purpose**: Factory reset - clears all stored data
- **Side Effects**: 
  - Erases EEPROM content
  - Clears preferences storage
  - Resets global variables to defaults
- **Usage**: Called during factory reset or first boot

#### `convertAdcToTemperature(int adcValue)` (Lines 236-255)
- **Purpose**: Converts ADC reading to temperature for LM35 sensor
- **Input**: `adcValue` - Raw ADC reading (0-4095)
- **Returns**: `float` - Temperature in Celsius
- **Algorithm**: 
  - Converts ADC to voltage: `voltage = (adcValue/4095.0) * 3.3`
  - Converts to temperature: `tempC = voltage / 0.01` (LM35: 10mV/°C)
  - Range validation: -10°C to 100°C
- **Complex Note**: Includes sensor calibration and error handling

#### `factoryReset()` (Lines 257-267)
- **Purpose**: Complete system reset and restart
- **Side Effects**: 
  - Clears all storage
  - Resets WiFi settings
  - Restarts ESP32
- **Usage**: Triggered by long button press (5 seconds)

### Hàm phần cứng

#### `initializeHardware()` (Lines 272-340)
- **Purpose**: Initialize all hardware components and test functionality
- **Side Effects**: 
  - Sets up GPIO pins
  - Tests LED/buzzer with startup sequence
  - Initializes LCD, RTC, temperature sensor
  - Updates hardware status flags
- **Complex Note**: Includes comprehensive hardware validation

#### `readTemperature()` (Lines 342-351)
- **Purpose**: Read and update current temperature
- **Side Effects**: Updates `currentTemp`, `minTemp`, `maxTemp` global variables
- **Called**: Every 5 seconds in main loop (Lines 1532-1540)

### Hàm hiển thị

#### `updateLCDContent(String line1, String line2)` (Lines 356-370)
- **Purpose**: Update LCD display with new content (only if changed)
- **Input**: Two strings for LCD lines (max 16 chars each)
- **Side Effects**: Updates global LCD state variables
- **Optimization**: Only updates LCD if content actually changed

#### `displayClock()` (Lines 372-427)
- **Purpose**: Main clock display with automatic mode switching
- **Modes**: 
  - Mode 0: Time + Temperature + Status
  - Mode 1: Weather Information
- **Auto-switch**: Every 60 seconds (Line 378)
- **Complex Logic**: Dynamic status display based on system state

#### `displayCountdown()` (Lines 429-455)
- **Purpose**: Shows countdown timer on LCD
- **Logic**: Calculates remaining time and triggers 5-second alarm on completion
- **Format**: "TIMER: MM:SS" + timer label

#### `displayMenu()` (Lines 457-467)
- **Purpose**: Simple menu display with auto-exit after 10 seconds

### Hệ thống báo thức & bộ đếm

#### `triggerAlarm(int index)` (Lines 472-479)
- **Purpose**: Activate alarm for specific alarm index
- **Side Effects**: 
  - Sets alarm state variables
  - Changes system state to STATE_ALARM
  - Records alarm start time

#### `stopAlarm()` (Lines 481-491)
- **Purpose**: Stop active alarm and reset system
- **Side Effects**: 
  - Turns off buzzer/LED
  - Resets alarm flags
  - Returns to normal state

#### `updateAlarmDisplay()` (Lines 493-524)
- **Purpose**: Handle alarm visual/audio feedback with blinking pattern
- **Pattern**: 500ms blink cycle with buzzer/LED
- **Auto-stop**: After 5 minutes (Line 521)
- **Complex Note**: Handles both regular alarms and timer alarms

### Hệ thống xử lý nút nhấn

#### `handleButton()` (Lines 529-593)
- **Purpose**: Complex button handling with multiple functions
- **Features**:
  - Immediate buzzer shutoff
  - Debounced input processing  
  - Long press detection (5s = factory reset)
  - Interrupt coordination
- **Complex Note**: Dual-mode operation with interrupt backup

#### `buttonInterrupt()` (Lines 600-619) 
- **Purpose**: Hardware interrupt for immediate buzzer control
- **Trigger**: FALLING edge on BUTTON_PIN
- **Critical**: Marked with `IRAM_ATTR` for interrupt execution
- **Debouncing**: 50ms minimum interval between interrupts
- **Complex Note**: ISR must be minimal and fast

### Giao diện web

#### `generateWebInterface()` (Lines 624-995)
- **Purpose**: Generate complete HTML web interface
- **Returns**: String containing full HTML document
- **Features**:
  - Responsive CSS design
  - Real-time JavaScript updates
  - Form handling for all configurations
  - Modern UI with gradients and animations
- **Size**: ~371 lines of HTML/CSS/JavaScript generation
- **Complex Note**: Large function handling complete web UI

### Các endpoint máy chủ web

#### `setupWebServer()` (Lines 1000-1213)
- **Purpose**: Configure all HTTP endpoints
- **Endpoints**:
  - `/` - Main web interface
  - `/set-alarm` - Add new alarm
  - `/delete-alarm` - Remove alarm
  - `/set-timer` - Start countdown timer
  - `/stop-timer` - Stop timer
  - `/wifi-config` - WiFi settings
  - `/weather-config` - Weather API settings
  - `/refresh-weather` - Manual weather update
  - `/status` - JSON API for real-time data
  - `/restart` - Device restart
  - `/factory-reset` - Factory reset
- **JSON API**: Returns system status for real-time web updates

#### API Endpoints Reference

**GET Endpoints:**
- `GET /` - Returns complete HTML web interface
- `GET /status` - Returns JSON with system status, sensor data, and configuration

**POST Endpoints:**
- `POST /set-alarm` - Parameters: hour, minute, label, day0-day6 (checkboxes)
- `POST /delete-alarm` - Parameters: index (alarm index to delete)
- `POST /set-timer` - Parameters: minutes, label
- `POST /stop-timer` - No parameters, stops active timer
- `POST /wifi-config` - Parameters: hotspot_ssid, hotspot_password
- `POST /weather-config` - Parameters: api_key, city_name, update_interval, enabled (checkbox)
- `POST /refresh-weather` - No parameters, forces immediate weather update
- `POST /restart` - No parameters, restarts ESP32
- `POST /factory-reset` - No parameters, factory reset and restart

**JSON Response Format (/status):**
```json
{
  "temperature": 25.5,
  "weather": {
    "temp": 28.3,
    "humidity": 65,
    "description": "Clear sky",
    "city": "Thu Duc",
    "valid": true,
    "errors": 0
  },
  "lcd": {
    "line1": "12:34:56  25.5C",
    "line2": "13/07/25 WIFI",
    "mode": 0
  },
  "hardware": {
    "lcd": true,
    "rtc": true,
    "wifi": true,
    "temp": true,
    "buzzer": true,
    "led": true
  },
  "timer": {
    "active": false,
    "remaining": 0,
    "label": ""
  },
  "alarms": {
    "count": 3,
    "active": false
  }
}
```

### Quản lý cấu hình

#### `loadConfiguration()` (Lines 1218-1237)
- **Purpose**: Load device settings from EEPROM
- **Loads**: Device config, alarm count, alarm data, timer data
- **Fallback**: Default values if data invalid

#### `saveConfiguration()` (Lines 1239-1244)
- **Purpose**: Save device settings to EEPROM
- **Validation**: Sets configValid flag before saving

#### `saveAlarms()` (Lines 1246-1253)
- **Purpose**: Persist alarm data to EEPROM
- **Format**: Alarm count + array of Alarm structures

#### `loadWeatherConfig()` (Lines 1255-1270) 
- **Purpose**: Load weather settings from Preferences (NVS)
- **Storage**: Uses ESP32 Preferences library for non-volatile storage

#### `saveWeatherConfig()` (Lines 1272-1286)
- **Purpose**: Save weather configuration to Preferences
- **Debug**: Includes console output for verification

### Hàm mạng

#### `setupWiFi()` (Lines 1291-1311)
- **Purpose**: Initialize WiFi with auto-configuration portal
- **Modes**:
  - Station + AP mode if connected to WiFi
  - AP-only mode if WiFi fails
- **Auto-config**: Uses WiFiManager for easy setup
- **Fallback**: Creates hotspot for manual configuration

### Hàm hệ thống chính

#### `setup()` (Lines 1316-1388)
- **Purpose**: Complete system initialization
- **Flow**:
  1. Serial communication setup
  2. GPIO configuration  
  3. Hardware interrupt attachment
  4. Hardware initialization and testing
  5. First boot detection and data clearing
  6. Configuration loading
  7. WiFi setup and connection
  8. Web server startup
  9. Initial weather data fetch
  10. State machine initialization
- **Critical**: Button interrupt attached early (Line 1330)

#### `loop()` (Lines 1390-1456)
- **Purpose**: Main system loop with 100ms cycle time
- **Tasks**:
  1. Web server client handling
  2. Temperature reading (every 5s)
  3. Timer alarm handling (5-second alert)
  4. Button processing (every 50ms)
  5. Hardware status updates
  6. Weather data updates
  7. State machine execution
- **Máy trạng thái**: Handles display and functionality based on current state

### Hàm thời tiết

#### `fetchWeatherData()` (Lines 1461-1555)
- **Purpose**: Fetch weather data from OpenWeatherMap API
- **Features**:
  - Debug logging for troubleshooting
  - Fallback to simulation mode
  - Error count tracking and recovery
  - HTTP request with timeout
  - JSON parsing
  - Error handling for common HTTP codes
- **Complex Note**: Includes simulation mode, error recovery, and API throttling

#### `checkAlarms()` (Lines 1557-1574)
- **Purpose**: Check if any alarms should trigger
- **Logic**: Compares current time with enabled alarms
- **Day Matching**: Uses dayOfTheWeek() for weekly schedules
- **Timing**: Only triggers within first 5 seconds of minute

---

## Cấu hình phần cứng

### Phân công chân
- **GPIO 12**: Status LED (output)
- **GPIO 25**: Buzzer (output) 
- **GPIO 26**: User button (input with pull-up + interrupt)
- **GPIO 34**: Temperature sensor LM35 (analog input)
- **I2C**: LCD display (address 0x27)
- **I2C**: RTC DS1307 module

### Linh kiện bên ngoài
- **16x2 LCD Display**: I2C interface for status display
- **DS1307 RTC**: Real-time clock with battery backup
- **LM35 Temperature Sensor**: Analog temperature measurement
- **Buzzer**: Audio alarm output
- **LED**: Visual status indicator
- **Button**: User input with hardware interrupt

### Yêu cầu phần cứng
- **ESP32 Development Board** (minimum 4MB flash)
- **3.3V Power Supply** (minimum 500mA capacity)
- **I2C Pull-up Resistors** (4.7kΩ for SDA/SCL if not built-in)
- **Button Pull-up Resistor** (10kΩ, or use internal pull-up)
- **LM35 Decoupling Capacitor** (100nF ceramic + 10µF electrolytic)

### Sơ đồ kết nối
```
ESP32 Pin Connections:
GPIO12 ────[220Ω]────LED────GND
GPIO25 ────BUZZER(+)   BUZZER(-)────GND
GPIO26 ────BUTTON────GND (with 10kΩ pull-up to 3.3V)
GPIO34 ────LM35(OUT)   LM35(GND)────GND   LM35(VCC)────3.3V
GPIO21 ────SDA(I2C)────LCD_SDA, RTC_SDA
GPIO22 ────SCL(I2C)────LCD_SCL, RTC_SCL
3.3V   ────VCC────LCD_VCC, RTC_VCC
GND    ────GND────LCD_GND, RTC_GND
```

**I2C Addressing:**
- LCD Display: 0x27 (PCF8574 I2C backpack)
- RTC DS1307: 0x68 (fixed address)

**Power Consumption:**
- ESP32: ~240mA (WiFi active)
- LCD with backlight: ~20mA
- RTC DS1307: ~1.5mA
- LM35: ~60µA
- Total: ~260mA (excluding buzzer/LED during alarms)

---

## Giao diện web

### Features
- **Responsive Design**: Works on desktop and mobile
- **Cập nhật thời gian thực**: JavaScript polls `/status` endpoint every 2 seconds
- **Modern UI**: CSS gradients, animations, and card-based layout
- **Multi-language**: Vietnamese interface with emoji icons

### Thành phần giao diện
- **Header Section**: Large clock display with date and temperature
- **Trạng thái phần cứng Cards**: Real-time component status with color indicators
- **Weather Information**: Current conditions with error tracking
- **Configuration Forms**: Tabbed interface for different settings
- **Alarm Management**: Add/edit/delete alarms with weekly scheduling
- **Timer Controls**: Start/stop countdown with custom labels
- **Device Controls**: Restart and factory reset options

### Configuration Sections
1. **Trạng thái phần cứng**: Real-time component status
2. **Cấu hình thời tiết**: API key, city selection, update interval
3. **Alarm Management**: Add/remove alarms with weekly scheduling
4. **Bộ đếm ngược**: Start/stop timer with custom labels
5. **WiFi Configuration**: Network settings and hotspot config
6. **Device Control**: Restart and factory reset options

### Cập nhật thời gian thực
**Update Mechanism**: JavaScript `setInterval()` calls `/status` endpoint every 2 seconds
**Updated Elements**:
- Clock display (HH:MM format)
- Temperature readings
- LCD content display
- Hardware status indicators
- Weather information
- Timer countdown
- Alarm status

### Hàm JavaScript
**Core Functions** (Lines 1067-1088):
- `deleteAlarm(index)` - Remove alarm with confirmation
- `stopTimer()` - Stop active timer with confirmation
- `resetWiFi()` - Reset WiFi settings and restart
- `restart()` - Restart device with confirmation
- `factoryReset()` - Factory reset with double confirmation
- `refreshWeather()` - Force immediate weather update
- `updateStatus()` - Fetch and update all real-time data

**Real-time Update Logic**:
```javascript
function updateStatus() {
  fetch('/status')
    .then(r => r.json())
    .then(data => {
      // Update clock display
      // Update temperature displays
      // Update hardware status
      // Update weather info
      // Update timer display
    })
    .catch(e => console.log('Status update failed:', e));
}
```

---

## Máy trạng thái

### Định nghĩa trạng thái
```cpp
enum SystemState {
  STATE_BOOT,       // Initial startup and hardware initialization
  STATE_NORMAL,     // Normal operation with clock display
  STATE_ALARM,      // Active alarm with audio/visual feedback  
  STATE_COUNTDOWN,  // Timer countdown display
  STATE_INFO,       // Information display mode
  STATE_ERROR,      // Error state with diagnostics
  STATE_MENU        // Configuration menu mode
};
```

### States and Transitions
```
STATE_BOOT → STATE_NORMAL (after setup complete)
STATE_NORMAL → STATE_ALARM (alarm triggered)
STATE_NORMAL → STATE_COUNTDOWN (timer started)
STATE_NORMAL → STATE_MENU (button press in some conditions)
STATE_ALARM → STATE_NORMAL (alarm stopped)
STATE_COUNTDOWN → STATE_NORMAL (timer finished/stopped)
STATE_MENU → STATE_NORMAL (timeout or button press)
```

### Xử lý trạng thái (Lines 1578-1590)
- **STATE_NORMAL**: Display clock and check alarms
- **STATE_COUNTDOWN**: Display timer countdown
- **STATE_ALARM**: Handle alarm display with blinking
- **STATE_MENU**: Show menu with auto-exit

### Sơ đồ luồng trạng thái
```
┌─────────────┐
│ STATE_BOOT  │
└─────┬───────┘
      │ setup() complete
      ▼
┌─────────────┐    alarm triggered    ┌─────────────┐
│ STATE_NORMAL│◄────────────────────► │ STATE_ALARM │
└─────┬───────┘                      └─────────────┘
      │ timer started                         │
      ▼                              alarm stopped
┌─────────────┐    timer finished            │
│STATE_COUNTDOWN│◄─────────────────────────────┘
└─────────────┘
      │ timer stopped
      ▼
┌─────────────┐
│ STATE_NORMAL│
└─────────────┘
```

**State Persistence**: Current state and state start time are maintained globally
**State Timeouts**: 
- STATE_ALARM: Auto-stop after 5 minutes
- STATE_MENU: Auto-exit after 10 seconds
**State Priority**: ALARM state has highest priority, can interrupt other states

---

## Các phần phức tạp

### 1. Button Interrupt System (Lines 529-619)
**Complexity**: Dual-mode button handling with interrupt and polling
**Why Complex**: 
- Immediate buzzer shutoff requires interrupt
- Normal button functions need debouncing
- Coordination between ISR and main loop
- Long press detection for factory reset

### 2. Weather API with Error Recovery (Lines 1461-1555)
**Complexity**: Multiple fallback mechanisms and error handling
**Why Complex**:
- Network timeout handling
- JSON parsing with error detection
- API rate limiting and error counting
- Simulation mode fallback
- Periodic error reset to prevent permanent blocking

### 3. LCD Display Mode Switching (Lines 372-427)
**Complexity**: Automatic mode switching with different content formats
**Why Complex**:
- Time-based mode switching (60-second intervals)
- Different data sources (RTC vs weather)
- Dynamic status indication
- String formatting with size constraints

### 4. Timer with 5-Second Alarm (Lines 1542-1568)
**Complexity**: Separate alarm state within timer system
**Why Complex**:
- Timer completion triggers separate 5-second alarm
- Fast blink pattern (250ms) different from regular alarms
- State coordination between timer and alarm systems
- Manual stop capability during alarm phase

### 5. EEPROM vs Preferences Storage (Lines 1218-1286)
**Complexity**: Two different storage mechanisms
**Why Complex**:
- EEPROM for structured data (config, alarms)
- Preferences for weather configuration
- Different APIs and error handling
- Migration and compatibility considerations

---

## Bố trí bộ nhớ EEPROM

### Bản đồ bộ nhớ
```
Address 0-399:    Cấu hình thiết bị (DeviceConfig struct)
Address 400-799:  Hệ thống báo thức Data (count + Alarm array)
Address 800-1023: Timer Data (CountdownTimer struct)
```

### Chi tiết lưu trữ
- **Device Config**: Name, hotspot SSID/password, validity flag
- **Alarm Data**: Count (4 bytes) + up to 5 Alarm structures
- **Timer Data**: Duration, state, label for persistence across reboots
- **Weather Config**: Stored separately in ESP32 Preferences (NVS)

### Cấu trúc dữ liệu in Memory

**DeviceConfig Structure (88 bytes):**
```cpp
struct DeviceConfig {
  char deviceName[64];     // 64 bytes - Device identifier
  char hotspotSSID[32];    // 32 bytes - WiFi hotspot name  
  char hotspotPassword[32]; // 32 bytes - WiFi hotspot password
  bool configValid;        // 1 byte - Validity flag
} config;
```

**Alarm Storage Layout:**
```
Bytes 400-403: int alarmCount (number of active alarms)
Bytes 404-447: Alarm[0] structure (44 bytes)
Bytes 448-491: Alarm[1] structure (44 bytes)
Bytes 492-535: Alarm[2] structure (44 bytes)
Bytes 536-579: Alarm[3] structure (44 bytes)
Bytes 580-623: Alarm[4] structure (44 bytes)
```

**Alarm Structure (44 bytes each):**
```cpp
struct Alarm {
  int hour;              // 4 bytes - Hour (0-23)
  int minute;            // 4 bytes - Minute (0-59)
  bool enabled;          // 1 byte - Active flag
  bool daysOfWeek[7];    // 7 bytes - Weekly schedule
  char label[32];        // 32 bytes - Description text
};
```

### Sao lưu & phục hồi
- **Validation**: `configValid` flag prevents loading corrupted data
- **Fallback**: Default values used if EEPROM data invalid
- **Factory Reset**: Clears all EEPROM + Preferences storage
- **Persistence**: Data survives power cycles and firmware updates (if EEPROM layout unchanged)

---

## Tích hợp mạng & API

### Quản lý WiFi
- **WiFiManager**: Automatic configuration portal
- **Dual Mode**: Station + Access Point simultaneously
- **Fallback**: Hotspot-only mode if WiFi connection fails
- **Auto-reconnect**: Handled by ESP32 WiFi library

### OpenWeatherMap API
- **Endpoint**: `http://api.openweathermap.org/data/2.5/weather`
- **Parameters**: City, country, API key, units (metric), language (Vietnamese)
- **Rate Limiting**: Configurable update interval (default 2 minutes)
- **Error Handling**: Timeout, HTTP errors, JSON parsing errors
- **Fallback**: Simulation mode with random data

### Web Server
- **Port**: 80 (HTTP)
- **Framework**: ESP32 WebServer library
- **Endpoints**: 11 different endpoints for configuration and control
- **Content**: HTML, JSON, and plain text responses
- **Security**: Basic form validation, no authentication

### Các cân nhắc bảo mật
- **No Authentication**: Web interface is open to all network users
- **Local Network Only**: Server only accessible on local WiFi network
- **Input Validation**: Basic form validation and length limits
- **Factory Reset Protection**: Requires double confirmation
- **API Key Security**: Stored in NVS, masked in web interface
- **Network Security**: Relies on WiFi network security (WPA2/WPA3)

**Recommendations for Production:**
- Add basic HTTP authentication
- Implement HTTPS with self-signed certificates
- Add rate limiting for API endpoints
- Implement session management
- Add CSRF protection for forms

---

This documentation provides a comprehensive overview of the Smart Clock system. The code is well-structured with clear separation of concerns, though some functions (especially `generateWebInterface()` and `fetchWeatherData()`) are quite large and could benefit from refactoring into smaller functions.
