#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "sensors.h"
#include "wifi_setup.h"
#include "mqtt.h"
#include "sas.h"

// ===== Globale Objekte =====
// Diese Objekte werden im gesamten Programm verwendet
Sensors sensors;           // Verwaltet BME280 und MPU9250 Sensoren
WifiManager wifiManager;   // Verwaltet WLAN-Verbindung und NTP-Zeit
MQTTClient mqttClient;     // Verwaltet MQTT-Kommunikation mit Azure IoT Hub
SensorData data;           // Struktur zum Speichern der Sensordaten

// ===== Timing-Variablen =====
// Speichern Zeitpunkte für periodische Aufgaben
unsigned long lastSensorRead = 0;   // Letzter Zeitpunkt der Sensordatenerfassung
unsigned long lastTimeUpdate = 0;   // Letzter Zeitpunkt der NTP-Zeitaktualisierung

// ===== Setup-Funktion =====
// Wird einmalig beim Start des ESP32 ausgeführt
void setup() {
    // Serielle Kommunikation initialisieren (115200 Baud)
    Serial.begin(115200);
    delay(2000);  // Warten damit Serial Monitor bereit ist
    
    // ===== Willkommens-Banner =====
    Serial.println("\n\n");
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║                                                       ║");
    Serial.println("║       ESP32 IoT Wetterstation - Tag 3                ║");
    Serial.println("║       Azure IoT Hub + MQTT                           ║");
    Serial.println("║                                                       ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    Serial.println();
    
    // ===== LED-Pin konfigurieren =====
    // GPIO2 (eingebaute LED auf vielen ESP32 Boards)
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // LED initial ausschalten
    
    // ===== System-Informationen ausgeben =====
    Serial.println("System Informationen:");
    Serial.printf("  CPU Frequenz: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("  Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());  // Verfügbarer RAM
    Serial.printf("  Chip ID: %llX\n", ESP.getEfuseMac());        // Eindeutige Chip-ID
    Serial.println();
    
    // ===== Sensoren initialisieren =====
    if (!sensors.begin()) {
        // Fehler bei Sensor-Initialisierung (z.B. Sensor nicht angeschlossen)
        Serial.println("\n❌ FEHLER: Sensor-Initialisierung fehlgeschlagen!");
        Serial.println("   Programm läuft trotzdem weiter (nur WLAN-Test)");
        // Programm wird nicht beendet, damit WLAN-Funktionalität getestet werden kann
    }
    
    // ===== WLAN initialisieren =====
    if (!wifiManager.begin()) {
        // WLAN-Verbindung fehlgeschlagen (z.B. falsches Passwort, Router nicht erreichbar)
        Serial.println("\n❌ FEHLER: WLAN-Verbindung fehlgeschlagen!");
        Serial.println("   Programm läuft trotzdem weiter (Offline-Modus)");
    } else {
        // WLAN erfolgreich verbunden
        
        // ===== LED-Blink-Bestätigung =====
        // LED blinkt 3x zur visuellen Bestätigung der WLAN-Verbindung
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
        }
        
        // ===== MQTT Client initialisieren =====
        delay(2000);  // Warten auf stabile NTP-Zeitsynchronisation
        
        // MQTT-Verbindung zu Azure IoT Hub aufbauen
        // Benötigt gültige Epoch-Zeit für SAS-Token-Generierung
        if (mqttClient.begin(wifiManager.getEpochTime())) {
            Serial.println("✅ MQTT Client bereit!");
        } else {
            // MQTT-Verbindung initial fehlgeschlagen
            // Wird in der Loop automatisch erneut versucht
            Serial.println("⚠️  MQTT Verbindung fehlgeschlagen (wird später versucht)");
        }
    }

    // ===== LED-Funktionstest =====
    // Testet ob die LED korrekt funktioniert
    Serial.println("\n=== LED Test ===");
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED sollte jetzt AN sein (3 Sekunden)...");
    delay(3000);
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED sollte jetzt AUS sein");
    Serial.println("================\n");
        
    Serial.println("\n✅ Setup abgeschlossen!");
    Serial.println("   Starte Hauptschleife...\n");
    
    delay(2000);  // Kurze Pause vor Start der Loop
}

