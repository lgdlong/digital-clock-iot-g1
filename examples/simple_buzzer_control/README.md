# Simple ESP32 Buzzer Control Example

This is a standalone example demonstrating immediate buzzer shutoff using button interrupts.

## Hardware Setup
- **Buzzer**: Connected to GPIO 25 (positive terminal to GPIO 25, negative to GND)
- **Button**: Connected to GPIO 26 (one terminal to GPIO 26, other to GND)
- **Power**: ESP32 powered via USB or external power

## Features
- **Immediate Response**: Uses hardware interrupt for instant buzzer shutoff
- **Debouncing**: Software debouncing prevents false triggers
- **Auto-stop**: Alarm stops automatically after 30 seconds
- **Test Mode**: Automatically triggers test alarms every 60 seconds

## How to Use
1. Upload this code to your ESP32
2. Open Serial Monitor at 115200 baud
3. The buzzer will start after 3 seconds
4. Press the button to immediately stop the buzzer
5. New test alarm will start every 60 seconds

## Code Flow
1. **Setup**: Initialize pins and attach interrupt
2. **Loop**: Handle alarm timing and button polling (backup)
3. **Interrupt**: Immediate buzzer shutoff when button pressed
4. **Functions**: Start/stop alarm management

## Pin Configuration
```cpp
#define BUZZER_PIN 25  // GPIO 25 for buzzer
#define BUTTON_PIN 26  // GPIO 26 for button (INPUT_PULLUP)
```
