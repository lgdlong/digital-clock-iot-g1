#include "alarm.h"
#include "display.h"

extern RTC_DS1307 rtc;
extern LiquidCrystal_I2C LCD;

void triggerAlarm(int index)
{
    activeAlarmIndex = index;
    alarmActive = true;
    timer.finished = false;
    currentState = STATE_ALARM;
    stateStartTime = millis();
}

void stopAlarm()
{
    alarmActive = false;
    timer.active = false;
    timer.finished = false;
    timer.alarmTriggered = false;
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

    if (millis() - stateStartTime > 5 * 60 * 1000)
    {
        stopAlarm();
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

void handleTimerAlarm()
{
    if (timer.alarmTriggered)
    {
        unsigned long alarmElapsed = millis() - timer.alarmStartTime;

        if (alarmElapsed < 5000)
        {
            static unsigned long lastBlinkTimer = 0;
            if (millis() - lastBlinkTimer > 250)
            {
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
            timer.alarmTriggered = false;
            timer.finished = false;
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(LED_PIN, LOW);
            Serial.println("=== COUNTDOWN ALARM FINISHED ===");
        }
    }
}
