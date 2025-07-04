#ifndef ALARM_H
#define ALARM_H

#include "config.h"
#include <RTClib.h>

// ==========================================
// ALARM & TIMER SYSTEM
// ==========================================
void triggerAlarm(int index);
void stopAlarm();
void updateAlarmDisplay();
void checkAlarms();
void handleTimerAlarm();

#endif
