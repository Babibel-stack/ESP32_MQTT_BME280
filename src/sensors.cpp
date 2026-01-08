#include "sensors.h"

// ===== Konstruktor =====
// Initialisiert Flags für Sensor-Status mit false (Sensoren noch nicht bereit)
Sensors::Sensors() : bme280Initialized(false), mpu9250Initialized(false) {
}

// ===== Hauptinitialisierung aller Sensoren =====
// Startet I2C-Bus, scannt nach Geräten und initialisiert BME280 und MPU9250
bool Sensors::begin() {
    Serial.println("\n=== Sensor Initialisierung ===");
    
    // ===== I2C Bus initialisieren =====
    // SDA = GPIO21, SCL = GPIO22 (Standard ESP32 Pins)
    Wire.begin(I2C_SDA, I2C_SCL);
    
    // I2C Taktfrequenz auf 400 kHz setzen (Fast Mode)
    // Standard wäre 100 kHz, 400 kHz ist schneller und wird von beiden Sensoren unterstützt
    Wire.setClock(400000);
    delay(100);  // Kurze Pause damit I2C-Bus stabil ist
    
    // ===== I2C Bus nach angeschlossenen Geräten durchsuchen =====
    scanI2C();
    
    // ========== BME280 Umweltsensor initialisieren ==========
    Serial.print("BME280 initialisieren... ");
    
    // BME280 kann auf Adresse 0x76 oder 0x77 sein (je nach Modul)
    // Versuche beide Adressen
    if (bme.begin(0x76, &Wire)) {
        Serial.println("OK (Adresse 0x76)");
        bme280Initialized = true;
    } else if (bme.begin(0x77, &Wire)) {
        Serial.println("OK (Adresse 0x77)");
        bme280Initialized = true;
    } else {
        // Sensor nicht gefunden auf beiden Adressen
        Serial.println("FEHLER!");
        bme280Initialized = false;
    }
    
    // BME280 konfigurieren falls erfolgreich initialisiert
    if (bme280Initialized) {
        bme.setSampling(
            Adafruit_BME280::MODE_NORMAL,      // Kontinuierlicher Messmodus
            Adafruit_BME280::SAMPLING_X2,      // Temperatur: 2x Oversampling
            Adafruit_BME280::SAMPLING_X16,     // Luftdruck: 16x Oversampling (höchste Genauigkeit)
            Adafruit_BME280::SAMPLING_X1,      // Luftfeuchtigkeit: 1x Oversampling
            Adafruit_BME280::FILTER_X16,       // IIR-Filter (glättet Werte)
            Adafruit_BME280::STANDBY_MS_500    // 500ms Pause zwischen Messungen
        );
        Serial.println("  -> BME280 konfiguriert");
    }
    
    // ========== MPU9250 Bewegungssensor initialisieren ==========
    Serial.print("MPU9250 initialisieren... ");
    
    // Wire-Objekt explizit setzen (wichtig für Bibliothek)
    mpu.setWire(&Wire);
    
    // WICHTIG: Diese Reihenfolge ist kritisch!
    // Erst Accelerometer, dann Gyroskop initialisieren
    mpu.beginAccel();  // 3-Achsen Beschleunigungssensor
    mpu.beginGyro();   // 3-Achsen Gyroskop (Rotationssensor)
    
    delay(100);  // Kurze Pause für Sensor-Stabilisierung
    
    // ===== Test-Lesung durchführen =====
    // Prüft ob Sensor tatsächlich antwortet und gültige Daten liefert
    mpu.accelUpdate();              // Beschleunigungsdaten aktualisieren
    float testValue = mpu.accelX(); // X-Achse auslesen
    
    // Prüfen ob Sensor gültige Werte liefert (nicht NaN = Not a Number)
    if (!isnan(testValue)) {
        Serial.println("OK");
        mpu9250Initialized = true;
        Serial.println("  -> MPU9250 konfiguriert");
        
        // Optional: Magnetometer initialisieren (9-Achsen IMU)
        // Wird in diesem Projekt nicht verwendet, aber verfügbar
        mpu.beginMag();
        
    } else {
        // Sensor antwortet nicht korrekt
        Serial.println("FEHLER!");
        Serial.println("  -> MPU9250 antwortet nicht (NaN)");
        mpu9250Initialized = false;
    }
    
    Serial.println("==============================\n");
    
    // Initialisierung gilt als erfolgreich wenn mindestens ein Sensor funktioniert
    bool success = (bme280Initialized || mpu9250Initialized);
    
    // ===== Status-Zusammenfassung ausgeben =====
    if (success) {
        Serial.println("✅ Sensoren bereit:");
        if (bme280Initialized) Serial.println("   - BME280: ✅");
        if (mpu9250Initialized) Serial.println("   - MPU9250: ✅");
    } else {
        Serial.println("❌ FEHLER: Keine Sensoren verfügbar!");
    }
    
    return success;
}

