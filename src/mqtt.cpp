#include "mqtt.h"
#include "config.h"
#include <ArduinoJson.h>

// Statische Instanz für Callback-Funktion
MQTTClient* MQTTClient::instance = nullptr;



 

// Baltimore CyberTrust Root CA Zertifikat
const char* AZURE_ROOT_CA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n" \
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
"1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n" \
"Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n" \
"8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n" \
"pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n" \
"MrY=\n" \
"-----END CERTIFICATE-----\n";


/**

// Schicht 3: X.509 Gerätezertifikat (von OstfaliaRootCA signiert)
const char* DEVICE_CERT = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDUDCCAjigAwIBAgIUYRW4fiX+xnXjCNwN37hsiBTDdHYwDQYJKoZIhvcNAQEL\n" \
"BQAwNzESMBAGA1UEAwwJSW9UUm9vdENBMRQwEgYDVQQKDAtPc3RmYWxpYUlvVDEL\n" \
"MAkGA1UEBhMCREUwHhcNMjYwNDIyMDY1NjQxWhcNMjcwNDIyMDY1NjQxWjBJMSQw\n" \
"IgYDVQQDDBtpb3RXZWF0aGVyc3RhdGlvbmVzcDMyLXg1MDkxFDASBgNVBAoMC09z\n" \
"dGZhbGlhSW9UMQswCQYDVQQGEwJERTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC\n" \
"AQoCggEBAKcBVzo8P0+odbOIwlEkUYikGWf23gmASJcPVbdZ68AQRRiwDwXKVggs\n" \
"gwfuJuLzCzWmn10R1yQ4FhKSUuQzbqHacgxYrlH/OZVmxWez6iAGO+kKesM7pn86\n" \
"y9k5uhO9vySezlylXr8AkPYJ4EWdG9msYW5xeVCUAzLvjyLYnMkUktk19j7sj0fh\n" \
"I4oksdqr8JDKw2HX2ANfsTbmX9aP1q6KCrXNVaTMgLeN5dAXFrGxHrmupjAvxIJN\n" \
"Xx+OUxAQ0qoVRtwhwMgSS7Tl5WvllYFqzlNQxv3wNrIxJ7xRS7QuKK0nBZcpjbCX\n" \
"jdBNNrdShyOnZqqww1L2jCvd9EpwIWUCAwEAAaNCMEAwHQYDVR0OBBYEFJcN1t3G\n" \
"LG7o3RJGxP2r2Qk1wiiFMB8GA1UdIwQYMBaAFGRbcex4kIMNta2uv1EK8CAY1VJg\n" \
"MA0GCSqGSIb3DQEBCwUAA4IBAQBKgPW/YlHkrNDZpFxGgjmutTKi+UVIKkE4ReYd\n" \
"31TrbZd9w/IqJ+ctBkfOykVq/SHlJKCYhXCtHy/R4u/iBtdgBSEfletxBEJBv8Xh\n" \
"4SPgSDQGGfETGF1TD53UvM6D7/ZGe8zVMFuF4nhaoQUGHRK3qIbrxJ4Sr4mMYi7d\n" \
"NpC6mnkHghWmf0ZaE9AAuS7K0/ATrqgQtvyFmHCV/zPT5TM8zPBx237BUNuvK86m\n" \
"J2ZT9hAv8CGwk5+Dim//xR7DAwuhvromEDeNLZwbA6ZICKNR+pW4rNGs6Om2WkBH\n" \
"oor/wScRHzYLXCM2j2f7Qn4HZv1ZXKisNvls9KZiGr0/z8cl\n" \
"-----END CERTIFICATE-----\n";

// Schicht 3: X.509 Privater Schlüssel des ESP32
const char* DEVICE_KEY_X509 = \
"-----BEGIN PRIVATE KEY-----\n" \
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCnAVc6PD9PqHWz\n" \
"iMJRJFGIpBln9t4JgEiXD1W3WevAEEUYsA8FylYILIMH7ibi8ws1pp9dEdckOBYS\n" \
"klLkM26h2nIMWK5R/zmVZsVns+ogBjvpCnrDO6Z/OsvZOboTvb8kns5cpV6/AJD2\n" \
"CeBFnRvZrGFucXlQlAMy748i2JzJFJLZNfY+7I9H4SOKJLHaq/CQysNh19gDX7E2\n" \
"5l/Wj9auigq1zVWkzIC3jeXQFxaxsR65rqYwL8SCTV8fjlMQENKqFUbcIcDIEku0\n" \
"5eVr5ZWBas5TUMb98DayMSe8UUu0LiitJwWXKY2wl43QTTa3Uocjp2aqsMNS9owr\n" \
"3fRKcCFlAgMBAAECggEAATpCofCRsAJ6VGduPEZ6W72GLDD2zykXyS3egMQdUPdY\n" \
"0BJmTKkA8CrikfbCfyL2+5sUji1QDqlXw3LJ4DGdeSl+DYPmteWE2rBXvMdrNWrv\n" \
"/NirTc6FaBQIjRLnJdId7Ka+C5snQaj6sjjxRN1DQTQE0Qug1b1Fb91goKhUGKim\n" \
"fpQtG71ec6KvfkB2WiPURvDs5emaLyShof+6qW9WR1tsSRTmKQphtyVckaJZCHrs\n" \
"TPWOOklnA4J+4YmMML5FGb+5RErhq8wEXHbnBnOEWDCCSknYOyaPcqjr4HflWb89\n" \
"EEDvX7CLIUGuri11hFBvZEUsQbudY38mt5zCBwIXCwKBgQDpC27L8PFC+owe9zIl\n" \
"Mb+JdS+cOLhc+xYJ9kZtxFHrT9cs7Pj6dv7v8XD5aWg8zV31z4rz7FmrSVQOx/E8\n" \
"E8Sg7xwymYUjfV7yWmn8idWqpa5ftU/CjVd4L9KobNB2zTx9Vyk02wMQ4fVD5APa\n" \
"p2iqi93T67+EPiMVaZ7a5BblwwKBgQC3dKA173UYJZnMdN2m0WAUPhvBH1QjYQ7U\n" \
"HmDsrRT/hbZZjWX4pusp/fzxYkJ7uZ/0X422lnbMCOtKFW23UX/npL81VdSsYR4n\n" \
"UuEgBJTt4BV0dyOWBn+lt/BCM2dFirPKyUdH2qCJe2TrScKUYgYu2NOktzy0X0bw\n" \
"b7YMs8FhtwKBgQDJven2huh00SzICbrRVBW5y0ah+MAxTfOwQBCMKa25BW8DJ0oK\n" \
"mUTtNphUcZmZOTej+j0SGIMvstUfzprZJUvM/fHtI9WL+ZZeO0MRKclcZv1jQTSA\n" \
"+ZbFJrE4uKpmjhlVcETtysMGtHV8hkGH4fDL4zTvpmAu/ROmD79DQr1HvQKBgFa0\n" \
"WhvjppXY/41wFmoHPr+Scw+dPOjTsGx0Rx7U1r3Fdl9wwqb4TOC119xFsmJDYuPe\n" \
"XV3UUyUvefSzl/0yGZ9jb7NHc5Jc/CFdvGSjhbe/VqSKwljfjWjHPFgtbzugyESp\n" \
"SHUfUQxpM1M4syvqAD4X4D9TEu+0sBRm18W9F0KVAoGAHkzgI+EWeh7QI8tRV0cc\n" \
"6CN8GlB+2Ksf9/+D901sYJNchAFTi/4o7AXy6gkA02jnUw2REsa1/sE0TfpHpoP2\n" \
"bOXjIdUOVblwAPOi5Cj1LihTa0m2seGb/ZHE+/hA1Iez0NG81xV5lWsxtZSmy16N\n" \
"X0koNWIQNuVDvD11qYfVyhU=\n" \
"-----END PRIVATE KEY-----\n";

*/