// ===== Loop-Funktion =====
// Wird kontinuierlich wiederholt nach Setup
void loop() {
    // Aktuelle Zeit in Millisekunden seit Programmstart
    unsigned long currentMillis = millis();
    
    // ===== WLAN-Überwachung =====
    // Prüft WLAN-Verbindung und stellt sie bei Bedarf wieder her
    wifiManager.handleReconnect();
    
    // ===== MQTT-Verarbeitung =====
    // MUSS regelmäßig aufgerufen werden für:
    // - Verarbeitung eingehender Messages (Cloud-to-Device)
    // - Aufrechterhaltung der Verbindung (Keep-Alive)
    mqttClient.loop();
    
    // ===== MQTT-Reconnect =====
    // Prüft MQTT-Verbindung und stellt sie bei Bedarf wieder her
    // Benötigt aktuelle Zeit für neues SAS-Token
    mqttClient.handleReconnect(wifiManager.getEpochTime());
    
    // ===== NTP-Zeit aktualisieren =====
    // Alle 10 Sekunden (10000 ms) die Zeit vom NTP-Server aktualisieren
    if (currentMillis - lastTimeUpdate >= 10000) {
        lastTimeUpdate = currentMillis;
        wifiManager.updateTime();
    }
    
    // ===== Sensordaten auslesen und senden =====
    // Wird alle SENSOR_READ_INTERVAL_MS ausgeführt (z.B. alle 5 Sekunden)
    if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
        lastSensorRead = currentMillis;
        
        // Status-LED einschalten während Datenerfassung
        digitalWrite(LED_PIN, HIGH);
        
        // Sensoren auslesen
        if (sensors.readAll(data)) {
            // Datenerfassung erfolgreich
            
            // ===== Formatierte Konsolen-Ausgabe =====
            // Kopfzeile mit System-Status
            Serial.println("╔════════════════════════════════════════════════════════╗");
            Serial.printf ("║ Zeit: %-15s | Uptime: %10lu ms      ║\n", 
                          wifiManager.getFormattedTime().c_str(),  // Aktuelle Uhrzeit
                          currentMillis);                          // Laufzeit seit Start
            Serial.printf ("║ Epoch: %-12lu | Heap: %10d bytes    ║\n",
                          wifiManager.getEpochTime(),              // Unix-Timestamp
                          ESP.getFreeHeap());                      // Freier RAM-Speicher
            Serial.printf ("║ WLAN: %-10s | RSSI: %4d dBm                ║\n",
                          wifiManager.isConnected() ? "Verbunden" : "Getrennt",
                          WiFi.RSSI());                            // WLAN-Signalstärke
            Serial.printf ("║ MQTT: %-10s | Azure IoT Hub                ║\n",
                          mqttClient.isConnected() ? "Verbunden" : "Getrennt");
            Serial.println("╠════════════════════════════════════════════════════════╣");
            
            // ===== BME280 Umwelt-Sensor Daten =====
            if (data.bme280Valid) {
                // Sensor hat gültige Daten geliefert
                Serial.println("║ BME280 - Umwelt-Sensor                                 ║");
                Serial.println("╟────────────────────────────────────────────────────────╢");
                Serial.printf ("║   🌡️  Temperatur:   %6.2f °C                        ║\n", data.temperature);
                Serial.printf ("║   💧 Luftfeuchte:  %6.2f %%                         ║\n", data.humidity);
                Serial.printf ("║   📊 Luftdruck:    %7.2f hPa                        ║\n", data.pressure);
            } else {
                // Sensor nicht verfügbar oder Lesefehler
                Serial.println("║ BME280 - ❌ NICHT VERFÜGBAR                            ║");
            }
            
            Serial.println("╠════════════════════════════════════════════════════════╣");
            
            // ===== MPU9250 Bewegungs-Sensor Daten =====
            if (data.mpu9250Valid) {
                // Sensor hat gültige Daten geliefert
                Serial.println("║ MPU9250 - Bewegungs-Sensor                             ║");
                Serial.println("╟────────────────────────────────────────────────────────╢");
                
                // Beschleunigungsdaten (in g - Erdbeschleunigung)
                Serial.println("║ Beschleunigung (g):                                    ║");
                Serial.printf ("║   X: %+7.3f  |  Y: %+7.3f  |  Z: %+7.3f     ║\n", 
                               data.accelX, data.accelY, data.accelZ);
                Serial.println("╟────────────────────────────────────────────────────────╢");
                
                // Gyroskop-Daten (in Grad pro Sekunde)
                Serial.println("║ Gyroskop (°/s):                                        ║");
                Serial.printf ("║   X: %+8.2f | Y: %+8.2f | Z: %+8.2f    ║\n", 
                               data.gyroX, data.gyroY, data.gyroZ);
            } else {
                // Sensor nicht verfügbar oder Lesefehler
                Serial.println("║ MPU9250 - ❌ NICHT VERFÜGBAR                           ║");
            }
            
            Serial.println("╚════════════════════════════════════════════════════════╝");
            
            // ===== JSON-Vorschau =====
            // Zeigt wie die Daten als JSON an Azure IoT Hub gesendet werden
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
            
            // ===== Daten an Azure IoT Hub senden =====
            // Nur wenn MQTT-Verbindung aktiv ist
            if (mqttClient.isConnected()) {
                // Sendet komplette Sensordaten als JSON-Telemetrie
                mqttClient.publishTelemetry(data, wifiManager.getEpochTime());
            }
            
        } else {
            // Fehler beim Auslesen der Sensoren
            Serial.println("⚠️  Fehler beim Auslesen der Sensoren");
        }
        
        // Status-LED wieder ausschalten
        digitalWrite(LED_PIN, LOW);
    }
    
    // ===== Kleine Pause =====
    // Verhindert zu hohe CPU-Last und ermöglicht WiFi-Stack-Verarbeitung
    delay(10);
}