# Smart Clock ESP32 - Source Code Documentation

## Overview

The Smart Clock ESP32 is a feature-rich IoT device that combines real-time clock functionality with weather monitoring, alarm management, countdown timers, and web-based configuration. Built on the ESP32 platform, it provides both standalone operation and WiFi connectivity for enhanced features.

## File Structure

- **Main File**: `src/main1.cpp` - Contains the complete firmware implementation
- **Platform**: ESP32 microcontroller
- **Development Framework**: Arduino IDE/PlatformIO
- **Version**: v5.1 Enhanced

## Hardware Components

### Pin Configuration
```cpp
#define LED_PIN 12        // Status LED
#define BUZZER_PIN 25     // Alarm buzzer
#define BUTTON_PIN 26     // User input button
#define NTC_PIN 34        // Temperature sensor (analog)
```

### Hardware Modules
- **LCD Display**: 16x2 I2C LCD (address 0x27)
- **Real-Time Clock**: DS1307 RTC module
- **Temperature Sensor**: NTC thermistor on GPIO 34
- **Web Server**: ESP32 built-in WiFi with web interface
- **Storage**: EEPROM and Preferences for persistent data

## System Architecture

### State Machine
The system operates on a state machine with four primary states:

```cpp
enum SystemState {
    STATE_BOOT,      // Initial startup phase
    STATE_NORMAL,    // Normal clock operation
    STATE_ALARM,     // Active alarm state
    STATE_COUNTDOWN  // Timer countdown display
}
```

### Data Structures

#### Device Configuration
```cpp
struct DeviceConfig {
    char deviceName[64];      // Device identifier
    char hotspotSSID[32];     // WiFi hotspot name
    char hotspotPassword[32]; // WiFi hotspot password
    bool configValid;         // Configuration validity flag
}
```

#### Hardware Status
```cpp
struct HardwareStatus {
    bool lcdOK, rtcOK, wifiOK;     // Hardware component status
    bool tempOK, buzzerOK, ledOK;  // Additional hardware status
    String lastError;              // Last error message
}
```

#### Alarm System
```cpp
struct Alarm {
    int hour, minute;           // Alarm time
    bool enabled;               // Alarm active status
    bool daysOfWeek[7];        // Weekly schedule
    char label[32];            // Alarm description
}
```

#### Countdown Timer
```cpp
struct CountdownTimer {
    unsigned long duration;     // Timer duration in seconds
    unsigned long startTime;    // Timer start timestamp
    bool active, finished;      // Timer state flags
    char label[32];            // Timer description
    bool alarmTriggered;       // Completion alarm state
    unsigned long alarmStartTime; // Alarm start timestamp
}
```

#### Weather Data
```cpp
struct WeatherData {
    float temperature;         // Current temperature
    int humidity;             // Humidity percentage
    String description;       // Weather description
    String city;             // City name
    bool dataValid;          // Data validity flag
    unsigned long lastUpdate; // Last update timestamp
    int errorCount;          // API error counter
}
```

## Core Functionality

### 1. System Initialization (`setup()`)

The `setup()` function is the entry point that initializes all system components in a specific order to ensure proper operation.

#### Detailed Setup Flow

```cpp
void setup() {
    // Phase 1: Basic System Initialization
    Serial.begin(115200);                    // Start serial communication
    while (!Serial) delay(10);               // Wait for serial ready
    
    // Phase 2: GPIO and Interrupt Setup
    pinMode(BUTTON_PIN, INPUT_PULLUP);       // Configure button with pull-up
    pinMode(LED_PIN, OUTPUT);                // Configure LED as output
    pinMode(BUZZER_PIN, OUTPUT);             // Configure buzzer as output
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, FALLING);
    
    // Phase 3: Hardware Component Testing
    digitalWrite(LED_PIN, HIGH);             // Test LED ON
    digitalWrite(BUZZER_PIN, HIGH);          // Test buzzer ON
    delay(200);                             // Brief test duration
    digitalWrite(LED_PIN, LOW);              // Turn off LED
    digitalWrite(BUZZER_PIN, LOW);           // Turn off buzzer
    
    // Phase 4: I2C and Display Initialization
    LCD.init();                             // Initialize LCD display
    LCD.backlight();                        // Turn on LCD backlight
    hw.lcdOK = true;                        // Mark LCD as functional
    
    // Phase 5: RTC Initialization
    if (rtc.begin()) {
        hw.rtcOK = true;                    // RTC initialized successfully
    } else {
        hw.rtcOK = false;                   // RTC initialization failed
    }
    
    // Phase 6: Temperature Sensor Setup
    if (analogRead(NTC_PIN) > 0 && analogRead(NTC_PIN) < 4095) {
        hw.tempOK = true;                   // Temperature sensor working
        currentTemp = convertAdcToTemperature(analogRead(NTC_PIN));
    } else {
        hw.tempOK = false;                  // Temperature sensor failed
    }
    
    // Phase 7: Storage and Configuration
    if (checkFirstBoot()) {                 // Check if this is first boot
        clearAllData();                     // Clear all stored data
    }
    loadConfiguration();                    // Load device configuration
    loadWeatherConfig();                    // Load weather settings
    
    // Phase 8: System Variables Initialization
    lastLCDModeChange = millis();           // Initialize LCD mode timer
    weather.dataValid = false;              // Reset weather data
    weather.lastUpdate = 0;                 // Reset weather update time
    weather.errorCount = 0;                 // Reset error counter
    
    // Phase 9: Network Setup
    setupWiFi();                           // Initialize WiFi connection
    if (WiFi.status() == WL_CONNECTED) {
        hw.wifiOK = true;                  // WiFi connected successfully
        fetchWeatherData();                // Get initial weather data
    } else {
        hw.wifiOK = false;                 // WiFi connection failed
    }
    
    // Phase 10: WiFiManager Configuration
    wifiManager.setConfigPortalBlocking(false);  // Non-blocking portal
    wifiManager.startConfigPortal(config.hotspotSSID, config.hotspotPassword);
    
    // Phase 11: Final Setup
    updateLCDContent("Smart Clock v5.1", "Starting...");  // Show startup message
    delay(2000);                           // Display startup message
    currentState = STATE_NORMAL;           // Set operational state
}
```

#### Hardware Initialization Functions Called

