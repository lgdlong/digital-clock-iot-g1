/*
 * ESP32 Buzzer Control with Button
 * Simple implementation for immediate buzzer shutoff
 */

#include <Arduino.h>

// Pin definitions
#define BUZZER_PIN 25
#define BUTTON_PIN 26

// Forward function declarations
void IRAM_ATTR buttonInterrupt();
void handleButtonPolling();
void startTestAlarm();
void stopAlarm();
void triggerAlarm();
bool isAlarmActive();

// Debouncing variables
volatile unsigned long lastInterruptTime = 0;
volatile bool buzzerStopRequested = false;

// Alarm state variables
bool alarmActive = false;
unsigned long alarmStartTime = 0;

void setup() {
  Serial.begin(115200);
  
  // Configure GPIO pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Ensure buzzer is off initially
  digitalWrite(BUZZER_PIN, LOW);
  
  // Attach interrupt for immediate buzzer control
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, FALLING);
  
  Serial.println("ESP32 Buzzer Control Ready");
  Serial.println("Press button to stop alarm when buzzer is sounding");
  
  // Start a test alarm after 3 seconds
  delay(3000);
  startTestAlarm();
}

void loop() {
  // Handle interrupt-requested buzzer stop
  if (buzzerStopRequested) {
    buzzerStopRequested = false;
    stopAlarm();
    Serial.println("Alarm stopped by interrupt!");
  }
  
  // Handle alarm timing and automatic shutoff
  if (alarmActive) {
    // Auto-stop alarm after 30 seconds if not manually stopped
    if (millis() - alarmStartTime > 30000) {
      stopAlarm();
      Serial.println("Alarm auto-stopped after 30 seconds");
    }
    
    // Blink pattern for visual indication
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
      lastBlink = millis();
      static bool ledState = false;
      ledState = !ledState;
      
      // You can add an LED on another pin for visual feedback
      // digitalWrite(LED_PIN, ledState);
    }
  }
  
  // Non-interrupt button handling (backup method)
  handleButtonPolling();
  
  // Start new test alarm every 60 seconds if no alarm is active
  static unsigned long lastTestAlarm = 0;
  if (!alarmActive && millis() - lastTestAlarm > 60000) {
    lastTestAlarm = millis();
    startTestAlarm();
  }
  
  delay(50); // Small delay to prevent excessive polling
}

// Interrupt Service Routine - executes immediately when button is pressed
void IRAM_ATTR buttonInterrupt() {
  unsigned long currentTime = millis();
  
  // Debouncing: ignore interrupts within 50ms of the last one
  if (currentTime - lastInterruptTime > 50) {
    lastInterruptTime = currentTime;
    
    // Check if button is actually pressed (LOW due to INPUT_PULLUP)
    if (digitalRead(BUTTON_PIN) == LOW) {
      // Immediately turn off buzzer if it's currently sounding
      if (digitalRead(BUZZER_PIN) == HIGH) {
        digitalWrite(BUZZER_PIN, LOW);
        buzzerStopRequested = true;
      }
    }
  }
}

// Non-interrupt button handling (backup method with debouncing)
void handleButtonPolling() {
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  static bool buttonState = HIGH;
  
  // Read the current button state
  bool reading = digitalRead(BUTTON_PIN);
  
  // Check if button state changed (for debouncing)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // If the button state has been stable for debounce delay
  if ((millis() - lastDebounceTime) > 50) {
    // If the button state has actually changed
    if (reading != buttonState) {
      buttonState = reading;
      
      // Button is pressed (LOW due to pull-up)
      if (buttonState == LOW) {
        // If buzzer is currently ON, turn it off immediately
        if (digitalRead(BUZZER_PIN) == HIGH) {
          digitalWrite(BUZZER_PIN, LOW);
          stopAlarm();
          Serial.println("Alarm stopped by polling!");
        }
      }
    }
  }
  
  lastButtonState = reading;
}

// Start a test alarm
void startTestAlarm() {
  if (!alarmActive) {
    alarmActive = true;
    alarmStartTime = millis();
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("TEST ALARM STARTED - Press button to stop!");
  }
}

// Stop the alarm
void stopAlarm() {
  if (alarmActive) {
    alarmActive = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("Alarm stopped");
  }
}

// Additional utility function to manually trigger alarm for testing
void triggerAlarm() {
  startTestAlarm();
}

// Function to check if alarm is currently active
bool isAlarmActive() {
  return alarmActive;
}
