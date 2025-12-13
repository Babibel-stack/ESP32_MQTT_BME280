#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "sensors.h"
#include "sas.h"

class MQTTClient {
private:
    WiFiClientSecure wifiClient;
    PubSubClient mqttClient;
    SASToken sasToken;
    
    String currentSasToken;
    unsigned long sasTokenExpiry;
    unsigned long lastReconnectAttempt;
    
    const unsigned long RECONNECT_INTERVAL = 5000;  // 5 Sekunden
    const int MQTT_PORT = 8883;  // Azure IoT Hub MQTT Port (TLS)
    
    bool connected;
    
    // Callback für eingehende Messages
    static void messageCallback(char* topic, byte* payload, unsigned int length);
    static MQTTClient* instance;  // Für Callback
    
    void handleIncomingMessage(char* topic, byte* payload, unsigned int length);
    
public:
    MQTTClient();
    
    bool begin(unsigned long currentEpoch);
    bool connect(unsigned long currentEpoch);
    bool isConnected();
    void disconnect();
    
    bool publishTelemetry(const SensorData& data, unsigned long currentEpoch);
    bool publishJSON(const char* json);
    
    void loop();  // Muss in main loop() aufgerufen werden
    void handleReconnect(unsigned long currentEpoch);
};

#endif