1. **`checkFirstBoot()`** - Detects if device is starting for the first time
2. **`clearAllData()`** - Clears all EEPROM and Preferences data on first boot
3. **`convertAdcToTemperature(int adcValue)`** - Converts ADC reading to temperature
4. **`loadConfiguration()`** - Loads device settings from EEPROM
5. **`loadWeatherConfig()`** - Loads weather API settings from Preferences
6. **`setupWiFi()`** - Manages WiFi connection and configuration
7. **`fetchWeatherData()`** - Initial weather data fetch if WiFi available
8. **`updateLCDContent(String, String)`** - Updates LCD display content

### 2. Main Loop (`loop()`)

The `loop()` function executes continuously and manages all real-time operations through a series of processing blocks.

#### Detailed Loop Flow

```cpp
void loop() {
    // ===== [A] WiFi Management Block =====
    static bool triedReconnect = false;
    static unsigned long lastWiFiRetry = 0;
    
    if (WiFi.status() != WL_CONNECTED) {
        // WiFi disconnected - attempt reconnection every 30 seconds
        if (!triedReconnect && millis() - lastWiFiRetry > 30000) {
            lastWiFiRetry = millis();
            triedReconnect = true;
            WiFi.reconnect();               // Attempt WiFi reconnection
        }
        rtcSynced = false;                  // Reset NTP sync flag
    } else {
        triedReconnect = false;             // Reset reconnection flag
    }
    
    // ===== [B] WiFiManager Processing Block =====
    wifiManager.process();                  // Process config portal requests
    
    // Stop config portal when WiFi connects to free port for web server
    if (WiFi.status() == WL_CONNECTED && wifiManager.getConfigPortalActive()) {
        wifiManager.stopConfigPortal();
    }
    
    // ===== [C] Web Server Lifecycle Management =====
    if (WiFi.status() == WL_CONNECTED && !webServerStarted) {
        setupWebServer();                   // Start web server when WiFi connects
        webServerStarted = true;
    }
    if (WiFi.status() != WL_CONNECTED && webServerStarted) {
        webServerStarted = false;           // Mark server as stopped when WiFi disconnects
    }
    if (webServerStarted) {
        server.handleClient();              // Process web requests
    }
    
    // ===== [D] Temperature Monitoring Block =====
    static unsigned long lastTempRead = 0;
    if (millis() - lastTempRead > 5000) {   // Read every 5 seconds
        lastTempRead = millis();
        if (hw.tempOK) {
            int adcValue = analogRead(NTC_PIN);
            currentTemp = convertAdcToTemperature(adcValue);
        }
    }
    
    // ===== [E] Timer and Alarm Processing Block =====
    handleTimerAlarm();                     // Process timer completion alerts
    
    // ===== [F] Button Processing Block =====
    static unsigned long lastButtonCheck = 0;
    if (millis() - lastButtonCheck > 50) {  // Check every 50ms (debouncing)
        lastButtonCheck = millis();
        handleButton();                     // Process button state and actions
    }
    
    // ===== [G] NTP Synchronization Block =====
    if (WiFi.status() == WL_CONNECTED && !rtcSynced) {
        syncTimeWithNTP();                  // Sync RTC with NTP time
        rtcSynced = true;
    }
    
    // ===== [H] Hardware Status Update Block =====
    hw.wifiOK = (WiFi.status() == WL_CONNECTED);  // Update WiFi status
    
    // ===== [I] Weather Data Fetching Block =====
    if (WiFi.status() == WL_CONNECTED) {
        fetchWeatherData();                 // Fetch weather data from API
    }
    
    // ===== [J] State Machine Processing Block =====
    switch (currentState) {
        case STATE_NORMAL:
            displayClock();                 // Show clock display
            checkAlarms();                  // Check for alarm triggers
            break;
        case STATE_COUNTDOWN:
            displayCountdown();             // Show timer countdown
            if (!timer.active) {
                currentState = STATE_NORMAL; // Return to normal when timer stops
            }
            break;
        case STATE_ALARM:
            updateAlarmDisplay();           // Show alarm interface with blinking
            break;
    }
    
    // ===== [Z] CPU Load Management =====
    delay(100);                            // Prevent CPU overload
}
```

#### Loop Processing Functions Called

1. **WiFi Management Functions:**
   - `WiFi.reconnect()` - Attempt WiFi reconnection
   - `wifiManager.process()` - Handle configuration portal
   - `wifiManager.stopConfigPortal()` - Stop config portal
   - `wifiManager.getConfigPortalActive()` - Check portal status

2. **Web Server Functions:**
   - `setupWebServer()` - Initialize web server endpoints
   - `server.handleClient()` - Process HTTP requests

3. **Hardware Monitoring Functions:**
   - `convertAdcToTemperature(int)` - Convert temperature reading
   - `handleTimerAlarm()` - Process timer completion alerts
   - `handleButton()` - Process button input with debouncing

4. **Network Functions:**
   - `syncTimeWithNTP()` - Synchronize time with NTP servers
   - `fetchWeatherData()` - Fetch weather from OpenWeather API

5. **Display Functions:**
   - `displayClock()` - Main clock display with mode switching
   - `displayCountdown()` - Timer countdown display
   - `updateAlarmDisplay()` - Alarm state display with effects

6. **Alarm Functions:**
   - `checkAlarms()` - Check if any alarms should trigger

### Network and Communication Functions

#### `void setupWiFi()`
**Purpose**: Establishes WiFi connection using WiFiManager with custom configuration portal.

