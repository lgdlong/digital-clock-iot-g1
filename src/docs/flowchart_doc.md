# Smart Clock ESP32 - Code Flow Documentation

This document describes the program flow of the Smart Clock ESP32 system to assist in creating flowcharts.

## Overview

The Smart Clock operates as a state machine with the following main states:
- `STATE_BOOT`: Initial startup phase
- `STATE_NORMAL`: Normal clock operation
- `STATE_ALARM`: Active alarm state
- `STATE_COUNTDOWN`: Timer countdown display

## 1. System Initialization (setup() function)

### 1.1 Hardware Initialization
```
START
├── Initialize Serial Communication (115200 baud)
├── Initialize GPIO Pins
│   ├── BUTTON_PIN (26) as INPUT_PULLUP
│   ├── LED_PIN (12) as OUTPUT
│   └── BUZZER_PIN (25) as OUTPUT
├── Attach Button Interrupt (buttonInterrupt on FALLING edge)
├── Test LED and Buzzer (200ms ON then OFF)
├── Initialize LCD (I2C 0x27, 16x2)
│   ├── LCD.init()
│   ├── LCD.backlight()
│   └── Set hw.lcdOK = true
├── Initialize RTC (DS1307)
│   ├── IF rtc.begin() SUCCESS
│   │   └── Set hw.rtcOK = true
│   └── ELSE Set hw.rtcOK = false
└── Initialize Temperature Sensor (NTC on GPIO 34)
    ├── Read analogRead(NTC_PIN)
    ├── IF reading valid (0 < value < 4095)
    │   ├── Set hw.tempOK = true
    │   └── Calculate currentTemp using convertAdcToTemperature()
    └── ELSE Set hw.tempOK = false
```

### 1.2 Data Initialization
```
├── Check First Boot
│   ├── Call checkFirstBoot()
│   └── IF first boot → Call clearAllData()
├── Load Configuration Data
│   ├── Call loadConfiguration() (from EEPROM)
│   └── Call loadWeatherConfig() (from Preferences)
├── Initialize System Variables
│   ├── Set lastLCDModeChange = millis()
│   ├── Set weather.dataValid = false
│   ├── Set weather.lastUpdate = 0
│   └── Set weather.errorCount = 0
```

### 1.3 Network and Services Setup
```
├── WiFi Setup
│   ├── Call setupWiFi()
│   ├── IF WiFi connected
│   │   ├── Set hw.wifiOK = true
│   │   └── Call fetchWeatherData() (initial fetch)
│   └── ELSE Set hw.wifiOK = false
├── WiFiManager Setup
│   ├── Set wifiManager.setConfigPortalBlocking(false)
│   └── Start config portal (non-blocking)
├── Web Server Setup
│   └── Call setupWebServer()
├── Initial Display
│   ├── Show "Smart Clock v5.1" and "Starting..."
│   ├── Delay 2000ms
│   └── Set currentState = STATE_NORMAL
```

## 2. Main Loop (loop() function)

The main loop runs continuously and handles multiple tasks in sequence:

### 2.1 WiFi Management (Section A)
```
[A] WiFi Auto-Reconnect
├── Check WiFi Status
├── IF WiFi disconnected
│   ├── Check if 30 seconds passed since last retry
│   ├── IF yes → Attempt WiFi.reconnect()
│   └── Set rtcSynced = false
└── ELSE reset reconnect flags
```

### 2.2 WiFiManager Processing (Section B)
```
[B] WiFiManager Processing
├── Call wifiManager.process() (non-blocking)
└── IF WiFi connected AND config portal active
    └── Stop config portal to free port for web server
```

### 2.3 Web Server Management (Section C)
```
[C] Web Server Lifecycle
├── IF WiFi connected AND web server not started
│   ├── Call setupWebServer()
│   └── Set webServerStarted = true
├── IF WiFi disconnected AND web server started
│   └── Set webServerStarted = false
└── IF web server started
    └── Call server.handleClient()
```

