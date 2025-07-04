#include <WiFi.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <PubSubClient.h>

// ==========================================
// SMART CLOCK v5.3 - STANDALONE ESP32
// ==========================================

#define FIRMWARE_VERSION "v5.3.0"

// Pin Definitions
#define LED_PIN 12        
#define BUZZER_PIN 25     
#define BUTTON_PIN 13     
#define NTC_PIN 34        

// Hardware Objects
LiquidCrystal_I2C LCD(0x27, 16, 2);
RTC_DS1307 rtc;
WiFiManager wifiManager;
Preferences preferences;

// MQTT Variables
const char* mqttServer = "broker.emqx.io";  // Thử EMQX thay vì HiveMQ
const int mqttPort = 1883;  // TCP port for ESP32
const char* mqttUser = "";  // Empty for public broker
const char* mqttPassword = "";  // Empty for public broker
const char* timePublishTopic = "clock/time";
const char* resetSubscribeTopic = "clock/reset";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// System States
enum SystemState {
  STATE_BOOT, STATE_NORMAL, STATE_ALARM, STATE_COUNTDOWN, STATE_INFO, STATE_ERROR, STATE_MENU
};

SystemState currentState = STATE_BOOT;
unsigned long stateStartTime = 0;

// Configuration
struct DeviceConfig {
  char deviceName[64] = "Smart Clock v5.3";
  char hotspotSSID[32] = "IOT_NHOM1";
  char hotspotPassword[32] = "12345678";
  bool configValid = false;
} config;

// Hardware Status
struct HardwareStatus {
  bool lcdOK = false;
  bool rtcOK = false;
  bool wifiOK = false;
  bool tempOK = false;
  bool buzzerOK = false;
  bool ledOK = false;
  String lastError = "";
} hw;

// Temperature Data
float currentTemp = 25.0;
float minTemp = 999.0;
float maxTemp = -999.0;

// Alarm System
struct Alarm {
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
struct CountdownTimer {
  unsigned long duration = 0; // in seconds
  unsigned long startTime = 0;
  bool active = false;
  bool finished = false;
  char label[32] = "Timer";
  bool alarmTriggered = false;
  unsigned long alarmStartTime = 0;
} timer;

// Weather Data
struct WeatherData {
  float temperature = 0.0;
  int humidity = 0;
  String description = "N/A";
  String city = "Thủ Đức";
  bool dataValid = false;
  unsigned long lastUpdate = 0;
  int errorCount = 0;
} weather;

// LCD Display State
String currentLCDLine1 = "";
String currentLCDLine2 = "";
unsigned long lastLCDUpdate = 0;
int lcdDisplayMode = 0; // 0: Clock/Temp, 1: Weather
unsigned long lastLCDModeChange = 0;

// Constants
#define EEPROM_SIZE 1024
#define CONFIG_ADDR 0
#define ALARM_ADDR 400
#define TIMER_ADDR 800

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

bool checkFirstBoot() {
  preferences.begin("clock", false);
  bool firstBoot = preferences.getBool("firstBoot", true);
  if (firstBoot) {
    preferences.putBool("firstBoot", false);
    preferences.end();
    return true;
  }
  preferences.end();
  return false;
}

void clearAllData() {
  Serial.println("Clearing all stored data...");
  
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  
  preferences.begin("clock", false);
  preferences.clear();
  preferences.end();
  
  alarmCount = 0;
  timer.active = false;
  timer.duration = 0;
  strcpy(config.deviceName, "SmartClock-v5.3");
  strcpy(config.hotspotSSID, "IOT_NHOM1");
  strcpy(config.hotspotPassword, "12345678");
  
  Serial.println("All data cleared!");
}

float convertAdcToTemperature(int adcValue) {
  if (adcValue == 0) return 25.0;
  float voltage = (adcValue / 4095.0) * 3.3;
  float tempC = voltage / 0.01;
  if (tempC < -10 || tempC > 100) {
    return 25.0;
  }
  return tempC;
}

void factoryReset() {
  Serial.println("=== FACTORY RESET ===");
  preferences.clear();
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  wifiManager.resetSettings();
  ESP.restart();
}

// ==========================================
// DISPLAY FUNCTIONS
// ==========================================

void updateLCDContent(String line1, String line2) {
  if (line1 != currentLCDLine1 || line2 != currentLCDLine2) {
    currentLCDLine1 = line1;
    currentLCDLine2 = line2;
    lastLCDUpdate = millis();
    
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print(line1.substring(0, 16));
    LCD.setCursor(0, 1);
    LCD.print(line2.substring(0, 16));
  }
}

void displayClock() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  if (now - lastLCDModeChange > 60000) {
    lcdDisplayMode = (lcdDisplayMode + 1) % 2;
    lastLCDModeChange = now;
  }
  