**Detailed Behavior**:
```cpp
void setupWiFi() {
    WiFiManager wifiManager;
    
    // Configure WiFiManager settings
    wifiManager.setConnectTimeout(30);     // 30-second connection timeout
    wifiManager.setConfigPortalTimeout(0); // No portal timeout (stays open)
    wifiManager.setBreakAfterConfig(true); // Exit portal after config
    
    // Add custom configuration parameters
    WiFiManagerParameter param_timezone("timezone", "Timezone (e.g., 7)", 
                                        String(config.timezone).c_str(), 3);
    WiFiManagerParameter param_weatherKey("weatherkey", "Weather API Key", 
                                          config.weatherApiKey, 33);
    WiFiManagerParameter param_city("city", "City Name", config.city, 31);
    WiFiManagerParameter param_deviceName("device", "Device Name", 
                                          config.deviceName, 31);
    
    wifiManager.addParameter(&param_timezone);
    wifiManager.addParameter(&param_weatherKey);
    wifiManager.addParameter(&param_city);
    wifiManager.addParameter(&param_deviceName);
    
    // Attempt auto-connection first
    bool connected = wifiManager.autoConnect("ESP32_CLOCK_SETUP");
    
    if (connected) {
        // Save any updated parameters
        config.timezone = param_timezone.getValue();
        strcpy(config.weatherApiKey, param_weatherKey.getValue());
        strcpy(config.city, param_city.getValue());
        strcpy(config.deviceName, param_deviceName.getValue());
        saveConfig();  // Persist changes
        
        // Configure timezone and sync time
        configTime(config.timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
        syncTimeWithNTP();
        hw.wifiOK = true;
    } else {
        hw.wifiOK = false;
    }
}
```

**Called by**: `setup()` and WiFi reconnection logic
**Calls**: `saveConfig()`, `syncTimeWithNTP()`, WiFiManager methods
**Global Variables Used**: `config`, `hw.wifiOK`

#### `void syncTimeWithNTP()`
**Purpose**: Synchronizes RTC with Network Time Protocol servers.

**Detailed Behavior**:
```cpp
void syncTimeWithNTP() {
    if (!hw.wifiOK) return;  // Must have WiFi connection
    
    // Configure NTP with timezone offset
    configTime(config.timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
    
    struct tm timeinfo;
    int attempts = 0;
    
    // Wait up to 10 seconds for time sync
    while (!getLocalTime(&timeinfo) && attempts < 10) {
        delay(1000);
        attempts++;
    }
    
    if (attempts < 10) {
        // Successfully got time from NTP
        DateTime ntpTime = DateTime(timeinfo.tm_year + 1900,
                                   timeinfo.tm_mon + 1,
                                   timeinfo.tm_mday,
                                   timeinfo.tm_hour,
                                   timeinfo.tm_min,
                                   timeinfo.tm_sec);
        
        rtc.adjust(ntpTime);    // Update RTC with NTP time
        hw.ntpSynced = true;    // Mark as successfully synced
    }
}
```

**Called by**: `setupWiFi()`, periodic sync operations
**Calls**: `configTime()`, `getLocalTime()`, `rtc.adjust()`
**Global Variables Used**: `hw.wifiOK`, `config.timezone`, `hw.ntpSynced`

#### `void fetchWeatherData()`
**Purpose**: Retrieves weather information from OpenWeatherMap API.

**Detailed Behavior**:
```cpp
void fetchWeatherData() {
    if (!hw.wifiOK || strlen(config.weatherApiKey) == 0) {
        weather.dataValid = false;
        return;
    }
    
    HTTPClient http;
    WiFiClient client;
    
    // Construct API URL
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + 
                 String(config.city) + "&appid=" + String(config.weatherApiKey) + 
                 "&units=metric";
    
    http.begin(client, url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Parse JSON response
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            // Extract weather data
            weather.temperature = doc["main"]["temp"];
            weather.humidity = doc["main"]["humidity"];
            weather.pressure = doc["main"]["pressure"];
            weather.city = doc["name"];
            weather.description = doc["weather"][0]["description"];
            weather.dataValid = true;
            weather.lastUpdate = millis();
            
            // Update current temperature for display
            currentTemp = weather.temperature;
        } else {
            weather.dataValid = false;
        }
    } else {
        weather.dataValid = false;
    }
    
    http.end();
}
```

**Called by**: Main loop every 10 minutes
**Calls**: HTTP and JSON parsing functions
**Global Variables Used**: `hw.wifiOK`, `config`, `weather`, `currentTemp`

### Web Server Functions

#### `void setupWebServer()`
**Purpose**: Configures and starts the web server with all API endpoints.

**Detailed Behavior**:
```cpp
void setupWebServer() {
    if (!hw.wifiOK) return;  // Requires WiFi connection
    
    // Enable CORS for all responses
    server.enableCORS(true);
    
    // Serve static files
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    
    // API Endpoints
    server.on("/api/esp32-time", HTTP_GET, []() {
        DateTime now = rtc.now();
        DynamicJsonDocument doc(256);
        
        doc["time"] = String(now.hour()) + ":" + 
                     String(now.minute()) + ":" + 
                     String(now.second());
        doc["date"] = String(now.day()) + "/" + 
                     String(now.month()) + "/" + 
                     String(now.year());
        doc["timestamp"] = now.unixtime();
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });
    
    server.on("/api/esp32-status", HTTP_GET, []() {
        DynamicJsonDocument doc(512);
        
        doc["wifi"] = hw.wifiOK;
        doc["ntp_synced"] = hw.ntpSynced;
        doc["device_name"] = config.deviceName;
        doc["uptime"] = millis();
        doc["free_heap"] = ESP.getFreeHeap();
        doc["alarm_count"] = alarmCount;
        doc["timer_active"] = timer.active;
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });
    
    server.on("/api/alarms", HTTP_GET, []() {
        DynamicJsonDocument doc(2048);
        JsonArray alarmsArray = doc.createNestedArray("alarms");
        
        for (int i = 0; i < alarmCount; i++) {
            JsonObject alarm = alarmsArray.createNestedObject();
            alarm["id"] = i;
            alarm["hour"] = alarms[i].hour;
            alarm["minute"] = alarms[i].minute;
            alarm["label"] = alarms[i].label;
            alarm["enabled"] = alarms[i].enabled;
            
            JsonArray days = alarm.createNestedArray("days");
            for (int j = 0; j < 7; j++) {
                days.add(alarms[i].daysOfWeek[j]);
            }
        }
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });
    
    server.on("/api/set-alarm", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error && alarmCount < MAX_ALARMS) {
                alarms[alarmCount].hour = doc["hour"];
                alarms[alarmCount].minute = doc["minute"];
                strcpy(alarms[alarmCount].label, doc["label"]);
                alarms[alarmCount].enabled = doc["enabled"];
                
                for (int i = 0; i < 7; i++) {
                    alarms[alarmCount].daysOfWeek[i] = doc["days"][i];
                }
                
                alarmCount++;
                saveAlarms();
                server.send(200, "application/json", "{\"success\":true}");
            } else {
                server.send(400, "application/json", "{\"error\":\"Invalid data\"}");
            }
        }
    });
    
    // Additional endpoints for update, delete, etc...
    
    server.begin();    // Start the server
    hw.webServerOK = true;
}
```

