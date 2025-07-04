#include "button.h"
#include "utils.h"
#include "alarm.h"

void handleButton()
{
    static unsigned long lastPress = 0;
    static bool lastState = HIGH;
    static unsigned long pressStartTime = 0;

    bool buttonState = digitalRead(BUTTON_PIN);

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
                switch (currentState)
                {
                case STATE_ALARM:
                    stopAlarm();
                    break;
                case STATE_NORMAL:
                    if (timer.alarmTriggered)
                    {
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