  if (now - lastUpdate < 1000) return;
  lastUpdate = now;
  
  DateTime rtcNow = rtc.now();
  
  if (lcdDisplayMode == 0) {
    char line1[17];
    snprintf(line1, sizeof(line1), "%02d:%02d:%02d %4.1f°C",
             rtcNow.hour(), rtcNow.minute(), rtcNow.second(), currentTemp);
    
    char line2[17];
    String status = hw.wifiOK ? "WIFI" : "DISC";
    if (alarmCount > 0) status = "A" + String(alarmCount);
    if (timer.active) status = "TIMER";
    
    snprintf(line2, sizeof(line2), "%02d/%02d/%02d %s",
             rtcNow.day(), rtcNow.month(), rtcNow.year() % 100, status.c_str());
    
    updateLCDContent(String(line1), String(line2));
  } else {
    char line1[17];
    char line2[17];
    
    if (weather.dataValid) {
      snprintf(line1, sizeof(line1), "%s %.1f°C",
               weather.city.substring(0, 8).c_str(), weather.temperature);
      snprintf(line2, sizeof(line2), "%s %d%%",
               weather.description.substring(0, 10).c_str(), weather.humidity);
    } else {
      strcpy(line1, "Thoi Tiet");
      strcpy(line2, "Khong co du lieu");
    }
    
    updateLCDContent(String(line1), String(line2));
  }
}

void displayCountdown() {
  if (!timer.active) return;
  
  unsigned long elapsed = (millis() - timer.startTime) / 1000;
  if (elapsed >= timer.duration) {
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

void displayMenu() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    updateLCDContent("MENU MODE", "Press to exit");
  }
  
  if (millis() - stateStartTime > 10000) {
    currentState = STATE_NORMAL;
  }
}

// ==========================================
// ALARM & TIMER SYSTEM
// ==========================================

void triggerAlarm(int index) {
  activeAlarmIndex = index;
  alarmActive = true;
  timer.finished = false;
  currentState = STATE_ALARM;
  stateStartTime = millis();
}

void stopAlarm() {
  alarmActive = false;
  timer.active = false;
  timer.finished = false;
  timer.alarmTriggered = false;
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  currentState = STATE_NORMAL;
  activeAlarmIndex = -1;
}

void updateAlarmDisplay() {
  static unsigned long lastBlink = 0;
  static bool blinkState = false;
  
  if (millis() - lastBlink > 500) {
    lastBlink = millis();
    blinkState = !blinkState;
    
    if (blinkState) {
      String label = "WAKE UP!";
      if (activeAlarmIndex >= 0) {
        label = alarms[activeAlarmIndex].label;
      } else if (timer.finished) {
        label = timer.label;
      }
      
      updateLCDContent("*** ALARM ***", label);
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
    } else {
      LCD.clear();
      currentLCDLine1 = "";
      currentLCDLine2 = "";
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }
  }
  
  if (millis() - stateStartTime > 5 * 60 * 1000) {
    stopAlarm();
  }
}

void checkAlarms() {
  if (alarmActive || currentState != STATE_NORMAL) return;
  
  DateTime now = rtc.now();
  int currentDay = now.dayOfTheWeek();
  
  for (int i = 0; i < alarmCount; i++) {
    if (alarms[i].enabled && 
        alarms[i].hour == now.hour() && 
        alarms[i].minute == now.minute() &&
        now.second() < 5 &&
        alarms[i].daysOfWeek[currentDay]) {
      triggerAlarm(i);
      break;
    }
  }
}

