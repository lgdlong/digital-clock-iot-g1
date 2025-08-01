#include <WiFi.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <time.h> // Include time.h for NTP

// ==========================================
// FORWARD DECLARATIONS
// ==========================================

// Display functions
void switchLcdDisplayMode();
void updateLCDContent(String line1, String line2);
void displayClock();
void displayCountdown();

// Alarm & Timer functions
void triggerAlarm(int index);
void stopAlarm();
void updateAlarmDisplay();
void checkAlarms();
void handleTimerAlarm();

// Weather functions
void fetchWeatherData();

// Utility functions
bool checkFirstBoot();
void clearAllData();
float convertAdcToTemperature(int adcValue);
void factoryReset();

// Configuration functions
void loadConfiguration();
void saveConfiguration();
void saveAlarms();
void loadWeatherConfig();
void saveWeatherConfig();

// Network functions
void setupWiFi();
void setupWebServer();

// Button handling
void handleButton();

// Interrupt handling for immediate buzzer shutoff
void IRAM_ATTR buttonInterrupt();

// Web interface
String generateWebInterface();

// Synchronize RTC with NTP time if WiFi is connected
void syncTimeWithNTP();

// ==========================================
// SMART CLOCK v5.0 - STANDALONE VERSION
// ==========================================

// Pin Definitions
#define LED_PIN 12
#define BUZZER_PIN 25
#define BUTTON_PIN 26
#define NTC_PIN 34

// Hardware Objects
LiquidCrystal_I2C LCD(0x27, 16, 2);
RTC_DS1307 rtc;
WebServer server(80);
WiFiManager wifiManager;
Preferences preferences;

// System States
enum SystemState
{
  STATE_BOOT,
  STATE_NORMAL,
  STATE_ALARM,
  STATE_COUNTDOWN
};

SystemState currentState = STATE_BOOT;
unsigned long stateStartTime = 0;

// Configuration
struct DeviceConfig
{
  char deviceName[64] = "Smart Clock v5.0";
  char hotspotSSID[32] = "IOT_NHOM1";
  char hotspotPassword[32] = "12345678";
  bool configValid = false;
} config;

// Hardware Status
struct HardwareStatus
{
  bool lcdOK = false;
  bool rtcOK = false;
  bool wifiOK = false;
  bool tempOK = false;
  bool buzzerOK = false;
  bool ledOK = false;
  String lastError = "";
} hw;

// Buzzer control variables for interrupt handling
volatile bool buzzerStopRequested = false;
volatile unsigned long lastInterruptTime = 0;

// Temperature Data
float currentTemp = 25.0;

// Alarm System
struct Alarm
{
  int hour = -1;
  int minute = -1;
  bool enabled = false;
  bool daysOfWeek[7] = {false};
  char label[32] = "";
};

#define MAX_ALARMS 5
Alarm alarms[MAX_ALARMS];
int alarmCount = 0;
bool alarmActive = false;
int activeAlarmIndex = -1;

// Countdown Timer
struct CountdownTimer
{
  unsigned long duration = 0; // in seconds
  unsigned long startTime = 0;
  bool active = false;
  bool finished = false;
  char label[32] = "Timer";
  // Add alarm state for 5-second alert
  bool alarmTriggered = false;
  unsigned long alarmStartTime = 0;
} timer;

// Weather Configuration
struct WeatherConfig
{
  char apiKey[64] = "";
  char cityName[32] = "Thu Duc";
  char countryCode[8] = "VN";
  bool enabled = false;
  int updateInterval = 600; // seconds
} weatherConfig;

// Weather Data
struct WeatherData
{
  float temperature = 0.0;
  int humidity = 0;
  String description = "N/A";
  String city = "Thu Duc";
  bool dataValid = false;
  unsigned long lastUpdate = 0;
  int errorCount = 0;
} weather;

// City Options for Vietnam
const char *vietnamCities[][2] = {
    {"Thu Duc", "Thu Duc"},
    {"Ho Chi Minh City", "TP. Hồ Chí Minh"},
    {"Hanoi", "Hà Nội"},
    {"Da Nang", "Đà Nẵng"},
    {"Can Tho", "Cần Thơ"},
    {"Hai Phong", "Hải Phòng"},
    {"Bien Hoa", "Biên Hòa"},
    {"Nha Trang", "Nha Trang"},
    {"Vung Tau", "Vũng Tàu"},
    {"Hue", "Huế"}};

// LCD Display State
String currentLCDLine1 = "";
String currentLCDLine2 = "";
int lcdDisplayMode = 0; // 0: Clock/Temp, 1: Weather
unsigned long lastLCDModeChange = 0;

// Constants
#define EEPROM_SIZE 1024
#define CONFIG_ADDR 0
#define ALARM_ADDR 400
#define TIMER_ADDR 800
bool rtcSynced = false; // True if RTC has been synced with NTP
bool webServerStarted = false;

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

bool checkFirstBoot()
{
  preferences.begin("clock", false);
  bool firstBoot = preferences.getBool("firstBoot", true);
  if (firstBoot)
  {
    preferences.putBool("firstBoot", false);
    preferences.end();
    return true;
  }
  preferences.end();
  return false;
}

void clearAllData()
{
  Serial.println("Clearing all stored data...");

  // Clear EEPROM
  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();

  // Clear preferences
  preferences.begin("clock", false);
  preferences.clear();
  preferences.end();

  // Reset variables
  alarmCount = 0;
  timer.active = false;
  timer.duration = 0;
  strcpy(config.deviceName, "SmartClock-v5");
  strcpy(config.hotspotSSID, "SmartClock-v5");
  strcpy(config.hotspotPassword, "smartclock123");

  Serial.println("All data cleared!");
}

float convertAdcToTemperature(int adcValue)
{
  // Convert ADC reading to temperature for LM35 sensor
  // LM35: 10mV/°C, linear output
  if (adcValue == 0)
    return 25.0; // Default temperature

  // Convert ADC to voltage (ESP32: 12-bit ADC, 3.3V reference)
  float voltage = (adcValue / 4095.0) * 3.3;

  // LM35: 10mV per degree Celsius
  // Temperature = Voltage / 0.01V per °C
  float tempC = voltage / 0.01;

  // Reasonable range check for LM35 (-55°C to +150°C, but typically 2°C to 100°C)
  if (tempC < -10 || tempC > 100)
  {
    Serial.println("Warning: Temperature out of range: " + String(tempC) + "°C");
    return 25.0; // Return default if out of range
  }

  return tempC;
}

void factoryReset()
{
  Serial.println("=== FACTORY RESET ===");
  preferences.clear();
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  wifiManager.resetSettings();
  ESP.restart();
}

