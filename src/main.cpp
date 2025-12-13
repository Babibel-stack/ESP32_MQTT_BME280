#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors.h"
#include "wifi_setup.h"
#include "mqtt.h"
#include "sas.h"

// Globale Objekte
Sensors sensors;
WifiManager wifiManager;
MQTTClient mqttClient;     // ← DIESE ZEILE HINZUFÜGEN!
SensorData data;

// Timing
unsigned long lastSensorRead = 0;
unsigned long lastTimeUpdate = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n");
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║                                                       ║");
    Serial.println("║       ESP32 IoT Wetterstation - Tag 3                ║");
    Serial.println("║       Azure IoT Hub + MQTT                           ║");
    Serial.println("║                                                       ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    Serial.println();
    
    // LED Pin konfigurieren
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // System Info
    Serial.println("System Informationen:");
    Serial.printf("  CPU Frequenz: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("  Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("  Chip ID: %llX\n", ESP.getEfuseMac());
    Serial.println();
    
    // Sensoren initialisieren
    if (!sensors.begin()) {
        Serial.println("\n❌ FEHLER: Sensor-Initialisierung fehlgeschlagen!");
        Serial.println("   Programm läuft trotzdem weiter (nur WLAN-Test)");
    }
    
    // WLAN initialisieren
    if (!wifiManager.begin()) {
        Serial.println("\n❌ FEHLER: WLAN-Verbindung fehlgeschlagen!");
        Serial.println("   Programm läuft trotzdem weiter (Offline-Modus)");
    } else {
        // LED blinken zur Bestätigung
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
        }
        
        // MQTT Client initialisieren
        delay(2000);  // Warte auf stabile NTP Zeit
        if (mqttClient.begin(wifiManager.getEpochTime())) {
            Serial.println("✅ MQTT Client bereit!");
        } else {
            Serial.println("⚠️  MQTT Verbindung fehlgeschlagen (wird später versucht)");
        }
    }




        // LED Test
    Serial.println("\n=== LED Test ===");
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED sollte jetzt AN sein (3 Sekunden)...");
    delay(3000);
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED sollte jetzt AUS sein");
    Serial.println("================\n");
        
    Serial.println("\n✅ Setup abgeschlossen!");
    Serial.println("   Starte Hauptschleife...\n");
    
    delay(2000);
}

void loop() {
    unsigned long currentMillis = millis();
    
    // WLAN Reconnect Logik
    wifiManager.handleReconnect();
    
    // MQTT Loop (wichtig!)
    mqttClient.loop();
    
    // MQTT Reconnect falls nötig
    mqttClient.handleReconnect(wifiManager.getEpochTime());
    
    // Zeit aktualisieren (alle 10 Sekunden)
    if (currentMillis - lastTimeUpdate >= 10000) {
        lastTimeUpdate = currentMillis;
        wifiManager.updateTime();
    }
    
    // Sensor Daten auslesen
    if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
        lastSensorRead = currentMillis;
        
        // Status LED kurz an
        digitalWrite(LED_PIN, HIGH);
        
        // Sensoren auslesen
        if (sensors.readAll(data)) {
            
            // Formatierte Ausgabe
            Serial.println("╔════════════════════════════════════════════════════════╗");
            Serial.printf ("║ Zeit: %-15s | Uptime: %10lu ms      ║\n", 
                          wifiManager.getFormattedTime().c_str(), 
                          currentMillis);
            Serial.printf ("║ Epoch: %-12lu | Heap: %10d bytes    ║\n",
                          wifiManager.getEpochTime(),
                          ESP.getFreeHeap());
            Serial.printf ("║ WLAN: %-10s | RSSI: %4d dBm                ║\n",
                          wifiManager.isConnected() ? "Verbunden" : "Getrennt",
                          WiFi.RSSI());
            Serial.printf ("║ MQTT: %-10s | Azure IoT Hub                ║\n",
                          mqttClient.isConnected() ? "Verbunden" : "Getrennt");
            Serial.println("╠════════════════════════════════════════════════════════╣");
            
            // BME280 Daten
            if (data.bme280Valid) {
                Serial.println("║ BME280 - Umwelt-Sensor                                 ║");
                Serial.println("╟────────────────────────────────────────────────────────╢");
                Serial.printf ("║   🌡️  Temperatur:   %6.2f °C                        ║\n", data.temperature);
                Serial.printf ("║   💧 Luftfeuchte:  %6.2f %%                         ║\n", data.humidity);
                Serial.printf ("║   📊 Luftdruck:    %7.2f hPa                        ║\n", data.pressure);
            } else {
                Serial.println("║ BME280 - ❌ NICHT VERFÜGBAR                            ║");
            }
            
            Serial.println("╠════════════════════════════════════════════════════════╣");
            
            // MPU9250 Daten
            if (data.mpu9250Valid) {
                Serial.println("║ MPU9250 - Bewegungs-Sensor                             ║");
                Serial.println("╟────────────────────────────────────────────────────────╢");
                Serial.println("║ Beschleunigung (g):                                    ║");
                Serial.printf ("║   X: %+7.3f  |  Y: %+7.3f  |  Z: %+7.3f     ║\n", 
                               data.accelX, data.accelY, data.accelZ);
                Serial.println("╟────────────────────────────────────────────────────────╢");
                Serial.println("║ Gyroskop (°/s):                                        ║");
                Serial.printf ("║   X: %+8.2f | Y: %+8.2f | Z: %+8.2f    ║\n", 
                               data.gyroX, data.gyroY, data.gyroZ);
            } else {
                Serial.println("║ MPU9250 - ❌ NICHT VERFÜGBAR                           ║");
            }
            
            Serial.println("╚════════════════════════════════════════════════════════╝");
            
            // JSON Format (Vorbereitung für MQTT)
            Serial.println("\nJSON Format (für Azure IoT Hub):");
            Serial.println("{");
            Serial.printf("  \"timestamp\": %lu,\n", wifiManager.getEpochTime());
            Serial.printf("  \"temperature\": %.2f,\n", data.temperature);
            Serial.printf("  \"humidity\": %.2f,\n", data.humidity);
            Serial.printf("  \"pressure\": %.2f,\n", data.pressure);
            Serial.printf("  \"accelX\": %.3f,\n", data.accelX);
            Serial.printf("  \"accelY\": %.3f,\n", data.accelY);
            Serial.printf("  \"accelZ\": %.3f,\n", data.accelZ);
            Serial.printf("  \"gyroX\": %.2f,\n", data.gyroX);
            Serial.printf("  \"gyroY\": %.2f,\n", data.gyroY);
            Serial.printf("  \"gyroZ\": %.2f\n", data.gyroZ);
            Serial.println("}\n");
            
            // An Azure IoT Hub senden!
            if (mqttClient.isConnected()) {
                mqttClient.publishTelemetry(data, wifiManager.getEpochTime());
            }
            
        } else {
            Serial.println("⚠️  Fehler beim Auslesen der Sensoren");
        }
        
        // LED wieder aus
        digitalWrite(LED_PIN, LOW);
    }
    
    // Kleine Pause
    delay(10);
}