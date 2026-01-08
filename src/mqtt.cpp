#include "mqtt.h"
#include "config.h"
#include <ArduinoJson.h>

// Statische Instanz für Callback-Funktion (ermöglicht Zugriff aus statischer Methode)
MQTTClient* MQTTClient::instance = nullptr;

// Baltimore CyberTrust Root CA Zertifikat (für sichere TLS-Verbindung zu Azure IoT Hub)
// Dieses Zertifikat wird von Azure IoT Hub verwendet
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

// Konstruktor: Initialisiert alle Member-Variablen und setzt statische Instanz
MQTTClient::MQTTClient() : mqttClient(wifiClient), connected(false), 
                           sasTokenExpiry(0), lastReconnectAttempt(0) {
    instance = this;  // Für statischen Callback-Zugriff
}

// Initialisiert den MQTT-Client mit Zertifikat und Verbindungsparametern
bool MQTTClient::begin(unsigned long currentEpoch) {
    Serial.println("\n=== MQTT Client Initialisierung ===");
    
    // TLS-Konfiguration: setInsecure() deaktiviert Zertifikatsprüfung
    // ACHTUNG: Nur für Entwicklung/Tests! In Produktion sollte setCACert() verwendet werden
    wifiClient.setInsecure();
    
    // MQTT Broker-Einstellungen konfigurieren
    mqttClient.setServer(IOT_HUB_HOSTNAME, MQTT_PORT);  // Azure IoT Hub Hostname und Port (8883 für TLS)
    mqttClient.setCallback(messageCallback);             // Callback für eingehende Messages
    mqttClient.setBufferSize(512);                       // Buffer-Größe für größere JSON-Payloads erhöhen
    
    // Debug-Ausgabe der Verbindungsparameter
    Serial.printf("IoT Hub: %s\n", IOT_HUB_HOSTNAME);
    Serial.printf("Device ID: %s\n", DEVICE_ID);
    
    // Verbindungsaufbau starten
    return connect(currentEpoch);
}

// Stellt MQTT-Verbindung zu Azure IoT Hub her mit SAS-Token-Authentifizierung
bool MQTTClient::connect(unsigned long currentEpoch) {
    // Prüfung ob gültige Zeit verfügbar ist (notwendig für SAS-Token)
    if (currentEpoch == 0) {
        Serial.println("❌ Keine gültige Zeit für SAS-Token!");
        return false;
    }
    
    // SAS-Token (Shared Access Signature) für Authentifizierung generieren
    Serial.print("Generiere SAS-Token... ");
    currentSasToken = sasToken.generateDefault(IOT_HUB_HOSTNAME, DEVICE_ID, DEVICE_KEY, currentEpoch);
    sasTokenExpiry = currentEpoch + 86400;  // Token gültig für 24 Stunden
    Serial.println("OK");
    
    // MQTT Username nach Azure IoT Hub Schema aufbauen
    // Format: {iothubhostname}/{device_id}/?api-version=2021-04-12
    String mqttUsername = String(IOT_HUB_HOSTNAME) + "/" + String(DEVICE_ID) + 
                         "/?api-version=2021-04-12";
    
    Serial.print("Verbinde mit Azure IoT Hub... ");
    
    // MQTT Verbindungsversuch mit:
    // - Client ID: Device ID
    // - Username: IoT Hub spezifischer Username
    // - Password: SAS-Token
    bool result = mqttClient.connect(DEVICE_ID, 
                                     mqttUsername.c_str(), 
                                     currentSasToken.c_str());
    
    if (result) {
        Serial.println("✅ Verbunden!");
        connected = true;
        
        // Cloud-to-Device Messages Topic abonnieren (für Befehle von der Cloud)
        // Wildcard '#' für alle Sub-Topics
        String c2dTopic = String("devices/") + DEVICE_ID + "/messages/devicebound/#";
        mqttClient.subscribe(c2dTopic.c_str());
        Serial.printf("Abonniert: %s\n", c2dTopic.c_str());
        
        return true;
    } else {
        // Verbindung fehlgeschlagen - State-Code gibt Hinweis auf Fehlerursache
        Serial.printf("❌ Fehler! State: %d\n", mqttClient.state());
        connected = false;
        return false;
    }
}

// Gibt aktuellen MQTT-Verbindungsstatus zurück
bool MQTTClient::isConnected() {
    return mqttClient.connected();
}

// Trennt MQTT-Verbindung
void MQTTClient::disconnect() {
    mqttClient.disconnect();
    connected = false;
}