// ==========================================
// BUTTON HANDLING
// ==========================================

void handleButton() {
  static unsigned long lastPress = 0;
  static bool lastState = HIGH;
  static unsigned long pressStartTime = 0;
  
  bool buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState != lastState && millis() - lastPress > 50) {
    lastPress = millis();
    
    if (buttonState == LOW) {
      pressStartTime = millis();
    } else {
      unsigned long pressDuration = millis() - pressStartTime;
      
      if (pressDuration >= 5000) {
        factoryReset();
      } else {
        switch(currentState) {
          case STATE_ALARM:
            stopAlarm();
            break;
          case STATE_NORMAL:
            if (timer.alarmTriggered) {
              timer.alarmTriggered = false;
              timer.finished = false;
              digitalWrite(BUZZER_PIN, LOW);
              digitalWrite(LED_PIN, LOW);
              Serial.println("Timer alarm stopped by button");
            }
            break;
        }
      }
    }
  }
  
  lastState = buttonState;
}

// ==========================================
// SERIAL COMMANDS
// ==========================================

void handleSerialCommands() {
  if (!Serial.available()) return;
  
  String command = Serial.readString();
  command.trim();
  command.toLowerCase();
  
  if (command == "help") {
    Serial.println("📋 Available commands:");
    Serial.println("  help - Show this help");
    Serial.println("  status - Show device status");
    Serial.println("  alarms - Show all alarms");
    Serial.println("  add - Add new alarm");
    Serial.println("  timer - Start countdown timer");
    Serial.println("  weather - Show weather info");
    Serial.println("  reset - Factory reset");
  }
  else if (command == "status") {
    DateTime now = rtc.now();
    Serial.println("📊 Device Status:");
    Serial.println("  Time: " + String(now.hour()) + ":" + String(now.minute()));
    Serial.println("  Temp: " + String(currentTemp, 1) + "°C");
    Serial.println("  WiFi: " + String(hw.wifiOK ? "Connected" : "Disconnected"));
    Serial.println("  Alarms: " + String(alarmCount));
    Serial.println("  Timer: " + String(timer.active ? "Running" : "Stopped"));
  }
  else if (command == "alarms") {
    Serial.println("📋 Current alarms:");
    if (alarmCount == 0) {
      Serial.println("  No alarms set");
    } else {
      for (int i = 0; i < alarmCount; i++) {
        Serial.print("  " + String(i + 1) + ". ");
        Serial.print(String(alarms[i].hour) + ":" + (alarms[i].minute < 10 ? "0" : "") + String(alarms[i].minute));
        Serial.print(" - " + String(alarms[i].label));
        Serial.println(alarms[i].enabled ? " (ON)" : " (OFF)");
      }
    }
  }
  else if (command == "add") {
    if (alarmCount >= MAX_ALARMS) {
      Serial.println("❌ Maximum alarms reached");
      return;
    }
    
    Serial.println("➕ Add new alarm:");
    Serial.print("Enter hour (0-23): ");
    while (!Serial.available()) delay(10);
    int hour = Serial.parseInt();
    
    Serial.print("Enter minute (0-59): ");
    while (!Serial.available()) delay(10);
    int minute = Serial.parseInt();
    
    Serial.print("Enter label (optional): ");
    while (!Serial.available()) delay(10);
    String label = Serial.readString();
    label.trim();
    
    alarms[alarmCount].hour = hour;
    alarms[alarmCount].minute = minute;
    alarms[alarmCount].enabled = true;
    strncpy(alarms[alarmCount].label, label.c_str(), sizeof(alarms[alarmCount].label) - 1);
    
    for (int i = 0; i < 7; i++) {
      alarms[alarmCount].daysOfWeek[i] = true;
    }
    
    alarmCount++;
    // saveAlarms();
    
    Serial.println("✅ Alarm added: " + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute));
  }
  else if (command == "timer") {
    if (timer.active) {
      Serial.println("❌ Timer already running");
      return;
    }
    
    Serial.print("Enter duration in minutes: ");
    while (!Serial.available()) delay(10);
    int minutes = Serial.parseInt();
    
    Serial.print("Enter label (optional): ");
    while (!Serial.available()) delay(10);
    String label = Serial.readString();
    label.trim();
    
    timer.duration = minutes * 60;
    timer.startTime = millis();
    timer.active = true;
    strncpy(timer.label, label.c_str(), sizeof(timer.label) - 1);
    currentState = STATE_COUNTDOWN;
    
    Serial.println("✅ Timer started: " + String(minutes) + " minutes");
  }
  else if (command == "weather") {
    Serial.println("🌤️ Weather Info:");
    if (weather.dataValid) {
      Serial.println("  City: " + weather.city);
      Serial.println("  Temp: " + String(weather.temperature, 1) + "°C");
      Serial.println("  Humidity: " + String(weather.humidity) + "%");
      Serial.println("  Description: " + weather.description);
    } else {
      Serial.println("  No weather data available");
    }
  }
  else if (command == "reset") {
    Serial.println("⚠️ Factory reset in 3 seconds...");
    delay(3000);
    factoryReset();
  }
  else {
    Serial.println("❌ Unknown command. Type 'help' for available commands");
  }
}

