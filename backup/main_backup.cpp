#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <time.h>

LiquidCrystal_I2C LCD(0x27, 16, 2);
RTC_DS1307 rtc;

#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 7 * 3600 // Việt Nam: UTC+7
#define UTC_OFFSET_DST 0

#define LED_PIN 12
#define BUZZER_PIN 25
#define LM35_PIN 34

float currentTemp = 25.0;

// Hàm chuyển đổi giá trị ADC của LM35 sang nhiệt độ
float convertAdcToTemperature(int adcValue) {
  if (adcValue == 0) return 25.0;
  
  float voltage = (adcValue / 4095.0) * 3.3;
  float tempC = voltage / 0.01;
  
  if (tempC < -10 || tempC > 100) {
    Serial.println("Warning: Temperature out of range: " + String(tempC) + "°C");
    return 25.0;
  }
  
  return tempC;
}

float readLM35Temperature() {
  int adcValue = analogRead(LM35_PIN);
  return convertAdcToTemperature(adcValue);
}

void printInfo(struct tm *timeinfo) {
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.printf("%02d:%02d:%02d %4.1fC",
             timeinfo->tm_hour,
             timeinfo->tm_min,
             timeinfo->tm_sec,
             currentTemp);

  LCD.setCursor(0, 1);
  LCD.printf("%02d/%02d/%02d",
             timeinfo->tm_mday,
             timeinfo->tm_mon + 1,
             (timeinfo->tm_year + 1900) % 100);
}

void printInfoFromRTC() {
  DateTime now = rtc.now();

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.printf("%02d:%02d:%02d %02d/%02d/%02d",
             now.hour(),
             now.minute(),
             now.second(),
             now.day(),
             now.month(),
             now.year() % 100);

  LCD.setCursor(0, 1);
  LCD.printf("Temp: %.1fC", currentTemp);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin();
  LCD.init();
  LCD.backlight();

  if (!rtc.begin()) {
    LCD.setCursor(0, 0);
    LCD.print("RTC not found!");
    Serial.println("RTC not found!");
    while (1);
  }

  currentTemp = readLM35Temperature();
  
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Connecting WiFi");
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin("Wokwi-GUEST", "", 6);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 20) {
    delay(500);
    LCD.print(".");
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      rtc.adjust(DateTime(
          timeinfo.tm_year + 1900,
          timeinfo.tm_mon + 1,
          timeinfo.tm_mday,
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec));
      Serial.println("Time synced from NTP.");
    }
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print("WiFi Connected!");
    delay(2000);
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed!");
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print("WiFi failed");
    delay(2000);
  }
  
  LCD.clear();
  Serial.println("Setup complete!");
}

void loop() {
  currentTemp = readLM35Temperature();

  if (WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      printInfo(&timeinfo);
    } else {
      printInfoFromRTC();
    }
  } else {
    printInfoFromRTC();
  }

  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  delay(500);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  delay(500);
} 