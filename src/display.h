#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// ==========================================
// DISPLAY FUNCTIONS
// ==========================================
void updateLCDContent(String line1, String line2);
void displayClock();
void displayCountdown();
void displayMenu();

#endif
