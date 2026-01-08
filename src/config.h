#ifndef CONFIG_H
#define CONFIG_H

// ========== WLAN Konfiguration ==========
#define WIFI_SSID "TP-Link_2640"          // DEINE WLAN SSID
#define WIFI_PASSWORD "46813374"          // DEIN WLAN Passwort
#define WIFI_TIMEOUT_MS 20000             // 20 Sekunden Timeout


// ========== persönlicher Hotspot Konfiguration ==========
//#define WIFI_SSID "iPhone"          // DEINE Hotspot SSID
//#define WIFI_PASSWORD "egdM-frqL-6yyL-Xqww"          // DEIN  Hotspot Passwort

// ========== NTP Konfiguration ==========
#define NTP_SERVER "pool.ntp.org"         // NTP Server
#define NTP_OFFSET_SECONDS 3600           // GMT+1 (Deutschland Winter)
#define NTP_UPDATE_INTERVAL_MS 60000      // Alle 60 Sekunden aktualisieren

// ========== Azure IoT Hub ==========
#define IOT_HUB_HOSTNAME "iotHubIvanFoka.azure-devices.net"  // ← Dein IoT Hub Name!
#define DEVICE_ID "iotWeatherstationesp32"
#define DEVICE_KEY "VCykuAaTiZdmGcRh+HTZXGCjbuy6TWUDSkWZFCzpdDY="  // ← Füge deinen Primary Key ein!

// MQTT Topics
#define MQTT_TELEMETRY_TOPIC "devices/" DEVICE_ID "/messages/events/"
#define MQTT_C2D_TOPIC "devices/" DEVICE_ID "/messages/devicebound/#"

// ========== Sensor Konfiguration ==========
#define SENSOR_READ_INTERVAL_MS 5000      // Sensoren alle 5 Sekunden auslesen

// ========== LED Pin ==========
#define LED_PIN 23                         // Onboard LED (GPIO2)

// ========== Debug Level ==========
#define DEBUG_LEVEL 3                      // 0=None, 1=Error, 2=Warn, 3=Info, 4=Debug

#endif