**Called by**: `setup()` after WiFi is established
**Calls**: Various server configuration methods, JSON serialization
**Global Variables Used**: `hw`, `config`, `alarms[]`, `alarmCount`, `timer`, `server`

### Configuration and Storage Functions

#### `void loadConfig()`
**Purpose**: Loads system configuration from persistent storage (Preferences).

**Detailed Behavior**:
```cpp
void loadConfig() {
    preferences.begin("config", false);  // Open config namespace
    
    // Load with defaults if not found
    config.timezone = preferences.getInt("timezone", 7);                    // Default: UTC+7
    preferences.getString("weatherKey", config.weatherApiKey, 
                         sizeof(config.weatherApiKey));                     // Weather API key
    preferences.getString("city", config.city, sizeof(config.city));        // City name
    preferences.getString("deviceName", config.deviceName, 
                         sizeof(config.deviceName));                        // Device name
    
    // Set defaults if empty
    if (strlen(config.weatherApiKey) == 0) {
        strcpy(config.weatherApiKey, "your_api_key_here");
    }
    if (strlen(config.city) == 0) {
        strcpy(config.city, "Ho Chi Minh City");
    }
    if (strlen(config.deviceName) == 0) {
        strcpy(config.deviceName, "ESP32 Clock");
    }
    
    preferences.end();
}
```

**Called by**: `setup()`
**Global Variables Used**: `config`, `preferences`

#### `void saveConfig()`
**Purpose**: Saves current configuration to persistent storage.

**Detailed Behavior**:
```cpp
void saveConfig() {
    preferences.begin("config", false);
    
    preferences.putInt("timezone", config.timezone);
    preferences.putString("weatherKey", config.weatherApiKey);
    preferences.putString("city", config.city);
    preferences.putString("deviceName", config.deviceName);
    
    preferences.end();
}
```

**Called by**: WiFi setup, configuration changes
**Global Variables Used**: `config`, `preferences`

#### `void loadAlarms()`
**Purpose**: Loads all configured alarms from EEPROM storage.

**Detailed Behavior**:
```cpp
void loadAlarms() {
    EEPROM.begin(sizeof(int) + sizeof(Alarm) * MAX_ALARMS);
    
    // Read alarm count first
    EEPROM.get(0, alarmCount);
    
    // Validate alarm count
    if (alarmCount < 0 || alarmCount > MAX_ALARMS) {
        alarmCount = 0;  // Reset if corrupted
        return;
    }
    
    // Read each alarm
    int offset = sizeof(int);
    for (int i = 0; i < alarmCount; i++) {
        EEPROM.get(offset, alarms[i]);
        offset += sizeof(Alarm);
    }
}
```

**Called by**: `setup()`
**Global Variables Used**: `alarms[]`, `alarmCount`

#### `void saveAlarms()`
**Purpose**: Saves all current alarms to EEPROM storage.

**Detailed Behavior**:
```cpp
void saveAlarms() {
    EEPROM.begin(sizeof(int) + sizeof(Alarm) * MAX_ALARMS);
    
    // Write alarm count first
    EEPROM.put(0, alarmCount);
    
    // Write each alarm
    int offset = sizeof(int);
    for (int i = 0; i < alarmCount; i++) {
        EEPROM.put(offset, alarms[i]);
        offset += sizeof(Alarm);
    }
    
    EEPROM.commit();  // Ensure data is written to flash
}
```

**Called by**: Alarm management functions
**Global Variables Used**: `alarms[]`, `alarmCount`

#### `void factoryReset()`
**Purpose**: Resets device to factory defaults and clears all stored data.

**Detailed Behavior**:
```cpp
void factoryReset() {
    updateLCDContent("Factory Reset", "Please wait...");
    
    // Clear all EEPROM data
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    
    // Clear preferences
    preferences.begin("config", false);
    preferences.clear();
    preferences.end();
    
    // Clear WiFi credentials
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    updateLCDContent("Reset Complete", "Restarting...");
    delay(2000);
    
    ESP.restart();  // Restart device
}
```

**Called by**: Long button press (5+ seconds)
**Calls**: `updateLCDContent()`, storage clearing functions, `ESP.restart()`

### Utility and Helper Functions

#### `void switchLcdDisplayMode()`
**Purpose**: Manually switches between LCD display modes.

**Detailed Behavior**:
```cpp
void switchLcdDisplayMode() {
    lcdDisplayMode = (lcdDisplayMode + 1) % 2;  // Toggle between 0 and 1
    lastLCDModeChange = millis();               // Reset auto-switch timer
}
```

**Called by**: `handleButton()` on short press
**Global Variables Used**: `lcdDisplayMode`, `lastLCDModeChange`

#### `bool isValidTimeFormat(String timeStr)`
**Purpose**: Validates time string format (HH:MM).

**Detailed Behavior**:
```cpp
bool isValidTimeFormat(String timeStr) {
    if (timeStr.length() != 5) return false;        // Must be exactly 5 chars
    if (timeStr.charAt(2) != ':') return false;     // Must have colon at position 2
    
    String hourStr = timeStr.substring(0, 2);       // Extract hour part
    String minStr = timeStr.substring(3, 5);        // Extract minute part
    
    int hour = hourStr.toInt();
    int minute = minStr.toInt();
    
    // Validate numeric ranges
    return (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59);
}
```

**Called by**: Web API endpoints for time validation
**Parameters**: Time string in "HH:MM" format
**Returns**: `true` if valid, `false` otherwise

#### `String formatTime(int hour, int minute)`
**Purpose**: Formats time integers into standardized string format.

**Detailed Behavior**:
```cpp
String formatTime(int hour, int minute) {
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
    return String(buffer);
}
```

**Called by**: Display and web API functions
**Parameters**: Hour (0-23) and minute (0-59)
**Returns**: Formatted time string "HH:MM"

#### `void displayClock()`
**Purpose**: Main display function that shows current time and alternates between clock and weather modes.

