#include "sensors.h"
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <MPU9250_asukiaaa.h>

Adafruit_BME280 bme;
MPU9250_asukiaaa mpu;   // Deine Bibliothek

void sensors_init() {
    Wire.begin();

    // BME280
    if (!bme.begin(0x76)) {
        Serial.println("BME280 not found!");
    }

    // MPU
    mpu.beginAccel();
    mpu.beginGyro();
    mpu.beginMag();
}

SensorData sensors_read() {
    SensorData d;

    d.temperature = bme.readTemperature();
    d.humidity    = bme.readHumidity();
    d.pressure    = bme.readPressure() / 100.0F;

    mpu.accelUpdate();
    d.accelX = mpu.accelX();
    d.accelY = mpu.accelY();
    d.accelZ = mpu.accelZ();

    return d;
}