// ===== I2C Bus Scanner =====
// Durchsucht alle möglichen I2C-Adressen (1-126) nach angeschlossenen Geräten
// Nützlich für Debugging und Fehlersuche bei Verkabelungsproblemen
void Sensors::scanI2C() {
    Serial.println("\n--- I2C Bus Scan ---");
    byte error, address;
    int devices = 0;  // Zähler für gefundene Geräte
    
    // Alle gültigen I2C-Adressen durchgehen (0x01 bis 0x7F)
    for(address = 1; address < 127; address++) {
        // Verbindungsversuch zur Adresse
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        // error == 0 bedeutet: Gerät hat geantwortet
        if (error == 0) {
            Serial.print("  Gerät gefunden bei 0x");
            if (address < 16) Serial.print("0");  // Führende Null für Formatierung
            Serial.print(address, HEX);
            
            // Bekannte Sensor-Adressen identifizieren
            if (address == 0x76 || address == 0x77) {
                Serial.print(" (BME280)");  // Umweltsensor
            } else if (address == 0x68 || address == 0x69) {
                Serial.print(" (MPU9250)");  // Bewegungssensor
            }
            Serial.println();
            devices++;
        }
    }
    
    // Zusammenfassung
    if (devices == 0) {
        Serial.println("  ⚠️  KEINE I2C Geräte gefunden!");
        Serial.println("     -> Verkabelung prüfen!");
    } else {
        Serial.printf("  ✅ Insgesamt %d Gerät(e) gefunden\n", devices);
    }
    Serial.println("--------------------\n");
}

// ===== BME280 Umweltsensor auslesen =====
// Liest Temperatur, Luftfeuchtigkeit und Luftdruck
bool Sensors::readBME280(SensorData &data) {
    // Prüfen ob Sensor initialisiert wurde
    if (!bme280Initialized) {
        data.bme280Valid = false;
        return false;
    }
    
    // Sensorwerte auslesen
    data.temperature = bme.readTemperature();    // Temperatur in °C
    data.humidity = bme.readHumidity();          // Relative Luftfeuchtigkeit in %
    data.pressure = bme.readPressure() / 100.0F; // Luftdruck in hPa (Pascal → Hektopascal)
    
    // Validierung: Prüfen ob alle Werte gültig sind (nicht NaN)
    if (isnan(data.temperature) || isnan(data.humidity) || isnan(data.pressure)) {
        data.bme280Valid = false;
        return false;
    }
    
    // Daten sind gültig
    data.bme280Valid = true;
    return true;
}