**Detailed Behavior**:
```cpp
void displayClock() {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();
    
    // Auto-switch display mode every 60 seconds
    if (now - lastLCDModeChange > 60000) {
        lcdDisplayMode = (lcdDisplayMode + 1) % 2;  // Toggle between 0 and 1
        lastLCDModeChange = now;
    }
    
    // Update display only every 1000ms (1 second)
    if (now - lastUpdate < 1000) return;
    lastUpdate = now;
    
    DateTime rtcNow = rtc.now();  // Get current time from RTC
    
    if (lcdDisplayMode == 0) {
        // Mode 0: Clock + Temperature
        // Line 1: "HH:MM:SS TT.TC" (time and temperature)
        // Line 2: "DD/MM/YY STATUS" (date and system status)
        char line1[17];
        snprintf(line1, sizeof(line1), "%02d:%02d:%02d %4.1fC",
                 rtcNow.hour(), rtcNow.minute(), rtcNow.second(), currentTemp);
        
        char line2[17];
        String status = hw.wifiOK ? "WIFI" : "DISC";
        if (alarmCount > 0) status = "A" + String(alarmCount);
        if (timer.active) status = "TIMER";
        
        snprintf(line2, sizeof(line2), "%02d/%02d/%02d %s",
                 rtcNow.day(), rtcNow.month(), rtcNow.year() % 100, status.c_str());
    } else {
        // Mode 1: Weather Information
        // Line 1: "CITY: TT.TC" (city and temperature)
        // Line 2: "Humidity: XX%" (humidity percentage)
        if (weather.dataValid) {
            snprintf(line1, sizeof(line1), "%s: %.1fC",
                     weather.city.substring(0, 8).c_str(), weather.temperature);
            snprintf(line2, sizeof(line2), "%s: %d%%", "Humidity", weather.humidity);
        } else {
            strcpy(line1, "Weather");
            strcpy(line2, "No data...");
        }
    }
    
    updateLCDContent(String(line1), String(line2));  // Update LCD if content changed
}
```

**Called by**: Main loop in STATE_NORMAL
**Calls**: `rtc.now()`, `updateLCDContent()`
**Global Variables Used**: `lcdDisplayMode`, `lastLCDModeChange`, `currentTemp`, `hw.wifiOK`, `alarmCount`, `timer.active`, `weather`

#### `void displayCountdown()`
**Purpose**: Shows countdown timer with remaining time in MM:SS format.

**Detailed Behavior**:
```cpp
void displayCountdown() {
    if (!timer.active) return;  // Exit if no active timer
    
    // Calculate elapsed time since timer started
    unsigned long elapsed = (millis() - timer.startTime) / 1000;
    
    // Check if timer has finished
    if (elapsed >= timer.duration) {
        timer.active = false;           // Stop timer
        timer.finished = true;          // Mark as finished
        timer.alarmTriggered = true;    // Start completion alarm
        timer.alarmStartTime = millis(); // Record alarm start time
        return;
    }
    
    // Calculate remaining time
    unsigned long remaining = timer.duration - elapsed;
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    
    // Format display: "TIMER: MM:SS"
    char line1[17];
    snprintf(line1, sizeof(line1), "TIMER: %02d:%02d", minutes, seconds);
    
    updateLCDContent(String(line1), String(timer.label));
}
```

**Called by**: Main loop in STATE_COUNTDOWN
**Calls**: `updateLCDContent()`
**Global Variables Used**: `timer` (all fields)

#### `void updateAlarmDisplay()`
**Purpose**: Manages alarm display with blinking effect and auto-timeout.

**Detailed Behavior**:
```cpp
void updateAlarmDisplay() {
    static unsigned long lastBlink = 0;
    static bool blinkState = false;
    
    // Blink every 500ms for visual alarm indication
    if (millis() - lastBlink > 500) {
        lastBlink = millis();
        blinkState = !blinkState;
        
        if (blinkState) {
            // Show alarm information
            String label = (activeAlarmIndex >= 0) ? 
                          alarms[activeAlarmIndex].label : 
                          (timer.finished ? timer.label : "ALARM");
            updateLCDContent("*** ALARM ***", label);
            
            // Activate buzzer and LED
            digitalWrite(BUZZER_PIN, HIGH);
            digitalWrite(LED_PIN, HIGH);
        } else {
            // Clear display and turn off alerts
            LCD.clear();
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(LED_PIN, LOW);
        }
    }
    
    // Auto-stop alarm after 5 minutes
    if (millis() - stateStartTime > 5 * 60 * 1000) {
        stopAlarm();
    }
}
```

**Called by**: Main loop in STATE_ALARM
**Calls**: `updateLCDContent()`, `stopAlarm()`, `digitalWrite()`, `LCD.clear()`
**Global Variables Used**: `activeAlarmIndex`, `alarms[]`, `timer`, `stateStartTime`

#### `void updateLCDContent(String line1, String line2)`
**Purpose**: Low-level LCD update with change detection to minimize unnecessary writes.

**Detailed Behavior**:
```cpp
void updateLCDContent(String line1, String line2) {
    // Only update LCD if content has actually changed
    if (line1 != currentLCDLine1 || line2 != currentLCDLine2) {
        currentLCDLine1 = line1;  // Store new content
        currentLCDLine2 = line2;
        
        LCD.clear();              // Clear previous content
        LCD.setCursor(0, 0);      // Position at top-left
        LCD.print(line1);         // Print first line
        LCD.setCursor(0, 1);      // Position at second line
        LCD.print(line2);         // Print second line
    }
}
```

**Called by**: All display functions
**Global Variables Used**: `currentLCDLine1`, `currentLCDLine2`, `LCD`

### Button and Interrupt Functions

#### `void handleButton()`
**Purpose**: Processes button input with debouncing and context-sensitive actions.

