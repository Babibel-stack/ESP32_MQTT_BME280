#pragma once
#include <Arduino.h>
#include "sensors.h"

void mqtt_init(String sasToken);
void mqtt_sendTelemetry(SensorData data);
