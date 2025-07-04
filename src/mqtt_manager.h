#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "config.h"
#include <PubSubClient.h>

// ==========================================
// MQTT FUNCTIONS
// ==========================================
void mqttCallback(char *topic, byte *payload, unsigned int length);
void publishCurrentTime();
void connectMQTT();

// MQTT Variables
extern const char *mqttServers[];
extern const int mqttPort;
extern const char *mqttUser;
extern const char *mqttPassword;
extern int currentBrokerIndex;
extern const char *timePublishTopic;
extern const char *resetSubscribeTopic;
extern WiFiClient espClient;
extern PubSubClient mqttClient;

#endif
