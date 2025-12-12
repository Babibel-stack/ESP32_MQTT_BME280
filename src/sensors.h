#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <MPU9250_asukiaaa.h>  // KORRIGIERT: asukiaaa statt Bolder Flight

// I2C Pins für ESP32
#define I2C_SDA 21
#define I2C_SCL 22

// Sensor Adressen
#define BME280_ADDRESS 0x76  // oder 0x77
#define MPU9250_ADDRESS 0x68

// Sensor Daten Struktur
struct SensorData {
    // BME280
    float temperature;    // °C
    float humidity;       // %
    float pressure;       // hPa
    
    // MPU9250
    float accelX;         // g
    float accelY;         // g
    float accelZ;         // g
    float gyroX;          // °/s
    float gyroY;          // °/s
    float gyroZ;          // °/s
    
    // Status
    bool bme280Valid;
    bool mpu9250Valid;
    unsigned long timestamp;  // millis()
};

class Sensors {
private:
    Adafruit_BME280 bme;
    MPU9250_asukiaaa mpu;  // asukiaaa Bibliothek
    
    bool bme280Initialized;
    bool mpu9250Initialized;
    
    void scanI2C();
    
public:
    Sensors();
    
    bool begin();
    bool readBME280(SensorData &data);
    bool readMPU9250(SensorData &data);
    bool readAll(SensorData &data);
    
    void printSensorData(const SensorData &data);
    bool isBME280Ready() { return bme280Initialized; }
    bool isMPU9250Ready() { return mpu9250Initialized; }
};

#endif