// ==========================================
// HARDWARE FUNCTIONS
// ==========================================
void initializeHardware()
{
  Serial.println("=== HARDWARE INIT v5.0 ===");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Test hardware
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  hw.ledOK = true;

  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  hw.buzzerOK = true;

  Wire.begin();
  LCD.init();
  LCD.backlight();
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Smart Clock v5.0");
  LCD.setCursor(0, 1);
  LCD.print("Standalone Mode");
  hw.lcdOK = true;

  if (rtc.begin())
  {
    hw.rtcOK = true;
    if (!rtc.isrunning())
    {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }
  else
  {
    hw.rtcOK = false;
    hw.lastError = "RTC Failed";
  }

  int adcValue = analogRead(NTC_PIN);
  if (adcValue > 0 && adcValue < 4095)
  {
    hw.tempOK = true;
    currentTemp = convertAdcToTemperature(adcValue);
  }
  else
  {
    hw.tempOK = false;
  }

  EEPROM.begin(EEPROM_SIZE);
  preferences.begin("smartclock", false);

  Serial.println("=== HARDWARE INIT COMPLETE ===");
}

void readTemperature()
{
  if (!hw.tempOK)
    return;
  int adcValue = analogRead(NTC_PIN);
  currentTemp = convertAdcToTemperature(adcValue);
}

// ==========================================
// DISPLAY FUNCTIONS - ENHANCED
// ==========================================
// Switch LCD display mode
void switchLcdDisplayMode()
{
  lcdDisplayMode = (lcdDisplayMode + 1) % 2; // or % N if you have more modes
  lastLCDModeChange = millis();
  displayClock();
}

void updateLCDContent(String line1, String line2)
{
  if (line1 != currentLCDLine1 || line2 != currentLCDLine2)
  {
    currentLCDLine1 = line1;
    currentLCDLine2 = line2;

    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print(line1.substring(0, 16));
    LCD.setCursor(0, 1);
    LCD.print(line2.substring(0, 16));
  }
}

void displayClock()
{
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  // Switch display mode every 60 seconds
  if (now - lastLCDModeChange > 60000)
  {
    lcdDisplayMode = (lcdDisplayMode + 1) % 2;
    lastLCDModeChange = now;
  }

  if (now - lastUpdate < 1000)
    return;
  lastUpdate = now;

  DateTime rtcNow = rtc.now();

  if (lcdDisplayMode == 0)
  {
    // Mode 0: Clock + Temperature
    char line1[17];
    snprintf(line1, sizeof(line1), "%02d:%02d:%02d %4.1fC",
             rtcNow.hour(), rtcNow.minute(), rtcNow.second(), currentTemp);

    char line2[17];
    String status = hw.wifiOK ? "WIFI" : "DISC";
    if (alarmCount > 0)
      status = "A" + String(alarmCount);
    if (timer.active)
      status = "TIMER";

    snprintf(line2, sizeof(line2), "%02d/%02d/%02d %s",
             rtcNow.day(), rtcNow.month(), rtcNow.year() % 100, status.c_str());

    updateLCDContent(String(line1), String(line2));
  }
  else
  {
    // Mode 1: Weather Information
    char line1[17];
    char line2[17];

    if (weather.dataValid)
    {
      snprintf(line1, sizeof(line1), "%s: %.1fC",
               weather.city.substring(0, 8).c_str(), weather.temperature);
      snprintf(line2, sizeof(line2), "%s: %d%%",
               "Humidity", weather.humidity);
    }
    else
    {
      strcpy(line1, "Weather");
      strcpy(line2, "No data...");
    }

    updateLCDContent(String(line1), String(line2));
  }
}

void displayCountdown()
{
  if (!timer.active)
    return;

  unsigned long elapsed = (millis() - timer.startTime) / 1000;
  if (elapsed >= timer.duration)
  {
    // Timer finished - trigger 5-second alarm
    timer.active = false;
    timer.finished = true;
    timer.alarmTriggered = true;
    timer.alarmStartTime = millis();
    Serial.println("=== COUNTDOWN FINISHED - 5 SECOND ALARM ===");
    return;
  }

  unsigned long remaining = timer.duration - elapsed;
  int minutes = remaining / 60;
  int seconds = remaining % 60;

  char line1[17];
  snprintf(line1, sizeof(line1), "TIMER: %02d:%02d", minutes, seconds);

  updateLCDContent(String(line1), String(timer.label));
}

// ==========================================
// ENHANCED ALARM & TIMER SYSTEM
// ==========================================

void triggerAlarm(int index)
{
  activeAlarmIndex = index;
  alarmActive = true;
  timer.finished = false; // Reset timer finished flag
  currentState = STATE_ALARM;
  stateStartTime = millis();
}

void stopAlarm()
{
  alarmActive = false;
  timer.active = false;
  timer.finished = false;
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  currentState = STATE_NORMAL;
  activeAlarmIndex = -1;
}

void updateAlarmDisplay()
{
  static unsigned long lastBlink = 0;
  static bool blinkState = false;

  if (millis() - lastBlink > 500)
  {
    lastBlink = millis();
    blinkState = !blinkState;

    if (blinkState)
    {
      String label = "WAKE UP!";
      if (activeAlarmIndex >= 0)
      {
        label = alarms[activeAlarmIndex].label;
      }
      else if (timer.finished)
      {
        label = timer.label;
      }

      updateLCDContent("*** ALARM ***", label);
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
    }
    else
    {
      LCD.clear();
      currentLCDLine1 = "";
      currentLCDLine2 = "";
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }
  }

  // Auto-stop after 5 minutes (300 seconds)
  if (millis() - stateStartTime > 5 * 60 * 1000)
  {
    stopAlarm();
  }
}

// ==========================================
// BUTTON HANDLING
// ==========================================
void handleButton()
{
  static unsigned long lastPress = 0;
  static bool lastState = HIGH;
  static unsigned long pressStartTime = 0;

  // Check if interrupt handler requested buzzer stop
  if (buzzerStopRequested)
  {
    buzzerStopRequested = false;

    // Handle the alarm state change
    if (currentState == STATE_ALARM || alarmActive)
    {
      stopAlarm();
      Serial.println("Alarm stopped by interrupt");
    }

    // Stop timer alarm if active
    if (timer.alarmTriggered)
    {
      timer.alarmTriggered = false;
      timer.finished = false;
      Serial.println("Timer alarm stopped by interrupt");
    }

    return; // Exit early if interrupt handled the buzzer
  }

  bool buttonState = digitalRead(BUTTON_PIN);

  // Immediate buzzer shutoff check (non-interrupt backup)
  if (buttonState == LOW)
  {
    // If buzzer is currently ON, turn it off immediately
    if (digitalRead(BUZZER_PIN) == HIGH)
    {
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);

      if (currentState == STATE_ALARM || alarmActive)
      {
        stopAlarm();
        Serial.println("Alarm stopped by button press");
        return;
      }

      if (timer.alarmTriggered)
      {
        timer.alarmTriggered = false;
        timer.finished = false;
        Serial.println("Timer alarm stopped by button press");
        return;
      }
    }
  }

  // Standard debounced button handling
  if (buttonState != lastState && millis() - lastPress > 50)
  {
    lastPress = millis();

    if (buttonState == LOW)
    {
      pressStartTime = millis();
    }
    else
    {
      unsigned long pressDuration = millis() - pressStartTime;

      if (pressDuration >= 5000)
      {
        factoryReset();
      }
      else
      {
        // Ưu tiên xử lý báo thức và timer nếu đang active
        if (currentState == STATE_ALARM)
        {
          stopAlarm();
        }
        else if (timer.alarmTriggered)
        {
          timer.alarmTriggered = false;
          timer.finished = false;
          digitalWrite(BUZZER_PIN, LOW);
          digitalWrite(LED_PIN, LOW);
          Serial.println("Timer alarm stopped by button");
        }
        // Nếu ở trạng thái bình thường, không có alarm/timer thì mới chuyển LCD
        else if (currentState == STATE_NORMAL)
        {
          switchLcdDisplayMode();
          Serial.println("LCD display mode switched");
        }
      }
    }
  }

  lastState = buttonState;
}

// ==========================================
// INTERRUPT HANDLER FOR IMMEDIATE BUZZER CONTROL
// ==========================================

