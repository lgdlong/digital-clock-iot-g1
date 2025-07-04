#include "mqtt_manager.h"
#include <RTClib.h>

// MQTT Variables
const char *mqttServers[] = {"broker.emqx.io", "test.mosquitto.org"};
const int mqttPort = 1883;
const char *mqttUser = "";
const char *mqttPassword = "";
int currentBrokerIndex = 0;
const char *timePublishTopic = "clock/time";
const char *resetSubscribeTopic = "clock/reset";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message = "";
    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    Serial.println("MQTT message received on topic: " + String(topic) + " - Message: " + message);
    if (String(topic) == resetSubscribeTopic && message == "reset")
    {
        publishCurrentTime();
    }
}

void publishCurrentTime()
{
    if (!mqttClient.connected())
        return;
    DateTime now = rtc.now();
    char timeBuffer[9];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    String timePayload = String(timeBuffer);
    if (mqttClient.publish(timePublishTopic, timePayload.c_str()))
    {
        Serial.println("Time published to MQTT: " + timePayload);
    }
    else
    {
        Serial.println("Failed to publish time to MQTT");
    }
}

void connectMQTT()
{
    int retry = 0;
    while (!mqttClient.connected() && WiFi.status() == WL_CONNECTED)
    {
        Serial.print("Attempting MQTT connection to ");
        Serial.print(mqttServers[currentBrokerIndex]);
        Serial.print(":");
        Serial.print(mqttPort);
        Serial.print("...");
        String clientId = "ESP32Clock-" + String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword))
        {
            Serial.println(" connected!");
            Serial.println("Client ID: " + clientId);
            mqttClient.subscribe(resetSubscribeTopic);
            Serial.println("Subscribed to: " + String(resetSubscribeTopic));
            Serial.println("Will publish to: " + String(timePublishTopic));
        }
        else
        {
            Serial.print(" failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
            retry++;
            if (retry >= 3 && currentBrokerIndex == 0)
            {
                Serial.println("Switching to backup MQTT broker: test.mosquitto.org");
                currentBrokerIndex = 1;
                retry = 0;
            }
            else if (retry >= 3 && currentBrokerIndex == 1)
            {
                Serial.println("All MQTT brokers failed. Will keep retrying...");
                retry = 0;
            }
        }
    }
}