// ===== MPU9250 Bewegungssensor auslesen =====
// Liest Beschleunigung (3 Achsen) und Rotation (3 Achsen)
bool Sensors::readMPU9250(SensorData &data) {
    // Prüfen ob Sensor initialisiert wurde
    if (!mpu9250Initialized) {
        data.mpu9250Valid = false;
        return false;
    }
    
    // ===== Beschleunigungssensor auslesen =====
    // Misst lineare Beschleunigung in g (Erdbeschleunigung)
    mpu.accelUpdate();           // Neue Messwerte vom Sensor holen
    data.accelX = mpu.accelX();  // X-Achse (vorwärts/rückwärts)
    data.accelY = mpu.accelY();  // Y-Achse (links/rechts)
    data.accelZ = mpu.accelZ();  // Z-Achse (oben/unten) - sollte ~1g sein im Ruhezustand
    
    // ===== Gyroskop auslesen =====
    // Misst Rotationsgeschwindigkeit in °/s (Grad pro Sekunde)
    mpu.gyroUpdate();           // Neue Messwerte vom Sensor holen
    data.gyroX = mpu.gyroX();   // Rotation um X-Achse (Nicken/Pitch)
    data.gyroY = mpu.gyroY();   // Rotation um Y-Achse (Rollen/Roll)
    data.gyroZ = mpu.gyroZ();   // Rotation um Z-Achse (Gieren/Yaw)
    
    // Validierung: Mindestens ein Wert pro Sensor-Typ muss gültig sein
    if (isnan(data.accelX) || isnan(data.gyroX)) {
        data.mpu9250Valid = false;
        return false;
    }
    
    // Daten sind gültig
    data.mpu9250Valid = true;
    return true;
}

// ===== Alle Sensoren auf einmal auslesen =====
// Zentrale Funktion die beide Sensoren ausliest und Zeitstempel hinzufügt
bool Sensors::readAll(SensorData &data) {
    // Zeitstempel setzen (Millisekunden seit Programmstart)
    data.timestamp = millis();
    
    // Beide Sensoren auslesen
    bool bmeOk = readBME280(data);    // Umweltsensor
    bool mpuOk = readMPU9250(data);   // Bewegungssensor
    
    // Erfolgreich wenn mindestens ein Sensor funktioniert hat
    return (bmeOk || mpuOk);
}

// ===== Formatierte Konsolenausgabe der Sensordaten =====
// Gibt alle Sensordaten in einem schön formatierten Rahmen aus
void Sensors::printSensorData(const SensorData &data) {
    // ===== Header mit Zeitstempel =====
    Serial.println("╔════════════════════════════════════════════════════════╗");
    Serial.printf ("║ Timestamp: %10lu ms                              ║\n", data.timestamp);
    Serial.println("╠════════════════════════════════════════════════════════╣");
    
    // ===== BME280 Daten =====
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
    
    // ===== MPU9250 Daten =====
    if (data.mpu9250Valid) {
        // Sensor hat gültige Daten geliefert
        Serial.println("║ MPU9250 - Bewegungs-Sensor                             ║");
        Serial.println("╟────────────────────────────────────────────────────────╢");
        
        // Beschleunigungsdaten (in g)
        // Bei horizontaler Ausrichtung: Z sollte ~1g sein (Erdanziehung)
        Serial.println("║ Beschleunigung (g):                                    ║");
        Serial.printf ("║   X: %+7.3f  |  Y: %+7.3f  |  Z: %+7.3f     ║\n", 
                       data.accelX, data.accelY, data.accelZ);
        Serial.println("╟────────────────────────────────────────────────────────╢");
        
        // Gyroskop-Daten (in °/s)
        // Im Ruhezustand sollten alle Werte nahe 0 sein
        Serial.println("║ Gyroskop (°/s):                                        ║");
        Serial.printf ("║   X: %+8.2f | Y: %+8.2f | Z: %+8.2f    ║\n", 
                       data.gyroX, data.gyroY, data.gyroZ);
    } else {
        // Sensor nicht verfügbar oder Lesefehler
        Serial.println("║ MPU9250 - ❌ NICHT VERFÜGBAR                           ║");
    }
    
    // ===== Footer =====
    Serial.println("╚════════════════════════════════════════════════════════╝");
    Serial.println();
}