// Sendet Sensordaten als JSON-Telemetrie an Azure IoT Hub
bool MQTTClient::publishTelemetry(const SensorData& data, unsigned long currentEpoch) {
    if (!isConnected()) {
        return false;
    }
    
    // JSON-Dokument mit Sensordaten erstellen
    StaticJsonDocument<256> doc;  // Statischer Speicher für JSON (256 Bytes)
    
    // Alle Sensorwerte in JSON-Struktur einfügen
    doc["timestamp"] = currentEpoch;          // Unix-Timestamp
    doc["temperature"] = data.temperature;    // Temperatur in °C
    doc["humidity"] = data.humidity;          // Luftfeuchtigkeit in %
    doc["pressure"] = data.pressure;          // Luftdruck in hPa
    doc["accelX"] = data.accelX;              // Beschleunigung X-Achse
    doc["accelY"] = data.accelY;              // Beschleunigung Y-Achse
    doc["accelZ"] = data.accelZ;              // Beschleunigung Z-Achse
    doc["gyroX"] = data.gyroX;                // Gyroskop X-Achse
    doc["gyroY"] = data.gyroY;                // Gyroskop Y-Achse
    doc["gyroZ"] = data.gyroZ;                // Gyroskop Z-Achse
    
    // JSON in String serialisieren
    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);
    
    // JSON über MQTT senden
    return publishJSON(jsonBuffer);
}

// Sendet JSON-String als MQTT-Message an Azure IoT Hub
bool MQTTClient::publishJSON(const char* json) {
    if (!isConnected()) {
        return false;
    }
    
    // Azure IoT Hub Device-to-Cloud Message Topic
    // Format: devices/{device_id}/messages/events/
    String topic = String("devices/") + DEVICE_ID + "/messages/events/";
    
    // MQTT Publish durchführen
    bool result = mqttClient.publish(topic.c_str(), json);
    
    // Statusmeldung ausgeben
    if (result) {
        Serial.println("📤 Telemetrie gesendet!");
    } else {
        Serial.println("❌ Fehler beim Senden!");
    }
    
    return result;
}

// MQTT Loop-Funktion - muss regelmäßig aufgerufen werden
// Verarbeitet eingehende Messages und hält Verbindung aufrecht
void MQTTClient::loop() {
    mqttClient.loop();
}

// Überwacht Verbindung und stellt sie bei Bedarf wieder her
void MQTTClient::handleReconnect(unsigned long currentEpoch) {
    // Nur reconnecten wenn Verbindung getrennt
    if (isConnected()) {
        return;
    }
    
    unsigned long now = millis();
    
    // Reconnect nur alle RECONNECT_INTERVAL Millisekunden versuchen
    if (now - lastReconnectAttempt >= RECONNECT_INTERVAL) {
        lastReconnectAttempt = now;
        
        Serial.println("⚠️  MQTT Verbindung verloren - Reconnect...");
        
        // Neuen Verbindungsversuch mit aktuellem Epoch (für neues SAS-Token)
        if (connect(currentEpoch)) {
            Serial.println("✅ MQTT Reconnect erfolgreich!");
        } else {
            Serial.println("❌ MQTT Reconnect fehlgeschlagen");
        }
    }
}

// Statische Callback-Funktion für eingehende MQTT-Messages
// Wird von PubSubClient-Bibliothek aufgerufen
void MQTTClient::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        // Weiterleitung an Instanzmethode
        instance->handleIncomingMessage(topic, payload, length);
    }
}

// Verarbeitet eingehende Cloud-to-Device Messages
void MQTTClient::handleIncomingMessage(char* topic, byte* payload, unsigned int length) {
    // Formatierte Header-Ausgabe
    Serial.println("\n╔═══════════════════════════════════════╗");
    Serial.println("║ 📥 CLOUD-TO-DEVICE MESSAGE           ║");
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.printf("Topic: %s\n", topic);
    Serial.printf("Length: %d bytes\n", length);
    Serial.print("Payload: ");
    
    // Payload als String ausgeben
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println("\n");
    
    // JSON-Payload parsen
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    
    // Fehlerbehandlung beim Parsen
    if (error) {
        Serial.printf("❌ JSON Parse Fehler: %s\n", error.c_str());
        return;
    }
    
    Serial.println("✅ JSON erfolgreich geparst");
    
    // LED-Steuerungsbefehl verarbeiten
    if (doc.containsKey("led")) {
        String ledState = doc["led"].as<String>();
        Serial.printf("LED Command: %s\n", ledState.c_str());
        
        // LED einschalten
        if (ledState == "on") {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("💡💡💡 LED EINGESCHALTET 💡💡💡");
            Serial.println("(GPIO2 = HIGH)");
        } 
        // LED ausschalten
        else if (ledState == "off") {
            digitalWrite(LED_PIN, LOW);
            Serial.println("⚫⚫⚫ LED AUSGESCHALTET ⚫⚫⚫");
            Serial.println("(GPIO2 = LOW)");
        }
    } else {
        Serial.println("⚠️  Kein 'led' Key in JSON gefunden");
    }
    
    Serial.println("═══════════════════════════════════════\n");
}