**Detailed Behavior**:
```cpp
void handleButton() {
    static unsigned long lastPress = 0;
    static bool lastState = HIGH;
    static unsigned long pressStartTime = 0;
    
    // Check for interrupt-requested buzzer stop first
    if (buzzerStopRequested) {
        buzzerStopRequested = false;
        
        // Handle alarm states
        if (currentState == STATE_ALARM || alarmActive) {
            stopAlarm();
            return;
        }
        
        // Handle timer alarm
        if (timer.alarmTriggered) {
            timer.alarmTriggered = false;
            timer.finished = false;
            return;
        }
    }
    
    bool buttonState = digitalRead(BUTTON_PIN);
    
    // Immediate buzzer shutoff (backup to interrupt)
    if (buttonState == LOW && digitalRead(BUZZER_PIN) == HIGH) {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        
        if (currentState == STATE_ALARM || alarmActive) {
            stopAlarm();
            return;
        }
        
        if (timer.alarmTriggered) {
            timer.alarmTriggered = false;
            timer.finished = false;
            return;
        }
    }
    
    // Debounced button state change detection
    if (buttonState != lastState && millis() - lastPress > 50) {
        lastPress = millis();
        
        if (buttonState == LOW) {
            pressStartTime = millis();  // Record press start time
        } else {
            // Button released - determine action based on press duration
            unsigned long pressDuration = millis() - pressStartTime;
            
            if (pressDuration >= 5000) {
                factoryReset();         // Long press: factory reset
            } else {
                // Short press: context-sensitive action
                if (currentState == STATE_ALARM) {
                    stopAlarm();
                } else if (timer.alarmTriggered) {
                    timer.alarmTriggered = false;
                    timer.finished = false;
                    digitalWrite(BUZZER_PIN, LOW);
                    digitalWrite(LED_PIN, LOW);
                } else if (currentState == STATE_NORMAL) {
                    switchLcdDisplayMode();  // Switch LCD display mode
                }
            }
        }
    }
    
    lastState = buttonState;
}
```

**Called by**: Main loop every 50ms
**Calls**: `stopAlarm()`, `factoryReset()`, `switchLcdDisplayMode()`, `digitalRead()`, `digitalWrite()`
**Global Variables Used**: `buzzerStopRequested`, `currentState`, `alarmActive`, `timer`

#### `void IRAM_ATTR buttonInterrupt()`
**Purpose**: Interrupt service routine for immediate buzzer control.

**Detailed Behavior**:
```cpp
void IRAM_ATTR buttonInterrupt() {
    unsigned long currentTime = millis();
    
    // Debouncing: ignore interrupts within 50ms
    if (currentTime - lastInterruptTime > 50) {
        lastInterruptTime = currentTime;
        
        // Verify button is actually pressed (LOW due to INPUT_PULLUP)
        if (digitalRead(BUTTON_PIN) == LOW) {
            // Check if buzzer or LED is currently active
            if (digitalRead(BUZZER_PIN) == HIGH || digitalRead(LED_PIN) == HIGH) {
                buzzerStopRequested = true;  // Set flag for main loop
            }
        }
    }
}
```

**Called by**: Hardware interrupt on FALLING edge of BUTTON_PIN
**Global Variables Used**: `lastInterruptTime`, `buzzerStopRequested`
**Special Notes**: IRAM_ATTR ensures function is stored in RAM for faster interrupt response

### Alarm and Timer Functions

#### `void checkAlarms()`
**Purpose**: Checks all configured alarms against current time and triggers matches.

**Detailed Behavior**:
```cpp
void checkAlarms() {
    if (alarmActive || currentState != STATE_NORMAL) return;  // Don't check during alarms
    
    DateTime now = rtc.now();                    // Get current time
    int currentDay = now.dayOfTheWeek();         // Get day of week (0=Sunday)
    
    for (int i = 0; i < alarmCount; i++) {
        if (alarms[i].enabled &&                 // Alarm must be enabled
            alarms[i].hour == now.hour() &&      // Hour must match
            alarms[i].minute == now.minute() &&  // Minute must match
            now.second() < 5 &&                  // Only trigger in first 5 seconds
            alarms[i].daysOfWeek[currentDay]) {  // Day must be enabled
            
            triggerAlarm(i);                     // Trigger this alarm
            return;                              // Only one alarm at a time
        }
    }
}
```

**Called by**: Main loop in STATE_NORMAL
**Calls**: `rtc.now()`, `triggerAlarm()`
**Global Variables Used**: `alarmActive`, `currentState`, `alarmCount`, `alarms[]`

#### `void triggerAlarm(int index)`
**Purpose**: Activates a specific alarm and transitions to alarm state.

**Detailed Behavior**:
```cpp
void triggerAlarm(int index) {
    activeAlarmIndex = index;           // Store which alarm is active
    alarmActive = true;                 // Set global alarm flag
    timer.finished = false;             // Reset timer finished flag
    currentState = STATE_ALARM;         // Change to alarm state
    stateStartTime = millis();          // Record when alarm started
}
```

**Called by**: `checkAlarms()`
**Global Variables Used**: `activeAlarmIndex`, `alarmActive`, `timer.finished`, `currentState`, `stateStartTime`

#### `void stopAlarm()`
**Purpose**: Deactivates all alarms and returns to normal operation.

**Detailed Behavior**:
```cpp
void stopAlarm() {
    alarmActive = false;                // Clear alarm flag
    timer.active = false;               // Stop any active timer
    timer.finished = false;             // Clear timer finished flag
    digitalWrite(BUZZER_PIN, LOW);      // Turn off buzzer
    digitalWrite(LED_PIN, LOW);         // Turn off LED
    currentState = STATE_NORMAL;        // Return to normal state
    activeAlarmIndex = -1;              // Clear active alarm index
}
```

**Called by**: `handleButton()`, `updateAlarmDisplay()`, interrupt handler
**Global Variables Used**: `alarmActive`, `timer`, `currentState`, `activeAlarmIndex`

#### `void handleTimerAlarm()`
**Purpose**: Manages the 5-second completion alarm when countdown timer finishes.

**Detailed Behavior**:
```cpp
void handleTimerAlarm() {
    if (timer.alarmTriggered) {
        unsigned long alarmElapsed = millis() - timer.alarmStartTime;
        
        if (alarmElapsed < 5000) {      // During 5-second alarm period
            static unsigned long lastBlinkTimer = 0;
            
            if (millis() - lastBlinkTimer > 250) {  // Fast blink every 250ms
                lastBlinkTimer = millis();
                static bool timerBlinkState = false;
                timerBlinkState = !timerBlinkState;
                
                if (timerBlinkState) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    digitalWrite(LED_PIN, HIGH);
                    updateLCDContent("*** TIMER ***", timer.label);
                } else {
                    digitalWrite(BUZZER_PIN, LOW);
                    digitalWrite(LED_PIN, LOW);
                    LCD.clear();
                }
            }
        } else {
            // 5-second alarm finished
            timer.alarmTriggered = false;
            timer.finished = false;
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(LED_PIN, LOW);
        }
    }
}
```

