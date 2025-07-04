#include "weather.h"

void fetchWeatherData()
{
    unsigned long now = millis();
    if (now - weather.lastUpdate > 600000)
    { // Update every 10 minutes
        weather.temperature = currentTemp + random(-3, 4);
        weather.humidity = random(40, 90);
        weather.description = "Mô phỏng";
        weather.city = "Thủ Đức";
        weather.dataValid = true;
        weather.lastUpdate = now;
        weather.errorCount = 0;
    }
}
