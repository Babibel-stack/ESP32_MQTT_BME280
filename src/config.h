#ifndef CONFIG_H
#define CONFIG_H

// ========== WLAN Konfiguration ==========
#define WIFI_SSID "TP-Link_2640"
#define WIFI_PASSWORD "46813374"
#define WIFI_TIMEOUT_MS 20000

// ========== persönlicher Hotspot Konfiguration ==========
//#define WIFI_SSID "iPhone"
//#define WIFI_PASSWORD "egdM-frqL-6yyL-Xqww"

// ========== NTP Konfiguration ==========
#define NTP_SERVER "pool.ntp.org"
#define NTP_OFFSET_SECONDS 3600
#define NTP_UPDATE_INTERVAL_MS 60000

// ========== Azure IoT Hub ==========
#define IOT_HUB_HOSTNAME "iotHubIvanFoka.azure-devices.net"
#define DEVICE_ID "iotWeatherstationesp32"
#define DEVICE_KEY "VCykuAaTiZdmGcRh+HTZXGCjbuy6TWUDSkWZFCzpdDY="

// MQTT Topics
#define MQTT_TELEMETRY_TOPIC "devices/" DEVICE_ID "/messages/events/"
#define MQTT_C2D_TOPIC "devices/" DEVICE_ID "/messages/devicebound/#"

// ========== MQTT QoS Konfiguration ========== ✅ NEU!
#define MQTT_QOS_LEVEL 1              // 0=keine Bestätigung, 1=PUBACK, 2=PUBCOMP
//#define MQTT_PORT 8883                // Azure IoT Hub MQTT Port (TLS)
//#define RECONNECT_INTERVAL 5000       // Reconnect alle 5 Sekunden

// ========== Sensor Konfiguration ==========
#define SENSOR_READ_INTERVAL_MS 5000

// ========== LED Pin ==========
#define LED_PIN 23

// ========== Debug Level ==========
#define DEBUG_LEVEL 3

#endif