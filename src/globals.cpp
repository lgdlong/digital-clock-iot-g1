#include "config.h"
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <WiFiManager.h>
#include <Preferences.h>

// ==========================================
// GLOBAL VARIABLES DEFINITIONS
// ==========================================
SystemState currentState = STATE_BOOT;
unsigned long stateStartTime = 0;
DeviceConfig config;
HardwareStatus hw;

// Temperature Data
float currentTemp = 25.0;
float minTemp = 999.0;
float maxTemp = -999.0;

// Alarm System
Alarm alarms[MAX_ALARMS];
int alarmCount = 0;
bool alarmActive = false;
int activeAlarmIndex = -1;

// Countdown Timer
CountdownTimer timer;

// Weather Data
WeatherData weather;

// LCD Display State
String currentLCDLine1 = "";
String currentLCDLine2 = "";
unsigned long lastLCDUpdate = 0;
int lcdDisplayMode = 0; // 0: Clock/Temp, 1: Weather
unsigned long lastLCDModeChange = 0;

// Hardware Objects
LiquidCrystal_I2C LCD(0x27, 16, 2);
RTC_DS1307 rtc;
WiFiManager wifiManager;
Preferences preferences;
