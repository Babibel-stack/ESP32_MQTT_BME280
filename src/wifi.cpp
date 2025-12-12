#include <Arduino.h>  // Wichtig für ESP32/Arduino Core
#include <WiFi.h>
#include <time.h>
#include "wifi.h"


const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";

void wifi_init() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
}

void wifi_syncTime() {
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    Serial.print("Syncing time... ");
    
    time_t now = time(nullptr);
    while (now < 100000) {
        delay(200);
        now = time(nullptr);
        Serial.print(".");
    }

    Serial.println("\nTime synced.");
}
