#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

class WifiManager {
private:
    WiFiUDP ntpUDP;
    NTPClient* timeClient;
    
    bool wifiConnected;
    bool ntpInitialized;
    
    unsigned long lastReconnectAttempt;
    const unsigned long RECONNECT_INTERVAL = 30000;
    
public:
    WifiManager();
    ~WifiManager();
    
    bool begin();
    bool connect();
    bool isConnected();
    void disconnect();
    
    bool initNTP();
    void updateTime();
    unsigned long getEpochTime();
    String getFormattedTime();
    
    void printNetworkInfo();
    void handleReconnect();
};

#endif