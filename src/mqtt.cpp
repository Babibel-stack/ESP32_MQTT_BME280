#include "mqtt.h"
#include <Arduino.h>

void mqtt_init(String sasToken) {
    Serial.println("MQTT init with SAS Token:");
    Serial.println(sasToken);
}

void mqtt_sendTelemetry(SensorData d) {
    Serial.print("Sending telemetry: ");
    Serial.println(d.temperature);
}