// ==========================================
// CONFIGURATION
// ==========================================

void loadConfiguration() {
  EEPROM.get(CONFIG_ADDR, config);
  if (!config.configValid) {
    strcpy(config.deviceName, "Smart Clock v5.3");
    strcpy(config.hotspotSSID, "IOT_NHOM1");
    strcpy(config.hotspotPassword, "12345678");
  }
  
  EEPROM.get(ALARM_ADDR, alarmCount);
  if (alarmCount > 0 && alarmCount <= MAX_ALARMS) {
    for (int i = 0; i < alarmCount; i++) {
      EEPROM.get(ALARM_ADDR + 4 + (i * sizeof(Alarm)), alarms[i]);
    }
  } else {
    alarmCount = 0;
  }
  
  EEPROM.get(TIMER_ADDR, timer);
}

void saveConfiguration() {
  config.configValid = true;
  EEPROM.put(CONFIG_ADDR, config);
  EEPROM.commit();
}

void saveAlarms() {
  EEPROM.put(ALARM_ADDR, alarmCount);
  for (int i = 0; i < alarmCount; i++) {
    EEPROM.put(ALARM_ADDR + 4 + (i * sizeof(Alarm)), alarms[i]);
  }
  EEPROM.commit();
}

// ==========================================
// MQTT FUNCTIONS
// ==========================================

void publishCurrentTime(); // prototype để đảm bảo mqttCallback gọi được

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("MQTT message received on topic: " + String(topic) + " - Message: " + message);
  if (String(topic) == resetSubscribeTopic && message == "reset") {
    publishCurrentTime();
  }
}

void publishCurrentTime() {
  if (!mqttClient.connected()) return;
  DateTime now = rtc.now();
  char timeBuffer[9];
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  String timePayload = String(timeBuffer);
  if (mqttClient.publish(timePublishTopic, timePayload.c_str())) {
    Serial.println("Time published to MQTT: " + timePayload);
  } else {
    Serial.println("Failed to publish time to MQTT");
  }
}

void connectMQTT() {
  while (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqttServer);
    Serial.print(":");
    Serial.print(mqttPort);
    Serial.print("...");
    
    String clientId = "ESP32Clock-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println(" connected!");
      Serial.println("Client ID: " + clientId);
      mqttClient.subscribe(resetSubscribeTopic);
      Serial.println("Subscribed to: " + String(resetSubscribeTopic));
      Serial.println("Will publish to: " + String(timePublishTopic));
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ==========================================
// WEATHER FUNCTIONS
// ==========================================

void fetchWeatherData() {
  unsigned long now = millis();
  if (now - weather.lastUpdate > 600000) { // Update every 10 minutes
    weather.temperature = currentTemp + random(-3, 4);
    weather.humidity = random(40, 90);
    weather.description = "Mô phỏng";
    weather.city = "Thủ Đức";
    weather.dataValid = true;
    weather.lastUpdate = now;
    weather.errorCount = 0;
  }
}

// ==========================================
// WIFI SETUP
// ==========================================

void setupWiFi() {
  Serial.println("Setting up WiFi...");
  
  bool connected = wifiManager.autoConnect(config.hotspotSSID, config.hotspotPassword);
  
  if (connected) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    hw.wifiOK = true;
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(config.hotspotSSID, config.hotspotPassword);
    Serial.println("Hotspot: " + String(config.hotspotSSID));
  } else {
    Serial.println("WiFi failed, hotspot only");
    hw.wifiOK = false;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.hotspotSSID, config.hotspotPassword);
  }
}