MQTTClient::MQTTClient() : mqttClient(wifiClient), connected(false), 
                           sasTokenExpiry(0), lastReconnectAttempt(0) {
    instance = this;
}


//Schicht 3: SAS-Token Authentifizierung (mit Azure IoT Hub)
bool MQTTClient::begin(unsigned long currentEpoch) {
    Serial.println("\n=== MQTT Client Initialisierung ===");
    
    // Alt — entfernen
    //wifiClient.setInsecure(); //Schicht 2: Unsichere Verbindung (ohne mutual TLS) — NICHT EMPFOHLEN!


    // Neu — einfügen
    wifiClient.setCACert(AZURE_ROOT_CA);  // ← Schicht 2: TLS mit Root CA
    
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


/*
// Ab hier X.509 Zertifikat Authentifizierung

bool MQTTClient::begin(unsigned long currentEpoch) {
    Serial.println("\n=== MQTT Client Initialisierung ===");

    wifiClient.setCACert(AZURE_ROOT_CA);       // Schicht 2: TLS Root CA
    wifiClient.setCertificate(DEVICE_CERT);    // Schicht 3: X.509 Gerätezertifikat
    wifiClient.setPrivateKey(DEVICE_KEY_X509); // Schicht 3: Privater Schlüssel

    mqttClient.setServer(IOT_HUB_HOSTNAME, MQTT_PORT);
    mqttClient.setCallback(messageCallback);
    mqttClient.setBufferSize(1024);

    Serial.printf("IoT Hub: %s\n", IOT_HUB_HOSTNAME);
    Serial.printf("Device ID: %s\n", DEVICE_ID);
    Serial.printf("MQTT QoS Level: %d\n", MQTT_QOS_LEVEL);

    return connect(currentEpoch);
}



bool MQTTClient::connect(unsigned long currentEpoch) {

    String mqttUsername = String(IOT_HUB_HOSTNAME) + "/" + 
                      String(DEVICE_ID) + 
                      "/?api-version=2021-04-12";
// OHNE &X509Cert=true

     // X509Cert=true signalisiert Azure dass X.509 verwendet wird

    Serial.print("Verbinde mit Azure IoT Hub (X.509)... ");

    // Schicht 3: X.509 — kein Passwort, Zertifikat übernimmt Authentifizierung
    bool result = mqttClient.connect(DEVICE_ID,
                                     mqttUsername.c_str(),  
                                     ""); // NULL = kein SAS-Token mehr

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

*/



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
        lastReconnectAttempt = now; // Timestamp VOR dem Versuch setzen
        
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