// Interrupt Service Routine for button press
void IRAM_ATTR buttonInterrupt()
{
  unsigned long currentTime = millis();

  // Simple debouncing - ignore interrupts within 50ms of the last one
  if (currentTime - lastInterruptTime > 50)
  {
    lastInterruptTime = currentTime;

    // Check if button is actually pressed (LOW due to INPUT_PULLUP)
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      // Immediately turn off buzzer if it's currently sounding
      if (digitalRead(BUZZER_PIN) == HIGH)
      {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        buzzerStopRequested = true;
        Serial.println("Buzzer stopped by interrupt!");
      }
    }
  }
}

// ==========================================
// WEB INTERFACE
// ==========================================

String generateWebInterface()
{
  DateTime now = rtc.now();

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Smart Clock v5.1 - Enhanced</title>";
  html += "<style>";

  // Enhanced CSS
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: 'Segoe UI', 'Arial', sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; min-height: 100vh; }";
  html += ".container { max-width: 1200px; margin: 0 auto; padding: 20px; }";

  // Header styles
  html += ".header { text-align: center; margin-bottom: 40px; }";
  html += ".header h1 { font-size: 3rem; font-weight: 700; color: #fff; margin-bottom: 10px; text-shadow: 0 4px 8px rgba(0,0,0,0.3); }";
  html += ".clock-display { font-size: 4rem; font-weight: bold; color: #FFD700; margin: 20px 0; text-shadow: 0 4px 8px rgba(0,0,0,0.5); }";
  html += ".subtitle { font-size: 1.2rem; opacity: 0.9; margin-bottom: 20px; }";

  // Card styles
  html += ".card { background: rgba(255,255,255,0.15); backdrop-filter: blur(10px); border: 1px solid rgba(255,255,255,0.2); border-radius: 20px; padding: 25px; margin: 20px 0; box-shadow: 0 8px 32px rgba(0,0,0,0.1); transition: transform 0.3s ease; }";
  html += ".card:hover { transform: translateY(-5px); }";
  html += ".card h3 { color: #FFD700; margin-bottom: 20px; font-size: 1.4rem; border-bottom: 2px solid rgba(255,215,0,0.3); padding-bottom: 10px; }";

  // Grid layout
  html += ".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); gap: 25px; }";
  html += ".grid-2 { grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); }";

  // Form styles
  html += ".form-group { margin: 15px 0; }";
  html += ".form-group label { display: block; margin-bottom: 8px; color: #fff; font-weight: 500; }";
  html += ".form-group input, .form-group select { width: 100%; padding: 12px 16px; background: rgba(255,255,255,0.1); border: 2px solid rgba(255,255,255,0.2); border-radius: 10px; color: white; font-size: 1rem; transition: all 0.3s ease; }";
  html += ".form-group input:focus, .form-group select:focus { outline: none; border-color: #FFD700; background: rgba(255,255,255,0.2); }";
  html += ".form-group input::placeholder { color: rgba(255,255,255,0.7); }";

  // Button styles
  html += ".btn { background: linear-gradient(135deg, #FFD700, #FFA500); color: #000; border: none; padding: 12px 24px; border-radius: 10px; cursor: pointer; margin: 8px 4px; font-weight: 600; font-size: 1rem; transition: all 0.3s ease; text-transform: uppercase; letter-spacing: 0.5px; }";
  html += ".btn:hover { background: linear-gradient(135deg, #FFA500, #FF8C00); transform: translateY(-2px); box-shadow: 0 4px 12px rgba(255,215,0,0.4); }";
  html += ".btn-danger { background: linear-gradient(135deg, #ff4757, #ff3838); color: white; }";
  html += ".btn-danger:hover { background: linear-gradient(135deg, #ff3838, #ff2e2e); }";
  html += ".btn-success { background: linear-gradient(135deg, #2ed573, #1abc9c); color: white; }";
  html += ".btn-success:hover { background: linear-gradient(135deg, #1abc9c, #16a085); }";

  // Status styles
  html += ".status { display: inline-flex; align-items: center; padding: 8px 16px; border-radius: 20px; margin: 5px; font-weight: 500; font-size: 0.9rem; }";
  html += ".status.ok { background: rgba(46, 213, 115, 0.2); border: 1px solid #2ed573; color: #2ed573; }";
  html += ".status.error { background: rgba(255, 71, 87, 0.2); border: 1px solid #ff4757; color: #ff4757; }";
  html += ".status::before { content: '●'; margin-right: 8px; font-size: 1.2rem; }";

  // LCD and special displays
  html += ".lcd { background: #000; color: #00ff00; font-family: 'Courier New', monospace; padding: 20px; border-radius: 10px; margin: 15px 0; border: 3px solid #333; font-size: 1.1rem; letter-spacing: 2px; }";
  html += ".timer-display { font-size: 3rem; color: #FFD700; text-align: center; margin: 20px 0; font-weight: bold; text-shadow: 0 4px 8px rgba(0,0,0,0.5); }";
  html += ".weather-info { text-align: center; padding: 20px; }";
  html += ".weather-temp { font-size: 2.5rem; color: #FFD700; font-weight: bold; }";

  // Alarm list styles
  html += ".alarm-item { background: rgba(255,255,255,0.1); border: 1px solid rgba(255,255,255,0.2); border-radius: 12px; padding: 15px; margin: 10px 0; display: flex; justify-content: space-between; align-items: center; }";
  html += ".alarm-time { font-size: 1.3rem; font-weight: bold; color: #FFD700; }";
  html += ".alarm-label { flex: 1; margin: 0 15px; }";
  html += ".alarm-days { font-size: 0.9rem; opacity: 0.8; margin-top: 5px; }";

  // Checkbox styles
  html += ".checkbox-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(120px, 1fr)); gap: 10px; margin: 10px 0; }";
  html += ".checkbox-item { display: flex; align-items: center; }";
  html += ".checkbox-item input[type='checkbox'] { margin-right: 8px; transform: scale(1.2); }";

  // Responsive
  html += "@media (max-width: 768px) { .container { padding: 15px; } .header h1 { font-size: 2rem; } .clock-display { font-size: 2.5rem; } .grid { grid-template-columns: 1fr; } .alarm-item { flex-direction: column; align-items: flex-start; } }";

  html += "</style></head><body>";

  html += "<div class='container'>";

  // Enhanced Header
  html += "<div class='header'>";
  html += "<h1>🕐 Smart Clock v5.1</h1>";
  html += "<div class='clock-display'>" + String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute()) + "</div>";
  html += "<div class='subtitle'>📅 " + String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + " | 🌡️ " + String(currentTemp, 1) + "°C</div>";
  html += "</div>";

  // LCD Display
  html += "<div class='card'>";
  html += "<h3>📺 Màn hình LCD (Chế độ " + String(lcdDisplayMode == 0 ? "Đồng hồ" : "Thời tiết") + ")</h3>";
  html += "<div class='lcd'>";
  html += "Dòng 1: " + currentLCDLine1 + "<br>";
  html += "Dòng 2: " + currentLCDLine2;
  html += "</div>";
  html += "<small style='opacity: 0.8;'>Tự động chuyển đổi mỗi 60 giây</small>";
  html += "</div>";

  html += "<div class='grid'>";

  // Enhanced Hardware Status
  html += "<div class='card'>";
  html += "<h3>⚙️ Phần cứng</h3>";
  html += "<div style='display: flex; flex-wrap: wrap; gap: 10px;'>";
  html += "<div class='status " + String(hw.lcdOK ? "ok'>📺 LCD" : "error'>📺 LCD") + "</div>";
  html += "<div class='status " + String(hw.rtcOK ? "ok'>🕐 RTC" : "error'>🕐 RTC") + "</div>";
  html += "<div class='status " + String(hw.wifiOK ? "ok'>📶 WiFi" : "error'>📶 WiFi") + "</div>";
  html += "<div class='status " + String(hw.tempOK ? "ok'>🌡️ Nhiệt độ" : "error'>🌡️ Nhiệt độ") + "</div>";
  html += "<div class='status " + String(hw.buzzerOK ? "ok'>🔊 Loa" : "error'>🔊 Loa") + "</div>";
  html += "<div class='status " + String(hw.ledOK ? "ok'>💡 LED" : "error'>💡 LED") + "</div>";
  html += "</div>";
  html += "</div>";

  // Weather Information
  html += "<div class='card'>";
  html += "<h3>🌤️ Thông tin thời tiết</h3>";
  html += "<div class='weather-info'>";
  if (weather.dataValid)
  {
    html += "<div class='weather-temp'>" + String(weather.temperature, 1) + "°C</div>";
    html += "<div>" + weather.description + "</div>";
    html += "<div>Độ ẩm: " + String(weather.humidity) + "%</div>";
    html += "<div>📍 " + weather.city + "</div>";
    if (weather.errorCount > 0)
    {
      html += "<div style='color: #ff4757; font-size: 0.9rem; margin-top: 10px;'>⚠️ Lỗi API: " + String(weather.errorCount) + "</div>";
    }
  }
  else
  {
    html += "<div style='opacity: 0.6;'>Chưa có dữ liệu thời tiết</div>";
    html += "<div style='font-size: 0.8rem; margin-top: 10px;'>💡 Cấu hình API key bên dưới</div>";
  }
  html += "</div>";
  html += "</div>";

  // Weather Configuration
  html += "<div class='card'>";
  html += "<h3>⚙️ Cấu hình thời tiết</h3>";
  html += "<form action='/weather-config' method='POST'>";
  html += "<div class='form-group'>";
  html += "<label>🔑 OpenWeather API Key:</label>";
  html += "<input type='text' name='api_key' value='" + String(weatherConfig.apiKey) + "' placeholder='Nhập API key từ openweathermap.org' maxlength='63'>";
  html += "<small style='opacity: 0.8; font-size: 0.8rem;'>💡 Đăng ký miễn phí tại <a href='https://openweathermap.org/api' target='_blank' style='color: #FFD700;'>openweathermap.org</a></small>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label>🏙️ Chọn thành phố:</label>";
  html += "<select name='city_name' style='width: 100%; padding: 12px; background: rgba(255,255,255,0.1); border: 2px solid rgba(255,255,255,0.2); border-radius: 10px; color: white;'>";

  // Add city options
  for (int i = 0; i < 10; i++)
  {
    bool selected = (strcmp(weatherConfig.cityName, vietnamCities[i][0]) == 0);
    html += "<option value='" + String(vietnamCities[i][0]) + "'" + (selected ? " selected" : "") + ">" + String(vietnamCities[i][1]) + "</option>";
  }

  html += "</select>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label>🔄 Cập nhật mỗi (phút):</label>";
  html += "<input type='number' name='update_interval' value='" + String(weatherConfig.updateInterval / 60) + "' min='5' max='60' placeholder='10'>";
  html += "</div>";
  html += "<div class='form-group' style='display: flex; align-items: center; gap: 10px;'>";
  html += "<input type='checkbox' name='enabled' id='weather_enabled' " + String(weatherConfig.enabled ? "checked" : "") + " style='transform: scale(1.2);'>";
  html += "<label for='weather_enabled'>🌤️ Bật thời tiết thực</label>";
  html += "</div>";
  html += "<div style='margin: 10px 0; font-size: 0.85rem; opacity: 0.8;'>";
  html += "💡 <strong>Ghi chú:</strong> API key phải được cấu hình và checkbox này phải được tích để lấy dữ liệu thời tiết thật";
  html += "</div>";
  html += "<button type='submit' class='btn btn-success'>💾 Lưu cấu hình</button>";
  html += "</form>";
  html += "</div>";

  html += "</div>"; // End first grid

  // Enhanced Alarm Management
  html += "<div class='card'>";
  html += "<h3>⏰ Quản lý báo thức thông minh</h3>";
  html += "<form action='/set-alarm' method='POST' style='margin-bottom: 30px;'>";
  html += "<div class='grid grid-2'>";
  html += "<div class='form-group'>";
  html += "<label>⏰ Giờ báo thức:</label>";
  html += "<input type='number' name='hour' min='0' max='23' placeholder='VD: 7' required>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label>⏱️ Phút báo thức:</label>";
  html += "<input type='number' name='minute' min='0' max='59' placeholder='VD: 30' required>";
  html += "</div>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label>🏷️ Nhãn báo thức:</label>";
  html += "<input type='text' name='label' placeholder='VD: Thức dậy đi làm' maxlength='30'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label>📅 Chọn ngày trong tuần:</label>";
  html += "<div class='checkbox-grid'>";
  String days[] = {"Chủ nhật", "Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7"};
  for (int i = 0; i < 7; i++)
  {
    html += "<div class='checkbox-item'>";
    html += "<input type='checkbox' name='day" + String(i) + "' id='day" + String(i) + "'>";
    html += "<label for='day" + String(i) + "'>" + days[i] + "</label>";
    html += "</div>";
  }
  html += "</div>";
  html += "</div>";
  html += "<button type='submit' class='btn btn-success'>➕ Thêm báo thức</button>";
  html += "</form>";

  // Display existing alarms
  html += "<h4 style='color: #FFD700; margin: 20px 0 15px 0;'>📋 Danh sách báo thức (" + String(alarmCount) + ")</h4>";
  if (alarmCount > 0)
  {
    for (int i = 0; i < alarmCount; i++)
    {
      html += "<div class='alarm-item'>";
      html += "<div>";
      html += "<div class='alarm-time'>🕐 " + String(alarms[i].hour) + ":" + (alarms[i].minute < 10 ? "0" : "") + String(alarms[i].minute) + "</div>";
      html += "<div class='alarm-label'>📝 " + String(alarms[i].label) + "</div>";

      // Show active days
      String activeDays = "📅 ";
      bool hasActiveDays = false;
      for (int j = 0; j < 7; j++)
      {
        if (alarms[i].daysOfWeek[j])
        {
          if (hasActiveDays)
            activeDays += ", ";
          activeDays += days[j];
          hasActiveDays = true;
        }
      }
      if (!hasActiveDays)
        activeDays += "Không lặp lại";
      html += "<div class='alarm-days'>" + activeDays + "</div>";
      html += "</div>";
      html += "<button onclick=\"deleteAlarm(" + String(i) + ")\" class='btn btn-danger'>🗑️ Xóa</button>";
      html += "</div>";
    }
  }
  else
  {
    html += "<div style='text-align: center; opacity: 0.6; padding: 20px;'>Chưa có báo thức nào</div>";
  }
  html += "</div>";

  // Enhanced Countdown Timer
  html += "<div class='card'>";
  html += "<h3>⏱️ Đồng hồ đếm ngược</h3>";
  if (timer.active)
  {
    unsigned long remaining = timer.duration - ((millis() - timer.startTime) / 1000);
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    html += "<div class='timer-display'>⏱️ " + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds) + "</div>";
    html += "<div style='text-align: center; font-size: 1.2rem; margin: 15px 0;'>📝 " + String(timer.label) + "</div>";
    html += "<div style='text-align: center;'>";
    html += "<button onclick=\"stopTimer()\" class='btn btn-danger'>⏹️ Dừng đếm ngược</button>";
    html += "</div>";
  }
  else
  {
    html += "<form action='/set-timer' method='POST'>";
    html += "<div class='form-group'>";
    html += "<label>⏰ Số phút đếm ngược:</label>";
    html += "<input type='number' name='minutes' min='1' max='999' placeholder='VD: 25 (Pomodoro)' required>";
    html += "</div>";
    html += "<div class='form-group'>";
    html += "<label>🏷️ Nhãn đếm ngược:</label>";
    html += "<input type='text' name='label' placeholder='VD: Nấu cơm, Họp online' maxlength='30'>";
    html += "</div>";
    html += "<button type='submit' class='btn btn-success'>▶️ Bắt đầu đếm ngược</button>";
    html += "</form>";
  }
  html += "</div>";

  html += "<div class='grid'>";

  // WiFi Configuration
  html += "<div class='card'>";
  html += "<h3>📶 Cấu hình mạng WiFi</h3>";
  html += "<div style='margin: 15px 0;'>";
  html += "<div>🌐 SSID hiện tại: <strong>" + String(WiFi.SSID()) + "</strong></div>";
  html += "<div>📍 Địa chỉ IP: <strong>" + WiFi.localIP().toString() + "</strong></div>";
  html += "<div>📡 Hotspot ESP32: <strong>" + String(config.hotspotSSID) + "</strong></div>";
  html += "</div>";
  html += "<form action='/wifi-config' method='POST'>";
  html += "<div class='form-group'>";
  html += "<label>📡 Tên Hotspot:</label>";
  html += "<input type='text' name='hotspot_ssid' value='" + String(config.hotspotSSID) + "' maxlength='31' placeholder='SmartClock-v5'>";
  html += "</div>";
  html += "<div class='form-group'>";
  html += "<label>🔒 Mật khẩu Hotspot:</label>";
  html += "<input type='password' name='hotspot_password' value='" + String(config.hotspotPassword) + "' maxlength='31' placeholder='8+ ký tự'>";
  html += "</div>";
  html += "<button type='submit' class='btn'>💾 Cập nhật WiFi</button>";
  html += "<button type='button' onclick=\"resetWiFi()\" class='btn btn-danger'>🔄 Reset WiFi</button>";
  html += "</form>";
  html += "</div>";

  // Device Control
  html += "<div class='card'>";
  html += "<h3>🎛️ Điều khiển thiết bị</h3>";
  html += "<div style='text-align: center;'>";
  html += "<button onclick=\"restart()\" class='btn'>🔄 Khởi động lại</button>";
  html += "<button onclick=\"factoryReset()\" class='btn btn-danger'>⚠️ Khôi phục gốc</button>";
  html += "</div>";
  html += "<div style='margin-top: 15px; font-size: 0.9rem; opacity: 0.8;'>";
  html += "💡 <strong>Mẹo:</strong> Nhấn giữ nút vật lý 5 giây để reset toàn bộ";
  html += "</div>";
  html += "</div>";

  html += "</div>"; // End second grid
  html += "</div>"; // End container

  // Enhanced JavaScript with Real-time Updates
  html += "<script>";
  html += "function deleteAlarm(index){if(confirm('🗑️ Bạn có chắc muốn xóa báo thức này?')){fetch('/delete-alarm?index='+index,{method:'POST'}).then(()=>location.reload());}}";
  html += "function stopTimer(){if(confirm('⏹️ Dừng đếm ngược?')){fetch('/stop-timer',{method:'POST'}).then(()=>updateStatus());}}";
  html += "function resetWiFi(){if(confirm('🔄 Reset cấu hình WiFi và khởi động lại?')){fetch('/reset-wifi',{method:'POST'});}}";
  html += "function restart(){if(confirm('🔄 Khởi động lại thiết bị?')){fetch('/restart',{method:'POST'});}}";
  html += "function factoryReset(){if(confirm('⚠️ Khôi phục cài đặt gốc? Tất cả dữ liệu sẽ bị xóa!\\n\\nHành động này không thể hoàn tác!')){fetch('/factory-reset',{method:'POST'});}}";

  // Real-time update function
  html += "function updateStatus(){";
  html += "fetch('/status').then(r=>r.json()).then(data=>{";

  // Update LCD display
  html += "const lcdEl=document.querySelector('.lcd');";
  html += "if(lcdEl && data.lcd){";
  html += "lcdEl.innerHTML='Dòng 1: '+data.lcd.line1+'<br>Dòng 2: '+data.lcd.line2;";
  html += "}";

  // Update clock display
  html += "const clockEl=document.querySelector('.clock-display');";
  html += "if(clockEl){";
  html += "const now=new Date();";
  html += "const time=now.getHours().toString().padStart(2,'0')+':'+now.getMinutes().toString().padStart(2,'0');";
  html += "clockEl.innerHTML=time;";
  html += "}";

  // Update temperature
  html += "const tempEls=document.querySelectorAll('.subtitle, .weather-temp');";
  html += "tempEls.forEach(el=>{";
  html += "if(el.classList.contains('subtitle')){";
  html += "const now=new Date();";
  html += "const date=now.getDate().toString().padStart(2,'0')+'/'+(now.getMonth()+1).toString().padStart(2,'0')+'/'+now.getFullYear();";
  html += "el.innerHTML='📅 '+date+' | 🌡️ '+data.temperature.toFixed(1)+'C';";
  html += "}";
  html += "});";

  // Update weather info
  html += "const weatherTemp=document.querySelector('.weather-temp');";
  html += "if(weatherTemp && data.weather.valid){";
  html += "weatherTemp.innerHTML=data.weather.temp.toFixed(1)+'C';";
  html += "}";

  // Update hardware status
  html += "const statusEls=document.querySelectorAll('.status');";
  html += "statusEls.forEach(el=>{";
  html += "const text=el.textContent;";
  html += "if(text.includes('LCD')){el.className='status '+(data.hardware.lcd?'ok':'error');}";
  html += "else if(text.includes('RTC')){el.className='status '+(data.hardware.rtc?'ok':'error');}";
  html += "else if(text.includes('WiFi')){el.className='status '+(data.hardware.wifi?'ok':'error');}";
  html += "else if(text.includes('Nhiệt độ')){el.className='status '+(data.hardware.temp?'ok':'error');}";
  html += "else if(text.includes('Loa')){el.className='status '+(data.hardware.buzzer?'ok':'error');}";
  html += "else if(text.includes('LED')){el.className='status '+(data.hardware.led?'ok':'error');}";
  html += "});";

  // Update timer display
  html += "const timerCard=document.querySelector('.card h3').parentElement;";
  html += "if(data.timer.active && data.timer.remaining){";
  html += "const min=Math.floor(data.timer.remaining/60);";
  html += "const sec=data.timer.remaining%60;";
  html += "const timerDisplay=timerCard.querySelector('.timer-display');";
  html += "if(timerDisplay){";
  html += "timerDisplay.innerHTML='⏱️ '+min+':'+(sec<10?'0':'')+sec;";
  html += "}";
  html += "}";

  html += "}).catch(e=>console.log('Status update failed:',e));";
  html += "}";

  // Initial update and periodic refresh
  html += "updateStatus();";
  html += "setInterval(updateStatus,2000);"; // Update every 2 seconds for real-time feel

  // Page visibility API to pause updates when tab is not active
  html += "document.addEventListener('visibilitychange',function(){";
  html += "if(!document.hidden){updateStatus();}";
  html += "});";

  html += "</script>";

  html += "</body></html>";
  return html;
}

