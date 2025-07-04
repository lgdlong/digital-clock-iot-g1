#include "wifi_manager.h"

void setupWiFi()
{
    Serial.println("Setting up WiFi...");

    bool connected = wifiManager.autoConnect(config.hotspotSSID, config.hotspotPassword);

    if (connected)
    {
        Serial.println("WiFi connected: " + WiFi.localIP().toString());
        hw.wifiOK = true;
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(config.hotspotSSID, config.hotspotPassword);
        Serial.println("Hotspot: " + String(config.hotspotSSID));
    }
    else
    {
        Serial.println("WiFi failed, hotspot only");
        hw.wifiOK = false;
        WiFi.mode(WIFI_AP);
        WiFi.softAP(config.hotspotSSID, config.hotspotPassword);
    }
}
