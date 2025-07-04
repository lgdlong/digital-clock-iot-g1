#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include <Preferences.h>
#include <EEPROM.h>

// ==========================================
// UTILITY FUNCTIONS
// ==========================================
bool checkFirstBoot();
void clearAllData();
float convertAdcToTemperature(int adcValue);
void factoryReset();

#endif