// ==========================================
// MAIN FUNCTIONS
// ==========================================

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("=== Smart Clock v5.3 Standalone ===");
  Serial.println("Initializing hardware...");
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  LCD.init();
  LCD.backlight();
  hw.lcdOK = true;
  Serial.println("✓ LCD initialized");
  
  if (rtc.begin()) {
    hw.rtcOK = true;
    Serial.println("✓ RTC initialized");
  } else {
    hw.rtcOK = false;
    Serial.println("✗ RTC failed");
  }
  
  if (analogRead(NTC_PIN) > 0 && analogRead(NTC_PIN) < 4095) {
    hw.tempOK = true;
    currentTemp = convertAdcToTemperature(analogRead(NTC_PIN));
    Serial.println("✓ Temperature sensor initialized");
  } else {
    hw.tempOK = false;
    Serial.println("✗ Temperature sensor failed");
  }
  
  hw.ledOK = true;
  hw.buzzerOK = true;
  Serial.println("✓ LED and Buzzer ready");
  
  if (checkFirstBoot()) {
    Serial.println("First boot detected - clearing all data");
    clearAllData();
  }
  
  loadConfiguration();
  
  lastLCDModeChange = millis();
  
  weather.dataValid = false;
  weather.lastUpdate = 0;
  weather.errorCount = 0;
  
  Serial.println("Starting WiFi setup...");
  setupWiFi();
  
  if (WiFi.status() == WL_CONNECTED) {
    hw.wifiOK = true;
    Serial.println("✓ WiFi connected: " + WiFi.localIP().toString());
    
    // Initialize MQTT
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqttCallback);
    
    fetchWeatherData();
  } else {
    hw.wifiOK = false;
    Serial.println("WiFi setup completed (Hotspot mode)");
  }
  
  updateLCDContent("Smart Clock v5.3", "Standalone Mode");
  delay(2000);
  
  currentState = STATE_NORMAL;
  Serial.println("=== Setup Complete ===");
  Serial.println("Type 'help' for available commands");
}

void loop() {
  handleSerialCommands();
  
  // MQTT loop
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      connectMQTT();
    }
    mqttClient.loop();
  }
  
  static unsigned long lastTempRead = 0;
  if (millis() - lastTempRead > 5000) {
    lastTempRead = millis();
    if (hw.tempOK) {
      int adcValue = analogRead(NTC_PIN);
      currentTemp = convertAdcToTemperature(adcValue);
    }
  }
  
  if (timer.alarmTriggered) {
    unsigned long alarmElapsed = millis() - timer.alarmStartTime;
    
    if (alarmElapsed < 5000) {
      static unsigned long lastBlinkTimer = 0;
      if (millis() - lastBlinkTimer > 250) {
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
      timer.alarmTriggered = false;
      timer.finished = false;
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      Serial.println("=== COUNTDOWN ALARM FINISHED ===");
    }
  }
  
  static unsigned long lastButtonCheck = 0;
  if (millis() - lastButtonCheck > 50) {
    lastButtonCheck = millis();
    handleButton();
  }
  
  hw.wifiOK = (WiFi.status() == WL_CONNECTED);
  
  fetchWeatherData();
  
  switch (currentState) {
    case STATE_NORMAL:
      displayClock();
      checkAlarms();
      break;
      
    case STATE_COUNTDOWN:
      displayCountdown();
      if (!timer.active) {
        currentState = STATE_NORMAL;
      }
      break;
      
    case STATE_ALARM:
      updateAlarmDisplay();
      break;
      
    case STATE_MENU:
      displayMenu();
      break;
  }
  
  delay(100);
}