// ==========================================
// WEB SERVER ENDPOINTS
// ==========================================

void setupWebServer()
{
  Serial.println("===Setting up web server...===");
  // Main page
  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html; charset=utf-8", generateWebInterface()); });

  Serial.println("===Web server setup complete.===");
  // Set alarm
  server.on("/set-alarm", HTTP_POST, []()
            {
    if(alarmCount < MAX_ALARMS) {
      alarms[alarmCount].hour = server.arg("hour").toInt();
      alarms[alarmCount].minute = server.arg("minute").toInt();
      alarms[alarmCount].enabled = true;
      strncpy(alarms[alarmCount].label, server.arg("label").c_str(), sizeof(alarms[alarmCount].label) - 1);
      
      for(int i = 0; i < 7; i++) {
        alarms[alarmCount].daysOfWeek[i] = server.hasArg("day" + String(i));
      }
      
      alarmCount++;
      saveAlarms();
    }
    server.sendHeader("Location", "/");
    server.send(302); });

  // Delete alarm
  server.on("/delete-alarm", HTTP_POST, []()
            {
    int index = server.arg("index").toInt();
    if(index >= 0 && index < alarmCount) {
      for(int i = index; i < alarmCount - 1; i++) {
        alarms[i] = alarms[i + 1];
      }
      alarmCount--;
      saveAlarms();
    }
    server.sendHeader("Location", "/");
    server.send(302); });

  // Set timer
  server.on("/set-timer", HTTP_POST, []()
            {
    timer.duration = server.arg("minutes").toInt() * 60;
    timer.startTime = millis();
    timer.active = true;
    strncpy(timer.label, server.arg("label").c_str(), sizeof(timer.label) - 1);
    currentState = STATE_COUNTDOWN;
    server.sendHeader("Location", "/");
    server.send(302); });

  // Stop timer
  server.on("/stop-timer", HTTP_POST, []()
            {
    timer.active = false;
    currentState = STATE_NORMAL;
    server.sendHeader("Location", "/");
    server.send(302); });

  // WiFi config
  server.on("/wifi-config", HTTP_POST, []()
            {
    strncpy(config.hotspotSSID, server.arg("hotspot_ssid").c_str(), sizeof(config.hotspotSSID) - 1);
    strncpy(config.hotspotPassword, server.arg("hotspot_password").c_str(), sizeof(config.hotspotPassword) - 1);
    saveConfiguration();
    WiFi.softAP(config.hotspotSSID, config.hotspotPassword);
    server.sendHeader("Location", "/");
    server.send(302); });

  // Reset WiFi
  server.on("/reset-wifi", HTTP_POST, []()
            {
    wifiManager.resetSettings();
    server.send(200, "text/plain", "WiFi reset. Device restarting...");
    delay(1000);
    ESP.restart(); });

  // Restart
  server.on("/restart", HTTP_POST, []()
            {
    server.send(200, "text/plain", "Device restarting...");
    delay(1000);
    ESP.restart(); });

  // Factory reset
  server.on("/factory-reset", HTTP_POST, []()
            {
    server.send(200, "text/plain", "Factory reset initiated...");
    delay(1000);
    factoryReset(); });

  // Weather config
  server.on("/weather-config", HTTP_POST, []()
            {
    strncpy(weatherConfig.apiKey, server.arg("api_key").c_str(), sizeof(weatherConfig.apiKey) - 1);
    strncpy(weatherConfig.cityName, server.arg("city_name").c_str(), sizeof(weatherConfig.cityName) - 1);
    weatherConfig.updateInterval = 10; // Always set to 10 seconds regardless of user input
    weatherConfig.enabled = server.hasArg("enabled");
    
    saveWeatherConfig();
    
    // Reset weather data to force refresh
    weather.dataValid = false;
    weather.lastUpdate = 0;
    weather.errorCount = 0;
    
    server.sendHeader("Location", "/");
    server.send(302); });

  // Status API for real-time updates
  server.on("/status", HTTP_GET, []()
            {
    DynamicJsonDocument doc(1024);
    doc["temperature"] = currentTemp;
    doc["weather"]["temp"] = weather.temperature;
    doc["weather"]["humidity"] = weather.humidity;
    doc["weather"]["description"] = weather.description;
    doc["weather"]["city"] = weather.city;
    doc["weather"]["valid"] = weather.dataValid;
    doc["weather"]["errors"] = weather.errorCount;
    doc["lcd"]["line1"] = currentLCDLine1;
    doc["lcd"]["line2"] = currentLCDLine2;
    doc["lcd"]["mode"] = lcdDisplayMode;
    doc["hardware"]["lcd"] = hw.lcdOK;
    doc["hardware"]["rtc"] = hw.rtcOK;
    doc["hardware"]["wifi"] = hw.wifiOK;
    doc["hardware"]["temp"] = hw.tempOK;
    doc["hardware"]["buzzer"] = hw.buzzerOK;
    doc["hardware"]["led"] = hw.ledOK;
    doc["timer"]["active"] = timer.active;
    if (timer.active) {
      unsigned long remaining = timer.duration - ((millis() - timer.startTime) / 1000);
      doc["timer"]["remaining"] = remaining;
      doc["timer"]["label"] = timer.label;
    }
    doc["alarms"]["count"] = alarmCount;
    doc["alarms"]["active"] = alarmActive;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json; charset=utf-8", response); });

  server.begin();
  Serial.println("Web server started");
}

// ==========================================
// CONFIGURATION
// ==========================================

void loadConfiguration()
{
  EEPROM.get(CONFIG_ADDR, config);
  if (!config.configValid)
  {
    strcpy(config.deviceName, "Smart Clock v5.0");
    strcpy(config.hotspotSSID, "SmartClock-v5");
    strcpy(config.hotspotPassword, "smartclock123");
  }

  EEPROM.get(ALARM_ADDR, alarmCount);
  if (alarmCount > 0 && alarmCount <= MAX_ALARMS)
  {
    for (int i = 0; i < alarmCount; i++)
    {
      EEPROM.get(ALARM_ADDR + 4 + (i * sizeof(Alarm)), alarms[i]);
    }
  }
  else
  {
    alarmCount = 0;
  }

  EEPROM.get(TIMER_ADDR, timer);
}

void saveConfiguration()
{
  config.configValid = true;
  EEPROM.put(CONFIG_ADDR, config);
  EEPROM.commit();
}

void saveAlarms()
{
  EEPROM.put(ALARM_ADDR, alarmCount);
  for (int i = 0; i < alarmCount; i++)
  {
    EEPROM.put(ALARM_ADDR + 4 + (i * sizeof(Alarm)), alarms[i]);
  }
  EEPROM.commit();
}

void loadWeatherConfig()
{
  preferences.begin("weather", false);

  String apiKey = preferences.getString("apiKey", "");
  strncpy(weatherConfig.apiKey, apiKey.c_str(), sizeof(weatherConfig.apiKey) - 1);

  String cityName = preferences.getString("cityName", "Thu Duc");
  strncpy(weatherConfig.cityName, cityName.c_str(), sizeof(weatherConfig.cityName) - 1);

  String countryCode = preferences.getString("countryCode", "VN");
  strncpy(weatherConfig.countryCode, countryCode.c_str(), sizeof(weatherConfig.countryCode) - 1);

  weatherConfig.enabled = preferences.getBool("enabled", false);
  weatherConfig.updateInterval = 10; // Force 10 seconds interval

  preferences.end();
}

void saveWeatherConfig()
{
  preferences.begin("weather", false);

  preferences.putString("apiKey", weatherConfig.apiKey);
  preferences.putString("cityName", weatherConfig.cityName);
  preferences.putString("countryCode", weatherConfig.countryCode);
  preferences.putBool("enabled", weatherConfig.enabled);
  preferences.putInt("updateInterval", 10); // Always save as 10 seconds

  preferences.end();

  Serial.println("Weather config saved:");
  Serial.println("  City: " + String(weatherConfig.cityName));
  Serial.println("  API Key: " + String(strlen(weatherConfig.apiKey) > 0 ? "***" : "Not set"));
  Serial.println("  Enabled: " + String(weatherConfig.enabled ? "Yes" : "No"));
}

// ==========================================
// WIFI SETUP
// ==========================================
void tryConnectWiFiFirst()
{
  // Removed LCD.clear() and LCD WiFi setup message
  LCD.clear();
  updateLCDContent("WiFi", "Connecting WiFi...");

  Serial.println("[WiFi] Trying to connect to previously saved WiFi...");

  WiFi.mode(WIFI_STA); // Đảm bảo ở chế độ Station
  WiFi.begin();        // Dùng WiFi đã lưu trong flash

  unsigned long t0 = millis();
  bool connected = false;
  int retryCount = 0;

  while (millis() - t0 < 5000)
  { // Thử trong 5 giây
    wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
      connected = true;
      break;
    }
    retryCount++;
    Serial.print("[WiFi] Retry #");
    Serial.print(retryCount);
    Serial.print(" - Status: ");
    switch (status)
    {
    case WL_IDLE_STATUS:
      Serial.println("IDLE");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("NO SSID AVAILABLE");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("SCAN COMPLETED");
      break;
    case WL_CONNECTED:
      Serial.println("CONNECTED");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("CONNECT FAILED");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("CONNECTION LOST");
      break;
    case WL_DISCONNECTED:
      Serial.println("DISCONNECTED");
      break;
    default:
      Serial.println("UNKNOWN");
      break;
    }
    delay(300);
  }

  if (connected)
  {
    Serial.print("[WiFi] Connected! IP: ");
    Serial.println(WiFi.localIP());
    // Stop the config portal and AP after successful connection
    wifiManager.stopConfigPortal();
    WiFi.softAPdisconnect(true); // This disables the AP interface
    Serial.println("[WiFi] Stopping WiFiManager config portal.");
  }
  else
  {
    Serial.println("[WiFi] Failed to connect in 5 seconds. Starting WiFiManager AP mode...");
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.startConfigPortal(config.hotspotSSID, config.hotspotPassword);
  }
}

void setupWiFi()
{
  Serial.println("Setting up WiFi...");
  tryConnectWiFiFirst();
  // bool connected = wifiManager.autoConnect(config.hotspotSSID, config.hotspotPassword);
}

// ==========================================
// HÀM CON: Xử lý timer alarm tách ra cho rõ ràng
// ==========================================
void handleTimerAlarm()
{
  if (timer.alarmTriggered)
  {
    unsigned long alarmElapsed = millis() - timer.alarmStartTime;

    if (alarmElapsed < 5000)
    { // 5s alarm
      static unsigned long lastBlinkTimer = 0;
      if (millis() - lastBlinkTimer > 250)
      { // Blink nhanh
        lastBlinkTimer = millis();
        static bool timerBlinkState = false;
        timerBlinkState = !timerBlinkState;

        if (timerBlinkState)
        {
          digitalWrite(BUZZER_PIN, HIGH);
          digitalWrite(LED_PIN, HIGH);
          updateLCDContent("*** TIMER ***", timer.label);
        }
        else
        {
          digitalWrite(BUZZER_PIN, LOW);
          digitalWrite(LED_PIN, LOW);
          LCD.clear();
        }
      }
    }
    else
    {
      // Hết 5s thì tắt chuông
      timer.alarmTriggered = false;
      timer.finished = false;
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      Serial.println("=== COUNTDOWN ALARM FINISHED ===");
    }
  }
}

// ==========================================
// MAIN FUNCTIONS
// ==========================================
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("=== Smart Clock v5.1 Enhanced ===");
  Serial.println("Initializing hardware...");

  // Initialize button with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Attach interrupt for immediate buzzer shutoff
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, FALLING);
  Serial.println("✓ Button interrupt attached to GPIO 26");

  // Test LED and Buzzer at startup
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Initialize hardware
  LCD.init();
  LCD.backlight();
  hw.lcdOK = true;
  Serial.println("✓ LCD initialized");

  if (rtc.begin())
  {
    hw.rtcOK = true;
    Serial.println("✓ RTC initialized");
  }
  else
  {
    hw.rtcOK = false;
    Serial.println("✗ RTC failed");
  }

  if (analogRead(NTC_PIN) > 0 && analogRead(NTC_PIN) < 4095)
  {
    hw.tempOK = true;
    currentTemp = convertAdcToTemperature(analogRead(NTC_PIN));
    Serial.println("✓ Temperature sensor initialized");
  }
  else
  {
    hw.tempOK = false;
    Serial.println("✗ Temperature sensor failed");
  }

  // Test LED and Buzzer
  hw.ledOK = true;
  hw.buzzerOK = true;
  Serial.println("✓ LED and Buzzer ready");

  // Check for first boot and clear data if needed
  if (checkFirstBoot())
  {
    Serial.println("First boot detected - clearing all data");
    clearAllData();
  }

  // Load configuration
  loadConfiguration();
  loadWeatherConfig();

  // Initialize LCD mode change timer
  lastLCDModeChange = millis();

  // Initialize weather data
  weather.dataValid = false;
  weather.lastUpdate = 0;
  weather.errorCount = 0;

  Serial.println("Starting WiFi setup...");
  setupWiFi();

  if (WiFi.status() == WL_CONNECTED)
  {
    hw.wifiOK = true;
    Serial.println("✓ WiFi connected: " + WiFi.localIP().toString());
    // Fetch initial weather data
    fetchWeatherData();
  }
  else
  {
    hw.wifiOK = false;
    Serial.println("WiFi setup completed (Hotspot mode)");
  }

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.startConfigPortal(config.hotspotSSID, config.hotspotPassword);

  // Initial display
  updateLCDContent("Smart Clock v5.1", "Starting...");
  delay(2000);

  currentState = STATE_NORMAL;
  Serial.println("=== Setup Complete ===");
}