### 2.4 Temperature Reading (Section D)
```
[D] Temperature Sensor (every 5 seconds)
├── Check if 5000ms elapsed since last reading
├── IF yes AND hw.tempOK
│   ├── Read analogRead(NTC_PIN)
│   └── Update currentTemp = convertAdcToTemperature(adcValue)
```

### 2.5 Timer and Alarm Processing (Section E)
```
[E] Timer/Alarm Processing
└── Call handleTimerAlarm()
    ├── Check if timer finished and trigger 5-second alarm
    ├── Handle timer alarm buzzer/LED control
    └── Manage timeout states
```

### 2.6 Button Processing (Section F)
```
[F] Button Handling (every 50ms)
├── Check if 50ms elapsed since last check
└── IF yes → Call handleButton()
    ├── Check for interrupt-requested buzzer stop
    ├── Handle immediate buzzer shutoff
    ├── Process debounced button press
    ├── Detect short press vs long press (5+ seconds)
    ├── Short press: Stop alarms/timers OR switch LCD mode
    └── Long press (5s+): Factory reset
```

### 2.7 NTP Synchronization (Section G)
```
[G] RTC-NTP Sync
├── IF WiFi connected AND not yet synced
│   ├── Call syncTimeWithNTP()
│   └── Set rtcSynced = true
```

### 2.8 Status Updates (Section H)
```
[H] Hardware Status Update
└── Update hw.wifiOK = (WiFi.status() == WL_CONNECTED)
```

### 2.9 Weather Data Fetching (Section I)
```
[I] Weather Data Fetch
├── IF WiFi connected
└── Call fetchWeatherData()
    ├── Check if update interval elapsed
    ├── Make HTTP request to OpenWeather API
    ├── Parse JSON response
    └── Update weather struct with new data
```

### 2.10 State Machine Processing (Section J)
```
[J] State Machine
├── SWITCH currentState
├── CASE STATE_NORMAL:
│   ├── Call displayClock()
│   │   ├── Auto-switch LCD mode every 60 seconds
│   │   ├── Mode 0: Time + Temperature + Status
│   │   └── Mode 1: Weather information
│   └── Call checkAlarms()
│       ├── Get current time from RTC
│       ├── Check each alarm for time match
│       ├── Check day-of-week conditions
│       └── IF match → Call triggerAlarm(index)
├── CASE STATE_COUNTDOWN:
│   ├── Call displayCountdown()
│   │   ├── Calculate remaining time
│   │   ├── Display MM:SS format
│   │   └── IF timer finished → trigger 5-second alarm
│   └── IF timer not active → Set currentState = STATE_NORMAL
└── CASE STATE_ALARM:
    └── Call updateAlarmDisplay()
        ├── Blink alarm display every 500ms
        ├── Control buzzer and LED patterns
        ├── Display alarm information
        └── Handle auto-timeout after 5 minutes
```

### 2.11 CPU Load Management (Section Z)
```
[Z] CPU Load Control
└── delay(100) // Prevent CPU overload
```

## 3. Key Functions Flow

### 3.1 Button Interrupt Handler
```
buttonInterrupt() [IRAM_ATTR]
├── Check debounce (50ms since last interrupt)
├── IF valid press
│   ├── Update lastInterruptTime
│   ├── Check if buzzer/LED currently active
│   └── IF active → Set buzzerStopRequested = true
```

### 3.2 handleButton() Function
```
handleButton()
├── Check buzzerStopRequested flag
├── IF flag set
│   ├── Stop alarms/timers immediately
│   └── Reset flag and return
├── Read current button state
├── IF button pressed AND buzzer active
│   └── Stop buzzer/LED immediately
├── Handle debounced button state changes
├── Measure press duration
├── IF long press (≥5000ms) → factoryReset()
└── IF short press → Context-sensitive action
    ├── IF alarm active → Stop alarm
    ├── IF timer alarm active → Stop timer alarm
    └── IF normal state → Switch LCD display mode
```

