# Smart Clock v5.0 - System Flowcharts Documentation

This document provides comprehensive flowcharts for the Smart Clock system based on the function prototypes and system architecture documented in `src/doc.txt`.

## 📋 Available Flowcharts

### 1. Main Logic Flowchart (`main_logic_flowchart.puml`)
**Purpose**: Shows the overall system operation and state machine logic.

**Key Features**:
- Complete system initialization sequence
- State machine with 6 states (BOOT, NORMAL, ALARM, COUNTDOWN, MENU, ERROR)
- Main loop processing with web server handling
- Button input processing with interrupt support
- Temperature monitoring and display management
- Alarm checking and triggering logic
- Weather data management
- Configuration persistence

**States Explained**:
- `STATE_BOOT`: System initialization and hardware setup
- `STATE_NORMAL`: Default operation showing clock/weather display
- `STATE_ALARM`: Active alarm with buzzer and LED indication
- `STATE_COUNTDOWN`: Timer countdown display mode
- `STATE_MENU`: Menu navigation interface
- `STATE_ERROR`: Error handling and recovery

### 2. Alarm System Flowchart (`alarm_system_flowchart.puml`)
**Purpose**: Detailed alarm system logic including scheduling, triggering, and stopping.

**Key Features**:
- Alarm loading from EEPROM on boot
- Continuous alarm checking against current time
- Day-of-week scheduling support
- Dual button handling (interrupt + polling)
- 5-minute auto-stop timeout
- Web interface alarm management
- Persistent storage of alarm changes

**Critical Paths**:
- Immediate buzzer stop via GPIO 26 interrupt
- Alarm validation and scheduling logic
- Web-based alarm configuration handling

### 3. Hardware Control Flowchart (`hardware_control_flowchart.puml`)
**Purpose**: Hardware interaction and GPIO control logic.

**Key Features**:
- Complete GPIO pin configuration
- I2C communication with LCD and RTC
- Temperature sensor reading and calibration
- Button debouncing and long-press detection
- Buzzer and LED control for alarms
- LCD display mode management
- Interrupt service routine for immediate response

**Hardware Pins**:
- GPIO 12: LED indicator
- GPIO 25: Buzzer output
- GPIO 26: Button input (with pullup + interrupt)
- GPIO 34: Temperature sensor (analog)
- I2C: LCD (0x27) and RTC (DS1307)

### 4. Web Interface Flowchart (`web_interface_flowchart.puml`)
**Purpose**: Web server and HTTP API logic.

**Key Features**:
- WiFi setup with captive portal
- Complete HTTP server with multiple endpoints
- Real-time status updates via JavaScript polling
- Form handling for alarm and timer management
- JSON API for system status
- Weather configuration management
- Factory reset via web interface

**API Endpoints**:
- `GET /`: Main web interface
- `POST /set-alarm`: Add/update alarm
- `POST /delete-alarm`: Remove alarm
- `POST /set-timer`: Start countdown timer
- `GET /status`: System status JSON
- `GET /alarms`: Current alarms JSON
- `GET /weather`: Weather data JSON

## 🔧 How to Use These Flowcharts

### For Development:
1. **Planning**: Use flowcharts to understand system architecture
2. **Debugging**: Follow execution paths to identify issues
3. **Testing**: Verify all paths and states are covered
4. **Documentation**: Reference for code reviews and maintenance

### For Visualization:
1. **PlantUML**: Use PlantUML to render the `.puml` files
2. **VS Code**: Install PlantUML extension for inline viewing
3. **Online**: Use PlantUML online editor at plantuml.com
4. **Export**: Generate PNG/SVG files for documentation

### For Integration:
1. **State Machine**: Follow main logic flowchart for state transitions
2. **Interrupt Handling**: Reference hardware flowchart for GPIO logic
3. **Web API**: Use web interface flowchart for HTTP endpoint development
4. **Alarm Logic**: Follow alarm flowchart for scheduling implementation

## 🎯 Key System Features Highlighted

### Immediate Buzzer Control
- **Hardware Interrupt**: GPIO 26 interrupt for instant response
- **Polling Backup**: Main loop button handling as fallback
- **Debouncing**: Both methods include debounce logic
- **Priority**: Alarm stop has highest priority in system

### Dual Display Modes
- **Auto-switching**: Clock and weather modes alternate every 60 seconds
- **Real-time Updates**: Temperature and time updated continuously
- **Status Indicators**: Visual feedback for system health

### Web Interface Integration
- **Real-time Updates**: JavaScript polling every 2 seconds
- **Mobile Responsive**: Works on all device sizes
- **Vietnamese UI**: Localized interface for user-friendly operation
- **API-first**: RESTful endpoints for all operations

### Robust Error Handling
- **Hardware Failures**: Graceful degradation and recovery
- **Network Issues**: Fallback modes and retry logic
- **Data Corruption**: Validation and default value handling
- **State Recovery**: Automatic return to normal operation

## 📁 File Structure

```
docs/
├── main_logic_flowchart.puml          # Overall system logic
├── alarm_system_flowchart.puml        # Alarm scheduling and control
├── hardware_control_flowchart.puml    # GPIO and hardware interaction
├── web_interface_flowchart.puml       # Web server and API logic
└── FLOWCHART_DOCUMENTATION.md         # This documentation file
```

## 🔄 System Integration Points

### Between Main Logic and Alarm System:
- State transitions trigger alarm checking
- Alarm events change system state
- Button handling affects both systems

### Between Hardware and Web Interface:
- Web commands trigger hardware actions
- Hardware status reported via web API
- Configuration changes affect hardware behavior

### Between All Systems:
- Shared global variables and state
- Common configuration storage (EEPROM/Preferences)
- Unified error handling and recovery

## 📝 Development Notes

### For Adding New Features:
1. Update relevant flowchart first
2. Identify integration points with existing systems
3. Consider state machine implications
4. Plan web interface updates
5. Test all execution paths

### For Debugging:
1. Identify current system state
2. Follow flowchart execution path
3. Check critical decision points
4. Verify hardware pin states
5. Review web interface logs

### For Maintenance:
1. Keep flowcharts updated with code changes
2. Document new states or transitions
3. Update hardware pin assignments
4. Maintain API endpoint documentation
5. Review and update integration points

---

*This documentation corresponds to Smart Clock v5.0 firmware as documented in `src/doc.txt` and implemented in `src/main1.cpp`.*
