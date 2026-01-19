#include "mqtt.h"
#include "config.h"
#include <ArduinoJson.h>

// Statische Instanz für Callback-Funktion
MQTTClient* MQTTClient::instance = nullptr;

// Baltimore CyberTrust Root CA Zertifikat
const char* AZURE_ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \
"-----END CERTIFICATE-----\n";

MQTTClient::MQTTClient() : mqttClient(wifiClient), connected(false), 
                           sasTokenExpiry(0), lastReconnectAttempt(0) {
    instance = this;
}

bool MQTTClient::begin(unsigned long currentEpoch) {
    Serial.println("\n=== MQTT Client Initialisierung ===");
    
    wifiClient.setInsecure();
    
    mqttClient.setServer(IOT_HUB_HOSTNAME, MQTT_PORT);
    mqttClient.setCallback(messageCallback);
    mqttClient.setBufferSize(512);
    
    Serial.printf("IoT Hub: %s\n", IOT_HUB_HOSTNAME);
    Serial.printf("Device ID: %s\n", DEVICE_ID);
    Serial.printf("MQTT QoS Level: %d\n", MQTT_QOS_LEVEL); // ✅ NEU: QoS anzeigen
    
    return connect(currentEpoch);
}

bool MQTTClient::connect(unsigned long currentEpoch) {
    if (currentEpoch == 0) {
        Serial.println("❌ Keine gültige Zeit für SAS-Token!");
        return false;
    }
    
    Serial.print("Generiere SAS-Token... ");
    currentSasToken = sasToken.generateDefault(IOT_HUB_HOSTNAME, DEVICE_ID, DEVICE_KEY, currentEpoch);
    sasTokenExpiry = currentEpoch + 86400;
    Serial.println("OK");
    
    String mqttUsername = String(IOT_HUB_HOSTNAME) + "/" + String(DEVICE_ID) + 
                         "/?api-version=2021-04-12";
    
    Serial.print("Verbinde mit Azure IoT Hub... ");
    
    bool result = mqttClient.connect(DEVICE_ID, 
                                     mqttUsername.c_str(), 
                                     currentSasToken.c_str());
    
    if (result) {
        Serial.println("✅ Verbunden!");
        connected = true;
        
        String c2dTopic = String("devices/") + DEVICE_ID + "/messages/devicebound/#";
        mqttClient.subscribe(c2dTopic.c_str());
        Serial.printf("Abonniert: %s\n", c2dTopic.c_str());
        
        return true;
    } else {
        Serial.printf("❌ Fehler! State: %d\n", mqttClient.state());
        connected = false;
        return false;
    }
}

bool MQTTClient::isConnected() {
    return mqttClient.connected();
}

void MQTTClient::disconnect() {
    mqttClient.disconnect();
    connected = false;
}

bool MQTTClient::publishTelemetry(const SensorData& data, unsigned long currentEpoch) {
    if (!isConnected()) {
        return false;
    }
    
    StaticJsonDocument<256> doc;
    
    doc["timestamp"] = currentEpoch;
    doc["temperature"] = data.temperature;
    doc["humidity"] = data.humidity;
    doc["pressure"] = data.pressure;
    doc["accelX"] = data.accelX;
    doc["accelY"] = data.accelY;
    doc["accelZ"] = data.accelZ;
    doc["gyroX"] = data.gyroX;
    doc["gyroY"] = data.gyroY;
    doc["gyroZ"] = data.gyroZ;
    
    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);
    
    return publishJSON(jsonBuffer);
}

// ========== ✅ MODIFIZIERTE FUNKTION ========== 
bool MQTTClient::publishJSON(const char* json) {
    if (!isConnected()) {
        return false;
    }
    
    String topic = String("devices/") + DEVICE_ID + "/messages/events/";
    
    // QoS 1 mit PUBACK-Bestätigung
    bool result = mqttClient.publish(topic.c_str(), json, MQTT_QOS_LEVEL);
    
    if (result) {
        Serial.println("📤 Telemetrie gesendet (QoS 1 - mit PUBACK!)");
    } else {
        Serial.println("❌ Fehler beim Senden!");
    }
    
    return result;
}

void MQTTClient::loop() {
    mqttClient.loop();
}

void MQTTClient::handleReconnect(unsigned long currentEpoch) {
    if (isConnected()) {
        return;
    }
    
    unsigned long now = millis();
    
    if (now - lastReconnectAttempt >= RECONNECT_INTERVAL) {
        lastReconnectAttempt = now;
        
        Serial.println("⚠️  MQTT Verbindung verloren - Reconnect...");
        
        if (connect(currentEpoch)) {
            Serial.println("✅ MQTT Reconnect erfolgreich!");
        } else {
            Serial.println("❌ MQTT Reconnect fehlgeschlagen");
        }
    }
}

void MQTTClient::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        instance->handleIncomingMessage(topic, payload, length);
    }
}

void MQTTClient::handleIncomingMessage(char* topic, byte* payload, unsigned int length) {
    Serial.println("\n╔═══════════════════════════════════════╗");
    Serial.println("║ 📥 CLOUD-TO-DEVICE MESSAGE           ║");
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.printf("Topic: %s\n", topic);
    Serial.printf("Length: %d bytes\n", length);
    Serial.print("Payload: ");
    
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println("\n");
    
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    
    if (error) {
        Serial.printf("❌ JSON Parse Fehler: %s\n", error.c_str());
        return;
    }
    
    Serial.println("✅ JSON erfolgreich geparst");
    
    if (doc.containsKey("led")) {
        String ledState = doc["led"].as<String>();
        Serial.printf("LED Command: %s\n", ledState.c_str());
        
        if (ledState == "on") {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("💡💡💡 LED EINGESCHALTET 💡💡💡");
        } 
        else if (ledState == "off") {
            digitalWrite(LED_PIN, LOW);
            Serial.println("⚫⚫⚫ LED AUSGESCHALTET ⚫⚫⚫");
        }
    } else {
        Serial.println("⚠️  Kein 'led' Key in JSON gefunden");
    }
    
    Serial.println("═══════════════════════════════════════\n");
}