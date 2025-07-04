#include "config_manager.h"
#include <EEPROM.h>

void loadConfiguration()
{
    EEPROM.get(CONFIG_ADDR, config);
    if (!config.configValid)
    {
        strcpy(config.deviceName, "Smart Clock v5.3");
        strcpy(config.hotspotSSID, "IOT_NHOM1");
        strcpy(config.hotspotPassword, "12345678");
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
