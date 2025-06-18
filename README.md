# IoT Digital Clock

This is a digital clock project using ESP32 with real-time synchronization from NTP servers and a backup RTC module. The project was developed for the IOT102 course.

## Features

> Read more about the key features in [KEY_FEATURES.md](KEY_FEATURES.md)

<!-- - Real-time clock display on 16x2 LCD screen
- Date display (DD/MM/YY)
- Temperature display (simulated value)
- NTP synchronization when WiFi is connected
- DS1307 RTC module as backup when WiFi is unavailable
- Status LED indicator (blinks every second) -->

## Hardware Components

- ESP32 Development Board (ESP32-DEVKIT-C)
- 16x2 LCD Display with I2C interface
- DS1307 RTC Module
- LED indicator
- Connecting wires

## Pin Connections

| Component   | Connection                                        |
| ----------- | ------------------------------------------------- |
| LCD Display | SDA → GPIO21, SCL → GPIO22, VCC → 3.3V, GND → GND |
| DS1307 RTC  | SDA → GPIO21, SCL → GPIO22, VCC → 3.3V, GND → GND |
| Status LED  | Anode → GPIO12, Cathode → GND                     |

## Software Dependencies

- Arduino framework for ESP32
- WiFi library
- Wire library (I2C communication)
- LiquidCrystal_I2C library
- RTCLib by Adafruit

## Installation and Setup

1. Clone this repository
2. Open the project in PlatformIO (recommended) or Arduino IDE
3. Install required libraries:
   - LiquidCrystal_I2C (by Marco Schwartz)
   - RTCLib (by Adafruit)
4. Modify the WiFi credentials in the code if needed
5. Upload the code to your ESP32

## Time Zone Configuration

The default time zone is set to UTC+7 (Vietnam). To change the time zone:

```cpp
#define UTC_OFFSET 7 * 3600  // Change the number 7 to your UTC offset
```

## Project Structure

- `/src/main.cpp`: Main application code
- `/include`: Header files
- `/lib`: Project-specific libraries
- `/platformio.ini`: PlatformIO configuration
- `/diagram.json`: Wokwi simulation diagram

## Simulation

This project can be simulated using Wokwi. The diagram.json file contains the component connections for the simulation.

## Docs

[IOT_Project_G1 - Google Tài liệu](https://docs.google.com/document/d/1ETpLSW7EK4e8zNShtvTOpb-vyrWY-4vWwZAHBhT7I4s/edit?usp=sharing)

## Authors

- Group 1 - IOT102
