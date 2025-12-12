#include "sensors.h"

// Konstruktor
Sensors::Sensors() : bme280Initialized(false), mpu9250Initialized(false) {
}

// Hauptinitialisierung
bool Sensors::begin() {
    Serial.println("\n=== Sensor Initialisierung ===");
    
    // I2C Bus starten
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000);
    delay(100);
    
    // I2C Bus scannen
    scanI2C();
    
    // ========== BME280 initialisieren ==========
    Serial.print("BME280 initialisieren... ");
    
    if (bme.begin(0x76, &Wire)) {
        Serial.println("OK (Adresse 0x76)");
        bme280Initialized = true;
    } else if (bme.begin(0x77, &Wire)) {
        Serial.println("OK (Adresse 0x77)");
        bme280Initialized = true;
    } else {
        Serial.println("FEHLER!");
        bme280Initialized = false;
    }
    
    if (bme280Initialized) {
        bme.setSampling(
            Adafruit_BME280::MODE_NORMAL,
            Adafruit_BME280::SAMPLING_X2,
            Adafruit_BME280::SAMPLING_X16,
            Adafruit_BME280::SAMPLING_X1,
            Adafruit_BME280::FILTER_X16,
            Adafruit_BME280::STANDBY_MS_500
        );
        Serial.println("  -> BME280 konfiguriert");
    }
    
    // ========== MPU9250 initialisieren (VEREINFACHT) ==========
    Serial.print("MPU9250 initialisieren... ");
    
    // Wire-Objekt setzen
    mpu.setWire(&Wire);
    
    // WICHTIG: Reihenfolge wie im funktionierenden Test!
    mpu.beginAccel();
    mpu.beginGyro();
    
    delay(100);  // Kurze Pause
    
    // Test-Lesung (wie im funktionierenden Standalone-Test)
    mpu.accelUpdate();
    float testValue = mpu.accelX();
    
    // Prüfen ob Sensor antwortet
    if (!isnan(testValue)) {
        Serial.println("OK");
        mpu9250Initialized = true;
        Serial.println("  -> MPU9250 konfiguriert");
        
        // Optional: Magnetometer
        mpu.beginMag();
        
    } else {
        Serial.println("FEHLER!");
        Serial.println("  -> MPU9250 antwortet nicht (NaN)");
        mpu9250Initialized = false;
    }
    
    Serial.println("==============================\n");
    
    bool success = (bme280Initialized || mpu9250Initialized);
    
    if (success) {
        Serial.println("✅ Sensoren bereit:");
        if (bme280Initialized) Serial.println("   - BME280: ✅");
        if (mpu9250Initialized) Serial.println("   - MPU9250: ✅");
    } else {
        Serial.println("❌ FEHLER: Keine Sensoren verfügbar!");
    }
    
    return success;
}

// I2C Bus Scanner
void Sensors::scanI2C() {
    Serial.println("\n--- I2C Bus Scan ---");
    byte error, address;
    int devices = 0;
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.print("  Gerät gefunden bei 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            
            if (address == 0x76 || address == 0x77) {
                Serial.print(" (BME280)");
            } else if (address == 0x68 || address == 0x69) {
                Serial.print(" (MPU9250)");
            }
            Serial.println();
            devices++;
        }
    }
    
    if (devices == 0) {
        Serial.println("  ⚠️  KEINE I2C Geräte gefunden!");
    } else {
        Serial.printf("  ✅ Insgesamt %d Gerät(e) gefunden\n", devices);
    }
    Serial.println("--------------------\n");
}

// BME280 auslesen
bool Sensors::readBME280(SensorData &data) {
    if (!bme280Initialized) {
        data.bme280Valid = false;
        return false;
    }
    
    data.temperature = bme.readTemperature();
    data.humidity = bme.readHumidity();
    data.pressure = bme.readPressure() / 100.0F;
    
    if (isnan(data.temperature) || isnan(data.humidity) || isnan(data.pressure)) {
        data.bme280Valid = false;
        return false;
    }
    
    data.bme280Valid = true;
    return true;
}

// MPU9250 auslesen (EXAKT wie im funktionierenden Test!)
bool Sensors::readMPU9250(SensorData &data) {
    if (!mpu9250Initialized) {
        data.mpu9250Valid = false;
        return false;
    }
    
    // Accelerometer auslesen
    mpu.accelUpdate();
    data.accelX = mpu.accelX();
    data.accelY = mpu.accelY();
    data.accelZ = mpu.accelZ();
    
    // Gyroskop auslesen
    mpu.gyroUpdate();
    data.gyroX = mpu.gyroX();
    data.gyroY = mpu.gyroY();
    data.gyroZ = mpu.gyroZ();
    
    // Validierung
    if (isnan(data.accelX) || isnan(data.gyroX)) {
        data.mpu9250Valid = false;
        return false;
    }
    
    data.mpu9250Valid = true;
    return true;
}

// Alle Sensoren auslesen
bool Sensors::readAll(SensorData &data) {
    data.timestamp = millis();
    
    bool bmeOk = readBME280(data);
    bool mpuOk = readMPU9250(data);
    
    return (bmeOk || mpuOk);
}

// Formatierte Ausgabe
void Sensors::printSensorData(const SensorData &data) {
    Serial.println("╔════════════════════════════════════════════════════════╗");
    Serial.printf ("║ Timestamp: %10lu ms                              ║\n", data.timestamp);
    Serial.println("╠════════════════════════════════════════════════════════╣");
    
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
    Serial.println();
}