void loop()
{
  // ===================== [A] XỬ LÝ WIFI: Auto Reconnect & Reset NTP Sync Flag =====================
  static bool triedReconnect = false;
  static unsigned long lastWiFiRetry = 0;
  if (WiFi.status() != WL_CONNECTED)
  {
    // Nếu mất WiFi, thử reconnect mỗi 30s, tránh reconnect liên tục
    if (!triedReconnect && millis() - lastWiFiRetry > 30000)
    {
      lastWiFiRetry = millis();
      triedReconnect = true;
      Serial.println("[WiFi] Attempting reconnect...");
      WiFi.reconnect();
    }
    rtcSynced = false; // Reset cờ sync NTP khi mất WiFi
  }
  else
  {
    triedReconnect = false; // Đã kết nối lại thì reset flag
  }

  // ===================== [B] XỬ LÝ WIFI MANAGER (Non-blocking Config Portal) =====================
  wifiManager.process();

  // Nếu đã kết nối WiFi thành công, tắt portal để nhường cổng cho webserver
  if (WiFi.status() == WL_CONNECTED && wifiManager.getConfigPortalActive())
  {
    wifiManager.stopConfigPortal();
    Serial.println("WiFiManager portal stopped!");
  }

  // ===================== [C] QUẢN LÝ KHỞI TẠO/DỪNG WEBSERVER THEO TRẠNG THÁI WIFI =====================
  if (WiFi.status() == WL_CONNECTED && !webServerStarted)
  {
    setupWebServer();
    webServerStarted = true;
    Serial.println("✓ Web server started");
  }
  if (WiFi.status() != WL_CONNECTED && webServerStarted)
  {
    webServerStarted = false;
    // (Optional) server.stop();
  }
  if (webServerStarted)
  {
    server.handleClient();
  }

  // ===================== [D] ĐỌC CẢM BIẾN NHIỆT ĐỘ (Định kỳ 5 giây) =====================
  static unsigned long lastTempRead = 0;
  if (millis() - lastTempRead > 5000)
  {
    lastTempRead = millis();
    if (hw.tempOK)
    {
      int adcValue = analogRead(NTC_PIN);
      currentTemp = convertAdcToTemperature(adcValue);
    }
  }

  // ===================== [E] XỬ LÝ ALARM/TIMER, BUZZER, LED =====================
  handleTimerAlarm();

  // ===================== [F] XỬ LÝ NÚT BẤM (Debounce mỗi 50ms) =====================
  static unsigned long lastButtonCheck = 0;
  if (millis() - lastButtonCheck > 50)
  {
    lastButtonCheck = millis();
    handleButton();
  }

  // ===================== [G] ĐỒNG BỘ RTC VỚI NTP (Khi mới có WiFi) =====================
  if (WiFi.status() == WL_CONNECTED && !rtcSynced)
  {
    syncTimeWithNTP();
    rtcSynced = true;
  }

  // ===================== [H] CẬP NHẬT TRẠNG THÁI WIFI CHO ỨNG DỤNG =====================
  hw.wifiOK = (WiFi.status() == WL_CONNECTED);

  // ===================== [I] FETCH DỮ LIỆU THỜI TIẾT (Chỉ khi có WiFi) =====================
  if (WiFi.status() == WL_CONNECTED)
    fetchWeatherData();

  // ===================== [J] STATE MACHINE: XỬ LÝ HIỂN THỊ GIAO DIỆN & CHẾ ĐỘ =====================
  switch (currentState)
  {
  case STATE_NORMAL:
    displayClock();
    checkAlarms();
    break;
  case STATE_COUNTDOWN:
    displayCountdown();
    if (!timer.active)
      currentState = STATE_NORMAL;
    break;
  case STATE_ALARM:
    updateAlarmDisplay();
    break;
  }

  // ===================== [Z] GIẢM TẢI CPU (Cho main loop mượt hơn) =====================
  delay(100);
}