### 3.3 displayClock() Function
```
displayClock()
├── Check if 60 seconds elapsed → Auto-switch LCD mode
├── Update only every 1000ms (1 second)
├── Get current time from RTC
├── IF lcdDisplayMode == 0 (Clock Mode)
│   ├── Format: "HH:MM:SS TT.TC"
│   └── Format: "DD/MM/YY STATUS"
└── IF lcdDisplayMode == 1 (Weather Mode)
    ├── IF weather.dataValid
    │   ├── Format: "CITY: TT.TC"
    │   └── Format: "Humidity: XX%"
    └── ELSE Show "Weather" / "No data..."
```

### 3.4 checkAlarms() Function
```
checkAlarms()
├── Get current DateTime from RTC
├── Calculate current day of week
├── FOR each alarm (i = 0 to alarmCount-1)
│   ├── Check IF alarm enabled
│   ├── Check IF hour matches
│   ├── Check IF minute matches
│   ├── Check IF day-of-week enabled
│   └── IF all conditions met
│       └── Call triggerAlarm(i)
```

### 3.5 handleTimerAlarm() Function
```
handleTimerAlarm()
├── IF timer.alarmTriggered (5-second alarm active)
│   ├── Check if 5 seconds elapsed since alarm start
│   ├── Control buzzer/LED blinking pattern
│   └── IF timeout reached → Stop alarm
├── IF timer.finished AND not alarmTriggered
│   └── Trigger 5-second completion alarm
└── Handle alarm timeout (5 minutes maximum)
```

### 3.6 fetchWeatherData() Function
```
fetchWeatherData()
├── Check if update interval elapsed (10 seconds)
├── IF WiFi not connected → return
├── IF too early for update → return
├── Create HTTP client
├── Build OpenWeather API URL with city and API key
├── Make HTTP GET request
├── IF HTTP response == 200
│   ├── Parse JSON response
│   ├── Extract temperature, humidity, description, city
│   ├── Set weather.dataValid = true
│   ├── Reset weather.errorCount = 0
│   └── Update weather.lastUpdate = millis()
└── ELSE
    ├── Increment weather.errorCount
    ├── IF error count > threshold → Set weather.dataValid = false
    └── Log error details
```

## 4. State Transitions

### 4.1 State Transition Diagram
```
STATE_BOOT
    ↓ (setup complete)
STATE_NORMAL
    ↓ (alarm triggered)
STATE_ALARM ←→ (button press / timeout)
    ↓ (alarm stopped)
STATE_NORMAL
    ↓ (timer started via web)
STATE_COUNTDOWN
    ↓ (timer finished)
STATE_NORMAL (with 5-second alarm)
```

### 4.2 State Transition Conditions
- **BOOT → NORMAL**: Setup completion
- **NORMAL → ALARM**: checkAlarms() finds matching alarm
- **NORMAL → COUNTDOWN**: Timer started via web interface
- **ALARM → NORMAL**: Button press OR 5-minute timeout
- **COUNTDOWN → NORMAL**: Timer finished OR timer stopped
- **ANY STATE → NORMAL**: Factory reset (5-second button hold)

## 5. Critical Timing Considerations

- **Button debounce**: 50ms minimum between readings
- **LCD updates**: Maximum 1 update per second
- **Temperature reading**: Every 5 seconds
- **LCD mode auto-switch**: Every 60 seconds
- **Weather data fetch**: Every 10 seconds (when WiFi available)
- **WiFi reconnect attempts**: Every 30 seconds when disconnected
- **Alarm timeout**: 5 minutes maximum
- **Timer completion alarm**: 5 seconds duration
- **Main loop cycle**: ~100ms (with delay(100))

## 6. Interrupt and Real-time Handling

### 6.1 Hardware Interrupts
- **Button GPIO 26**: FALLING edge triggers buttonInterrupt()
- **Purpose**: Immediate buzzer shutoff capability
- **ISR Safety**: Minimal processing, sets flag for main loop

### 6.2 Non-blocking Operations
- **WiFiManager**: Non-blocking config portal
- **Web Server**: Non-blocking client handling
- **Weather API**: Asynchronous HTTP requests with timeout
- **All timing**: Based on millis() for non-blocking delays

This flowchart documentation provides the complete program flow for creating detailed flowchart diagrams of the Smart Clock ESP32 system.