**Called by**: Main loop every cycle
**Calls**: `updateLCDContent()`, `digitalWrite()`, `LCD.clear()`
**Global Variables Used**: `timer` (all fields)

### 4. Button and Interrupt Handling

#### Button Processing Logic
```cpp
void handleButton()           // Main button handling with debouncing
void IRAM_ATTR buttonInterrupt() // Interrupt service routine
```

#### Button Behavior
- **Short Press**: Context-sensitive actions (stop alarms, switch display)
- **Long Press (5+ seconds)**: Factory reset
- **Interrupt Response**: Immediate buzzer shutoff capability
- **Debouncing**: 50ms hardware debouncing

### 5. Alarm and Timer System

#### Alarm Management
- **Multi-alarm Support**: Up to 5 simultaneous alarms
- **Weekly Scheduling**: Day-of-week repeat patterns
- **Label Support**: Custom alarm descriptions
- **Persistent Storage**: EEPROM-based alarm storage

#### Timer Functions
```cpp
void triggerAlarm(int index)  // Activate specific alarm
void stopAlarm()              // Deactivate all alarms
void handleTimerAlarm()       // Process timer completion alerts
```

#### Timer Features
- **Countdown Display**: Visual countdown with minutes:seconds format
- **Completion Alert**: 5-second buzzer/LED alert when timer finishes
- **Custom Labels**: User-defined timer descriptions
- **Web Control**: Start/stop timers via web interface

### 6. Weather Integration

#### Weather Data Management
```cpp
void fetchWeatherData()      // Fetch data from OpenWeather API
void loadWeatherConfig()     // Load weather settings
void saveWeatherConfig()     // Save weather settings
```

#### Weather Features
- **OpenWeather API**: Integration with weather service
- **Vietnamese Cities**: Predefined city list for Vietnam
- **Error Handling**: API failure management and retry logic
- **Simulation Mode**: Fallback simulation when API unavailable
- **Update Throttling**: 10-second minimum update interval

### 7. Web Interface

#### Web Server Features
- **Responsive Design**: Mobile-friendly interface with Bootstrap-like styling
- **Real-time Updates**: JavaScript-based status updates every 2 seconds
- **Multi-language**: Vietnamese interface with emoji icons
- **Progressive Enhancement**: Works without JavaScript

#### Web Endpoints
```cpp
"/"                  // Main dashboard interface
"/set-alarm"         // Add new alarm
"/delete-alarm"      // Remove existing alarm
"/set-timer"         // Start countdown timer
"/stop-timer"        // Stop active timer
"/weather-config"    // Configure weather settings
"/wifi-config"       // Configure WiFi settings
"/status"           // JSON API for real-time updates
"/restart"          // Device restart
"/factory-reset"    // Factory reset
"/reset-wifi"       // WiFi settings reset
```

#### Web Interface Sections
1. **Header**: Current time, date, and temperature
2. **LCD Display**: Real-time LCD content mirror
3. **Hardware Status**: Component health indicators
4. **Weather Information**: Current weather and configuration
5. **Alarm Management**: Add, view, and delete alarms
6. **Timer Control**: Countdown timer interface
7. **WiFi Configuration**: Network settings management
8. **Device Control**: System restart and reset options

### 8. Storage and Configuration

#### EEPROM Layout
```cpp
#define CONFIG_ADDR 0     // Device configuration (offset 0)
#define ALARM_ADDR 400    // Alarm data (offset 400)
#define TIMER_ADDR 800    // Timer data (offset 800)
```

#### Preferences (NVS) Usage
- **Weather Settings**: API keys, city selection, update intervals
- **Persistent Flags**: First boot detection, calibration data
- **User Preferences**: Display modes, custom settings

#### Configuration Functions
```cpp
void loadConfiguration()    // Load device settings from EEPROM
void saveConfiguration()    // Save device settings to EEPROM
void saveAlarms()          // Save alarm data to EEPROM
void loadWeatherConfig()   // Load weather settings from Preferences
void saveWeatherConfig()   // Save weather settings to Preferences
```

### 9. Utility Functions

#### Hardware Utilities
```cpp
bool checkFirstBoot()                      // Detect initial startup
void clearAllData()                        // Factory reset data clearing
float convertAdcToTemperature(int adcValue) // Temperature conversion
void factoryReset()                        // Complete system reset
```

#### Network Utilities
```cpp
void setupWiFi()           // WiFi connection management
void tryConnectWiFiFirst() // Attempt saved network connection
void syncTimeWithNTP()     // NTP time synchronization
```

## Error Handling and Recovery

### WiFi Error Handling
- **Connection Failures**: Automatic retry with exponential backoff
- **Portal Fallback**: WiFiManager configuration portal activation
- **Service Degradation**: Graceful function reduction when offline

### Hardware Error Handling
- **Sensor Failures**: Fallback values and error state indication
- **RTC Failures**: NTP synchronization as backup time source
- **Storage Errors**: Default configuration loading

### Weather API Error Handling
- **API Key Validation**: Invalid key detection and user notification
- **Rate Limiting**: Request throttling to prevent API abuse
- **Simulation Fallback**: Local weather simulation when API unavailable

## Security Considerations

### WiFi Security
- **WPA2/WPA3 Support**: Modern WiFi security protocols
- **Hotspot Protection**: Configurable hotspot passwords
- **Network Isolation**: Separate configuration and operational networks

### Web Interface Security
- **Input Validation**: Server-side validation of all form inputs
- **CSRF Protection**: Form-based request validation
- **Access Control**: Local network access restriction

## Performance Optimization

### Memory Management
- **Static Allocation**: Minimal dynamic memory allocation
- **Buffer Management**: Fixed-size buffers for string operations
- **Stack Optimization**: Careful function call hierarchy

### CPU Optimization
- **Non-blocking Operations**: Asynchronous network operations
- **Interrupt-driven I/O**: Hardware interrupt for critical responses
- **Throttled Updates**: Minimum intervals for expensive operations

### Power Management
- **Sleep Modes**: Strategic use of delay() for power savings
- **Peripheral Management**: Conditional activation of power-hungry components
- **LED/Buzzer Control**: Optimized duty cycles for alerting

## Development Guidelines

