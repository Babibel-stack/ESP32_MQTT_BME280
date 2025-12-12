#ifndef CONFIG_H
#define CONFIG_H

// ========== WLAN Konfiguration ==========
#define WIFI_SSID "TP-Link_2640"          // DEINE WLAN SSID
#define WIFI_PASSWORD "46813374"          // DEIN WLAN Passwort
#define WIFI_TIMEOUT_MS 20000             // 20 Sekunden Timeout

// ========== NTP Konfiguration ==========
#define NTP_SERVER "pool.ntp.org"         // NTP Server
#define NTP_OFFSET_SECONDS 3600           // GMT+1 (Deutschland Winter)
#define NTP_UPDATE_INTERVAL_MS 60000      // Alle 60 Sekunden aktualisieren

// ========== Azure IoT Hub (später) ==========
// #define IOT_HUB_HOSTNAME "dein-hub.azure-devices.net"
// #define DEVICE_ID "esp32-wetterstation"
// #define DEVICE_KEY "dein-device-key-hier"

// ========== Sensor Konfiguration ==========
#define SENSOR_READ_INTERVAL_MS 5000      // Sensoren alle 5 Sekunden auslesen

// ========== LED Pin ==========
#define LED_PIN 2                          // Onboard LED (GPIO2)

// ========== Debug Level ==========
#define DEBUG_LEVEL 3                      // 0=None, 1=Error, 2=Warn, 3=Info, 4=Debug

#endif