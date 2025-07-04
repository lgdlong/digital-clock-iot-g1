#include "serial_commands.h"
#include "utils.h"
#include <RTClib.h>

void handleSerialCommands()
{
    if (!Serial.available())
        return;

    String command = Serial.readString();
    command.trim();
    command.toLowerCase();

    if (command == "help")
    {
        Serial.println("📋 Available commands:");
        Serial.println("  help - Show this help");
        Serial.println("  status - Show device status");
        Serial.println("  alarms - Show all alarms");
        Serial.println("  add - Add new alarm");
        Serial.println("  timer - Start countdown timer");
        Serial.println("  weather - Show weather info");
        Serial.println("  reset - Factory reset");
    }
    else if (command == "status")
    {
        DateTime now = rtc.now();
        Serial.println("📊 Device Status:");
        Serial.println("  Time: " + String(now.hour()) + ":" + String(now.minute()));
        Serial.println("  Temp: " + String(currentTemp, 1) + "°C");
        Serial.println("  WiFi: " + String(hw.wifiOK ? "Connected" : "Disconnected"));
        Serial.println("  Alarms: " + String(alarmCount));
        Serial.println("  Timer: " + String(timer.active ? "Running" : "Stopped"));
    }
    else if (command == "alarms")
    {
        Serial.println("📋 Current alarms:");
        if (alarmCount == 0)
        {
            Serial.println("  No alarms set");
        }
        else
        {
            for (int i = 0; i < alarmCount; i++)
            {
                Serial.print("  " + String(i + 1) + ". ");
                Serial.print(String(alarms[i].hour) + ":" + (alarms[i].minute < 10 ? "0" : "") + String(alarms[i].minute));
                Serial.print(" - " + String(alarms[i].label));
                Serial.println(alarms[i].enabled ? " (ON)" : " (OFF)");
            }
        }
    }
    else if (command == "add")
    {
        if (alarmCount >= MAX_ALARMS)
        {
            Serial.println("❌ Maximum alarms reached");
            return;
        }

        Serial.println("➕ Add new alarm:");
        Serial.print("Enter hour (0-23): ");
        while (!Serial.available())
            delay(10);
        int hour = Serial.parseInt();

        Serial.print("Enter minute (0-59): ");
        while (!Serial.available())
            delay(10);
        int minute = Serial.parseInt();

        Serial.print("Enter label (optional): ");
        while (!Serial.available())
            delay(10);
        String label = Serial.readString();
        label.trim();

        alarms[alarmCount].hour = hour;
        alarms[alarmCount].minute = minute;
        alarms[alarmCount].enabled = true;
        strncpy(alarms[alarmCount].label, label.c_str(), sizeof(alarms[alarmCount].label) - 1);

        for (int i = 0; i < 7; i++)
        {
            alarms[alarmCount].daysOfWeek[i] = true;
        }

        alarmCount++;
        // saveAlarms();

        Serial.println("✅ Alarm added: " + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute));
    }
    else if (command == "timer")
    {
        if (timer.active)
        {
            Serial.println("❌ Timer already running");
            return;
        }

        Serial.print("Enter duration in minutes: ");
        while (!Serial.available())
            delay(10);
        int minutes = Serial.parseInt();

        Serial.print("Enter label (optional): ");
        while (!Serial.available())
            delay(10);
        String label = Serial.readString();
        label.trim();

        timer.duration = minutes * 60;
        timer.startTime = millis();
        timer.active = true;
        strncpy(timer.label, label.c_str(), sizeof(timer.label) - 1);
        currentState = STATE_COUNTDOWN;

        Serial.println("✅ Timer started: " + String(minutes) + " minutes");
    }
    else if (command == "weather")
    {
        Serial.println("🌤️ Weather Info:");
        if (weather.dataValid)
        {
            Serial.println("  City: " + weather.city);
            Serial.println("  Temp: " + String(weather.temperature, 1) + "°C");
            Serial.println("  Humidity: " + String(weather.humidity) + "%");
            Serial.println("  Description: " + weather.description);
        }
        else
        {
            Serial.println("  No weather data available");
        }
    }
    else if (command == "reset")
    {
        Serial.println("⚠️ Factory reset in 3 seconds...");
        delay(3000);
        factoryReset();
    }
    else
    {
        Serial.println("❌ Unknown command. Type 'help' for available commands");
    }
}
