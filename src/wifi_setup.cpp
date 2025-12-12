#include <Arduino.h>
#include <WiFi.h>
#include "wifi_setup.h"
#include "config.h"

WifiManager::WifiManager() : timeClient(nullptr), wifiConnected(false), ntpInitialized(false), lastReconnectAttempt(0) {
}

WifiManager::~WifiManager() {
    if (timeClient != nullptr) {
        delete timeClient;
    }
}

bool WifiManager::begin() {
    Serial.println("\n=== WLAN Initialisierung ===");
    
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    
    return connect();
}

bool WifiManager::connect() {
    Serial.print("Verbinde mit WLAN: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startAttempt = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_TIMEOUT_MS) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("✅ WLAN verbunden!");
        printNetworkInfo();
        
        if (initNTP()) {
            Serial.println("✅ NTP synchronisiert!");
        }
        
        return true;
    } else {
        wifiConnected = false;
        Serial.println("❌ WLAN Verbindung fehlgeschlagen!");
        Serial.println("   Prüfe SSID und Passwort in config.h");
        return false;
    }
}

bool WifiManager::isConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

void WifiManager::disconnect() {
    WiFi.disconnect();
    wifiConnected = false;
    Serial.println("WLAN getrennt");
}

bool WifiManager::initNTP() {
    Serial.print("Initialisiere NTP... ");
    
    timeClient = new NTPClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SECONDS, NTP_UPDATE_INTERVAL_MS);
    timeClient->begin();
    
    int attempts = 0;
    while (!timeClient->update() && attempts < 5) {
        Serial.print(".");
        timeClient->forceUpdate();
        delay(1000);
        attempts++;
    }
    Serial.println();
    
    if (attempts < 5) {
        ntpInitialized = true;
        Serial.print("Aktuelle Zeit: ");
        Serial.println(getFormattedTime());
        Serial.printf("Epoch Time: %lu\n", getEpochTime());
        return true;
    } else {
        ntpInitialized = false;
        Serial.println("❌ NTP Synchronisation fehlgeschlagen!");
        return false;
    }
}

void WifiManager::updateTime() {
    if (ntpInitialized && timeClient != nullptr) {
        timeClient->update();
    }
}

unsigned long WifiManager::getEpochTime() {
    if (ntpInitialized && timeClient != nullptr) {
        return timeClient->getEpochTime();
    }
    return 0;
}

String WifiManager::getFormattedTime() {
    if (ntpInitialized && timeClient != nullptr) {
        return timeClient->getFormattedTime();
    }
    return "00:00:00";
}

void WifiManager::printNetworkInfo() {
    Serial.println("\n--- Netzwerk Informationen ---");
    Serial.print("  IP Adresse:    ");
    Serial.println(WiFi.localIP());
    Serial.print("  Subnet Mask:   ");
    Serial.println(WiFi.subnetMask());
    Serial.print("  Gateway:       ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("  DNS:           ");
    Serial.println(WiFi.dnsIP());
    Serial.print("  MAC Adresse:   ");
    Serial.println(WiFi.macAddress());
    Serial.print("  RSSI:          ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println("-------------------------------\n");
}

void WifiManager::handleReconnect() {
    if (!isConnected()) {
        unsigned long now = millis();
        
        if (now - lastReconnectAttempt >= RECONNECT_INTERVAL) {
            lastReconnectAttempt = now;
            
            Serial.println("⚠️  WLAN Verbindung verloren - Reconnect...");
            
            WiFi.disconnect();
            delay(1000);
            
            if (connect()) {
                Serial.println("✅ Reconnect erfolgreich!");
            } else {
                Serial.println("❌ Reconnect fehlgeschlagen, versuche später erneut...");
            }
        }
    }
}