### Code Organization
- **Modular Structure**: Logical grouping of related functions
- **Clear Naming**: Descriptive function and variable names
- **Comment Standards**: Comprehensive inline documentation

### Testing Considerations
- **Hardware Testing**: Individual component validation
- **Network Testing**: WiFi and web server functionality
- **Integration Testing**: End-to-end feature validation

### Debugging Features
- **Serial Logging**: Comprehensive debug output
- **Status Indicators**: Visual hardware status feedback
- **Web Diagnostics**: Real-time system status via web interface

## Future Enhancement Opportunities

### Feature Extensions
- **MQTT Integration**: IoT platform connectivity
- **Cloud Synchronization**: Remote alarm and configuration management
- **Mobile App**: Dedicated smartphone application
- **Voice Control**: Speech recognition for alarm management

### Hardware Enhancements
- **Color Display**: TFT LCD upgrade for enhanced visuals
- **Multiple Sensors**: Humidity, pressure, air quality monitoring
- **Backup Battery**: RTC battery backup for time retention
- **Enclosure Design**: 3D-printed case for professional appearance

### Software Improvements
- **OTA Updates**: Over-the-air firmware updates
- **User Profiles**: Multiple user alarm and preference profiles
- **Advanced Scheduling**: Complex alarm patterns and exceptions
- **Data Logging**: Historical weather and usage data storage

## Complete Flow Summary

### System State Transitions
```
[BOOT] → setupWiFi() → setupWebServer() → [STATE_NORMAL]
   ↓
[STATE_NORMAL] ←→ [STATE_COUNTDOWN] ←→ [STATE_ALARM]
   ↓
[FACTORY_RESET] → ESP.restart()
```

### Key Decision Points in Main Loop

1. **Button Handling (Every 50ms)**
   - Emergency buzzer stop (interrupt-driven)
   - Short press: context-sensitive actions
   - Long press (5s): factory reset

2. **Timer Management (Every Cycle)**
   - Active timer: check completion
   - Finished timer: 5-second completion alarm
   - Timer display updates

3. **State-Specific Operations**
   - **STATE_NORMAL**: Clock display, alarm checking, periodic tasks
   - **STATE_COUNTDOWN**: Timer countdown display
   - **STATE_ALARM**: Blinking alarm display with auto-timeout

4. **Periodic Tasks (Time-Based)**
   - **Every 1s**: Display updates
   - **Every 60s**: Auto LCD mode switch
   - **Every 10min**: Weather data refresh
   - **Every 1hr**: NTP time synchronization

### Critical Dependencies
- **RTC Module**: Primary time source, NTP backup
- **WiFi Connection**: Web server, weather, NTP sync
- **EEPROM/Preferences**: Persistent storage for alarms/config
- **Interrupt System**: Immediate alarm response

### Error Handling Strategies
- **WiFi Failures**: Graceful degradation, retry logic
- **API Failures**: Default values, cached data
- **Storage Errors**: Factory reset option
- **Hardware Issues**: Status indicators, fallback modes

### Performance Optimizations
- **Display Updates**: Change detection to minimize LCD writes
- **Network Operations**: Non-blocking, time-based scheduling
- **Memory Management**: Fixed-size arrays, stack allocation
- **Interrupt Response**: Minimal ISR with flag-based main loop handling

## Troubleshooting Guide

### Common Issues

#### WiFi Connection Problems
1. **Check saved credentials**: Verify WiFi network name and password
2. **Signal strength**: Ensure adequate WiFi signal strength
3. **Network compatibility**: Confirm 2.4GHz network support
4. **Router settings**: Check for MAC filtering or access restrictions

#### Display Issues
1. **LCD initialization**: Verify I2C connections and address
2. **Contrast adjustment**: Check LCD contrast potentiometer
3. **Power supply**: Ensure stable 5V supply for LCD backlight
4. **Cable connections**: Verify SDA/SCL wire integrity

#### Time Synchronization Issues
1. **RTC battery**: Replace DS1307 backup battery if needed
2. **NTP access**: Verify internet connectivity for time sync
3. **Timezone settings**: Confirm GMT+7 configuration
4. **Manual time setting**: Use web interface for manual time correction

#### Weather Data Problems
1. **API key validity**: Verify OpenWeather API key status
2. **City configuration**: Ensure correct city name spelling
3. **Network connectivity**: Check internet access for API calls
4. **Rate limiting**: Verify API usage within free tier limits

### Reset Procedures

#### Soft Reset
- **Web Interface**: Use restart button in web interface
- **Serial Command**: Send restart command via serial monitor

#### WiFi Reset
- **Web Interface**: Use WiFi reset button in configuration
- **Button Sequence**: Long press device button for WiFi reset

#### Factory Reset
- **Button Method**: Hold device button for 5+ seconds
- **Web Interface**: Use factory reset button with confirmation
- **Serial Method**: Send factory reset command via serial

## Version History

### v5.1 Enhanced
- **Improved WiFi Management**: Enhanced reconnection logic
- **Real-time Web Updates**: JavaScript-based status updates
- **Better Error Handling**: Comprehensive error recovery
- **Performance Optimization**: Reduced memory usage and improved responsiveness

### v5.0 Standalone
- **Initial Release**: Complete feature implementation
- **Web Interface**: Full web-based configuration
- **Multi-alarm Support**: Up to 5 simultaneous alarms
- **Weather Integration**: OpenWeather API integration

## License and Credits

This project is developed for educational purposes as part of the IOT102 course curriculum. The code demonstrates practical IoT development concepts including:

- **ESP32 Programming**: Arduino framework usage
- **Web Development**: Responsive HTML/CSS/JavaScript
- **IoT Integration**: Weather API and real-time data
- **Embedded Systems**: Hardware interfacing and control
- **User Experience**: Intuitive interface design

## Support and Maintenance

For technical support, feature requests, or bug reports:
1. **Documentation**: Review this documentation and code comments
2. **Serial Monitor**: Enable debug output for detailed system information
3. **Web Interface**: Use built-in diagnostics and status information
4. **Code Review**: Examine source code for specific implementation details

## Conclusion

The Smart Clock ESP32 represents a comprehensive IoT solution that balances functionality, usability, and educational value. The codebase demonstrates modern embedded systems development practices while maintaining clarity and extensibility for future enhancements.