// ==========================================
// WEATHER FUNCTIONS
// ==========================================
void fetchWeatherData()
{
  // Throttle WEATHER CONFIG DEBUG to print only every 5 seconds
  static unsigned long lastWeatherDebug = 0;
  unsigned long now = millis();

  if (now - lastWeatherDebug > 3000)
  {

    Serial.println("[CHECK] wifiOK = " + String(hw.wifiOK));
    Serial.println("[CHECK] weatherConfig.enabled = " + String(weatherConfig.enabled));
    Serial.println("[CHECK] apiKey length = " + String(strlen(weatherConfig.apiKey)));
    Serial.println("[CHECK] time since last update = " + String(now - weather.lastUpdate));
    Serial.println("[CHECK] update interval = " + String(weatherConfig.updateInterval * 1000));
    //===============================
    Serial.println("=== WEATHER CONFIG DEBUG ===");
    Serial.println("WiFi OK: " + String(hw.wifiOK ? "Yes" : "No"));
    Serial.println("Weather Enabled: " + String(weatherConfig.enabled ? "Yes" : "No"));
    Serial.println("API Key Length: " + String(strlen(weatherConfig.apiKey)));
    Serial.println("API Key: " + String(strlen(weatherConfig.apiKey) > 0 ? "***" : "EMPTY"));
    lastWeatherDebug = now;
  }

  if (!hw.wifiOK || !weatherConfig.enabled || strlen(weatherConfig.apiKey) == 0)
  {
    if (now - lastWeatherDebug > 3000)
    {
      Serial.println("Weather API not enabled or WiFi not connected, using simulation...");
    }
    // Fallback to simulation if no API key or WiFi
    unsigned long now = millis();
    if (now - weather.lastUpdate > 600000)
    {                                                    // Update every 10 minutes
      weather.temperature = currentTemp + random(-3, 4); // Simulate outdoor temp
      weather.humidity = random(40, 90);
      weather.description = "Mô phỏng";
      weather.city = "Thu Duc";
      weather.dataValid = true;
      weather.lastUpdate = now;
      weather.errorCount = 0;
      Serial.println("Weather simulation updated");
    }
    return;
  }

  if (now - weather.lastUpdate < weatherConfig.updateInterval * 1000)
    return;

  Serial.println("Fetching weather data from OpenWeather API...");

  HTTPClient http;
  WiFiClient client;

  // Build OpenWeather API URL
  String url = "http://api.openweathermap.org/data/2.5/weather?q=Thu%20Duc,VN&appid=9b8496f9657da84893a94b1cf4c74bd7&units=metric&lang=vi";
  // url += "?q=" + String(weatherConfig.cityName) + "," + String(weatherConfig.countryCode);
  // url += "&appid=" + String(weatherConfig.apiKey);
  // url += "&units=metric";
  // url += "&lang=vi";

  Serial.println("API URL: " + url);

  http.begin(client, url);
  http.setTimeout(10000); // 10 second timeout

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();
    Serial.println("Weather API Response: " + payload);

    // Parse JSON response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {
      weather.temperature = doc["main"]["temp"];
      weather.humidity = doc["main"]["humidity"];
      weather.description = doc["weather"][0]["description"].as<String>();
      weather.city = doc["name"].as<String>();
      weather.dataValid = true;
      weather.lastUpdate = now;
      weather.errorCount = 0;

      Serial.println("Weather updated: " + weather.city + " " + String(weather.temperature, 1) + "°C");
    }
    else
    {
      Serial.println("JSON parsing error: " + String(error.c_str()));
      weather.errorCount++;
    }
  }
  else
  {
    Serial.println("HTTP error: " + String(httpCode));
    weather.errorCount++;

    if (httpCode == 401)
    {
      Serial.println("Invalid API key!");
    }
    else if (httpCode == 404)
    {
      Serial.println("City not found!");
    }
  }

  http.end();

  // If too many errors, disable for a while
  if (weather.errorCount > 5)
  {
    weather.lastUpdate = now + (30 * 60 * 1000); // Wait 30 minutes before retry
    Serial.println("Too many weather API errors. Waiting 30 minutes...");
  }
}

void checkAlarms()
{
  if (alarmActive || currentState != STATE_NORMAL)
    return;

  DateTime now = rtc.now();
  int currentDay = now.dayOfTheWeek();

  for (int i = 0; i < alarmCount; i++)
  {
    if (alarms[i].enabled &&
        alarms[i].hour == now.hour() &&
        alarms[i].minute == now.minute() &&
        now.second() < 5 &&
        alarms[i].daysOfWeek[currentDay])
    {
      triggerAlarm(i);
      break;
    }
  }
}

// Synchronize RTC with NTP time if WiFi is connected
void syncTimeWithNTP()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[NTP] WiFi not connected, skipping NTP sync.");
    return;
  }
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // GMT+7, adjust as needed
  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    Serial.printf("[NTP] Time from NTP: %04d-%02d-%02d %02d:%02d:%02d\n",
                  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    // Set RTC from NTP time
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
    Serial.println("[NTP] RTC updated from NTP.");
  }
  else
  {
    Serial.println("[NTP] Failed to get time from NTP server.");
  }
}