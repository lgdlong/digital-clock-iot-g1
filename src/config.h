#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <WiFiManager.h>
#include <Preferences.h>

// ==========================================
// FIRMWARE & VERSION
// ==========================================
#define FIRMWARE_VERSION "v5.3.0"

// ==========================================
// PIN DEFINITIONS
// ==========================================
#define LED_PIN 12
#define BUZZER_PIN 25
#define BUTTON_PIN 13
#define NTC_PIN 34

// ==========================================
// CONSTANTS
// ==========================================
#define EEPROM_SIZE 1024
#define CONFIG_ADDR 0
#define ALARM_ADDR 400
#define TIMER_ADDR 800
#define MAX_ALARMS 5

// ==========================================
// SYSTEM STATES
// ==========================================
enum SystemState
{
    STATE_BOOT,
    STATE_NORMAL,
    STATE_ALARM,
    STATE_COUNTDOWN,
    STATE_INFO,
    STATE_ERROR,
    STATE_MENU
};

// ==========================================
// DATA STRUCTURES
// ==========================================
struct DeviceConfig
{
    char deviceName[64] = "Smart Clock v5.3";
    char hotspotSSID[32] = "IOT_NHOM1";
    char hotspotPassword[32] = "12345678";
    bool configValid = false;
};

struct HardwareStatus
{
    bool lcdOK = false;
    bool rtcOK = false;
    bool wifiOK = false;
    bool tempOK = false;
    bool buzzerOK = false;
    bool ledOK = false;
    String lastError = "";
};

struct Alarm
{
    int hour = -1;
    int minute = -1;
    bool enabled = false;
    bool daysOfWeek[7] = {false};
    char label[32] = "";
};

struct CountdownTimer
{
    unsigned long duration = 0; // in seconds
    unsigned long startTime = 0;
    bool active = false;
    bool finished = false;
    char label[32] = "Timer";
    bool alarmTriggered = false;
    unsigned long alarmStartTime = 0;
};

struct WeatherData
{
    float temperature = 0.0;
    int humidity = 0;
    String description = "N/A";
    String city = "Thủ Đức";
    bool dataValid = false;
    unsigned long lastUpdate = 0;
    int errorCount = 0;
};

// ==========================================
// GLOBAL VARIABLES DECLARATIONS
// ==========================================
extern SystemState currentState;
extern unsigned long stateStartTime;
extern DeviceConfig config;
extern HardwareStatus hw;
extern float currentTemp;
extern float minTemp;
extern float maxTemp;
extern Alarm alarms[MAX_ALARMS];
extern int alarmCount;
extern bool alarmActive;
extern int activeAlarmIndex;
extern CountdownTimer timer;
extern WeatherData weather;
extern String currentLCDLine1;
extern String currentLCDLine2;
extern unsigned long lastLCDUpdate;
extern int lcdDisplayMode;
extern unsigned long lastLCDModeChange;

// Hardware Objects
extern LiquidCrystal_I2C LCD;
extern RTC_DS1307 rtc;
extern WiFiManager wifiManager;
extern Preferences preferences;

#endif
