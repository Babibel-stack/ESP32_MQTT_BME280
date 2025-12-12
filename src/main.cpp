
#include <Arduino.h>
#include "sensors.h"

// Optional: Logger einbinden wenn vorhanden
// #include "logger.h"
// Globale Objekte
Sensors sensors;
SensorData data;

// Konfiguration
const unsigned long SENSOR_INTERVAL = 5000;  // 5 Sekunden
unsigned long lastSensorRead = 0;



void setup() {
    Serial.begin(115200);
    delay(2000);  // Warte auf Serial Monitor
    

    
    // Optional: Logger statt Serial direkt
    // Logger::init(LOG_DEBUG);
    // LOG_I("MAIN", "ESP32 IoT Wetterstation gestartet");

    /*
    
    Logger::init(LOG_VERBOSE);
    
    LOG_I("SETUP", "System startet...");
    LOG_D("SETUP", "ESP32 Chip ID: %llX", ESP.getEfuseMac());
    LOG_W("SETUP", "Dies ist eine Warnung!");
    LOG_E("SETUP", "Dies ist ein Fehler!");

    */

    
    
    
    
    
    Serial.println("\n\n");
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║                                                       ║");
    Serial.println("║       ESP32 IoT Wetterstation - Tag 1                ║");
    Serial.println("║       Sensor Auslesen & Testen                       ║");
    Serial.println("║                                                       ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    Serial.println();
    
    // System Info
    Serial.println("System Informationen:");
    Serial.printf("  CPU Frequenz: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("  Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("  Chip ID: %llX\n", ESP.getEfuseMac());
    Serial.println();
    
    // Sensoren initialisieren
    if (!sensors.begin()) {
        Serial.println("\n❌ FEHLER: Keine Sensoren gefunden!");
        Serial.println("   Programm gestoppt. Bitte Hardware prüfen.");
        while(1) {
            delay(1000);  // Endlosschleife
        }
    }
    
    Serial.println("✅ Initialisierung erfolgreich!");
    Serial.println("   Starte kontinuierliche Messung...\n");
    
    delay(2000);
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Sensor Daten alle X Sekunden auslesen
    if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = currentMillis;
        
        // Alle Sensoren auslesen
        if (sensors.readAll(data)) {
            sensors.printSensorData(data);
            
            // Optional: Kompakt-Format für CSV Export
            Serial.println("CSV: timestamp,temp,hum,press,accX,accY,accZ,gyroX,gyroY,gyroZ");
            Serial.printf("     %lu,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n",
                data.timestamp,
                data.temperature, data.humidity, data.pressure,
                data.accelX, data.accelY, data.accelZ,
                data.gyroX, data.gyroY, data.gyroZ
            );
            Serial.println();
            
        } else {
            Serial.println("⚠️  Fehler beim Auslesen der Sensoren");
        }
        
        // Speicher-Check
        Serial.printf("💾 Free Heap: %d bytes\n\n", ESP.getFreeHeap());
    }
    
    // Kleine Pause für Stabilität
    delay(10);
}

// Alternative: Wenn du den Logger benutzt, ersetze die Serial.prints durch:
/*
void setup() {
    Logger::init(LOG_DEBUG);
    
    LOG_I("MAIN", "ESP32 IoT Wetterstation - Tag 1");
    LOG_I("MAIN", "CPU: %d MHz", ESP.getCpuFreqMHz());
    LOG_I("MAIN", "Chip ID: %llX", ESP.getEfuseMac());
    
    if (!sensors.begin()) {
        LOG_E("MAIN", "Keine Sensoren gefunden!");
        while(1) delay(1000);
    }
    
    LOG_I("MAIN", "Initialisierung erfolgreich");
}

*/
