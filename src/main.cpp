#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C LCD(0x27, 16, 2);
RTC_DS1307 rtc;

#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 7 * 3600 // Việt Nam: UTC+7
#define UTC_OFFSET_DST 0

#define LED_PIN 12

bool syncedFromNTP = false;

// Nhiệt độ giả lập, sau này thay bằng LM35
float fakeTemp = 30.5;

void printInfo(struct tm *timeinfo)
{
  LCD.setCursor(0, 0); // Hàng 1: Giờ + Nhiệt độ
  LCD.printf("%02d:%02d:%02d Temp:%.1fC",
             timeinfo->tm_hour,
             timeinfo->tm_min,
             timeinfo->tm_sec,
             fakeTemp);

  LCD.setCursor(0, 1); // Hàng 2: Ngày/Tháng/Năm
  LCD.printf("%02d/%02d/%02d",
             timeinfo->tm_mday,
             timeinfo->tm_mon + 1,
             (timeinfo->tm_year + 1900) % 100);
}

void printInfoFromRTC()
{
  DateTime now = rtc.now();

  LCD.setCursor(0, 0); // Hàng 1: Giờ + ngày
  LCD.printf("%02d:%02d:%02d %02d/%02d/%02d",
             now.hour(),
             now.minute(),
             now.second(),
             now.day(),
             now.month(),
             now.year() % 100);

  LCD.setCursor(0, 1); // Hàng 2: Nhiệt độ
  LCD.printf("Temp: %.1fC", fakeTemp);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin();
  LCD.init();
  LCD.backlight();

  if (!rtc.begin())
  {
    LCD.setCursor(0, 0);
    LCD.print("RTC not found!");
    while (1)
      ;
  }

  LCD.setCursor(0, 0);
  LCD.print("Connecting WiFi");
  WiFi.begin("Wokwi-GUEST", "", 6);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 20)
  {
    delay(500);
    LCD.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      rtc.adjust(DateTime(
          timeinfo.tm_year + 1900,
          timeinfo.tm_mon + 1,
          timeinfo.tm_mday,
          timeinfo.tm_hour,
          timeinfo.tm_min,
          timeinfo.tm_sec));
      syncedFromNTP = true;
      Serial.println("Time synced from NTP.");
    }
    LCD.clear();
  }
  else
  {
    LCD.clear();
    LCD.print("WiFi failed");
    delay(2000);
    LCD.clear();
  }
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      printInfo(&timeinfo);
    }
    else
    {
      printInfoFromRTC(); // fallback nếu getLocalTime() fail
    }
  }
  else
  {
    printInfoFromRTC();
  }

  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
