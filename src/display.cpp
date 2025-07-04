#include "display.h"

extern LiquidCrystal_I2C LCD;
extern RTC_DS1307 rtc;

void updateLCDContent(String line1, String line2)
{
    if (line1 != currentLCDLine1 || line2 != currentLCDLine2)
    {
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

void displayClock()
{
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();

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
        char line1[17];
        char line2[17];

        if (weather.dataValid)
        {
            snprintf(line1, sizeof(line1), "%s %.1f°C",
                     weather.city.substring(0, 8).c_str(), weather.temperature);
            snprintf(line2, sizeof(line2), "%s %d%%",
                     weather.description.substring(0, 10).c_str(), weather.humidity);
        }
        else
        {
            strcpy(line1, "Thoi Tiet");
            strcpy(line2, "Khong co du lieu");
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

void displayMenu()
{
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000)
    {
        lastUpdate = millis();
        updateLCDContent("MENU MODE", "Press to exit");
    }

    if (millis() - stateStartTime > 10000)
    {
        currentState = STATE_NORMAL;
    }
}
