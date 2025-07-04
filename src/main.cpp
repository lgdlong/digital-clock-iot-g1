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

// Include modular components
#include "config.h"
#include "utils.h"
#include "display.h"
#include "alarm.h"
#include "button.h"
#include "serial_commands.h"
#include "config_manager.h"
#include "mqtt_manager.h"
#include "wifi_manager.h"
#include "weather.h"

// ==========================================
// MAIN FUNCTIONS
// ==========================================

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("=== Smart Clock v5.3 Standalone ===");
  Serial.println("Initializing hardware...");

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

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

  hw.ledOK = true;
  hw.buzzerOK = true;
  Serial.println("✓ LED and Buzzer ready");

  if (checkFirstBoot())
  {
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

  if (WiFi.status() == WL_CONNECTED)
  {
    hw.wifiOK = true;
    Serial.println("✓ WiFi connected: " + WiFi.localIP().toString());

    // Initialize MQTT
    mqttClient.setServer(mqttServers[currentBrokerIndex], mqttPort);
    mqttClient.setCallback(mqttCallback);

    fetchWeatherData();
  }
  else
  {
    hw.wifiOK = false;
    Serial.println("WiFi setup completed (Hotspot mode)");
  }

  updateLCDContent("Smart Clock v5.3", "Standalone Mode");
  delay(2000);

  currentState = STATE_NORMAL;
  Serial.println("=== Setup Complete ===");
  Serial.println("Type 'help' for available commands");
}

void loop()
{
  handleSerialCommands();

  // MQTT loop
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!mqttClient.connected())
    {
      connectMQTT();
    }
    mqttClient.loop();
  }

  // Temperature reading
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

  // Timer alarm handling
  handleTimerAlarm();

  // Button handling
  static unsigned long lastButtonCheck = 0;
  if (millis() - lastButtonCheck > 50)
  {
    lastButtonCheck = millis();
    handleButton();
  }

  // Update WiFi status
  hw.wifiOK = (WiFi.status() == WL_CONNECTED);

  // Update weather data
  fetchWeatherData();

  // Main state machine
  switch (currentState)
  {
  case STATE_NORMAL:
    displayClock();
    checkAlarms();
    break;

  case STATE_COUNTDOWN:
    displayCountdown();
    if (!timer.active)
    {
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