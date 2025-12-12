#pragma once
#include <Arduino.h>

struct SensorData {
    float temperature;
    float humidity;
    float pressure;
    float accelX, accelY, accelZ;
};

void sensors_init();
SensorData sensors_read();

