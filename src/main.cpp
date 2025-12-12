#include <Arduino.h>
#include "wifi.h"
#include "sensors.h"
#include "sas.h"
#include "mqtt.h"

void setup() {
    Serial.begin(115200);

    // WLAN + Zeit
    wifi_init();
    wifi_syncTime();

    // Sensoren
    sensors_init();

    // SAS-Token generieren
    String sasToken = sas_generateToken();
    Serial.println("SAS Token: " + sasToken);

    // IoT Hub verbinden
    mqtt_init(sasToken);
}

void loop() {
    SensorData data = sensors_read();
    mqtt_sendTelemetry(data);
    delay(3000);
}
