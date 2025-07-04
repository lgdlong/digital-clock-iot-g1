#include "utils.h"
#include <WiFiManager.h>

extern Preferences preferences;
extern WiFiManager wifiManager;

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

    for (int i = 0; i < 512; i++)
    {
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

float convertAdcToTemperature(int adcValue)
{
    if (adcValue == 0)
        return 25.0;
    float voltage = (adcValue / 4095.0) * 3.3;
    float tempC = voltage / 0.01;
    if (tempC < -10 || tempC > 100)
    {
        return 25.0;
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
