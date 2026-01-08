#include <Arduino.h>
#include <WiFi.h>
#include "wifi_setup.h"
#include "config.h"

// Konstruktor: Initialisiert alle Variablen mit Standardwerten
WifiManager::WifiManager() : timeClient(nullptr), wifiConnected(false), ntpInitialized(false), lastReconnectAttempt(0) {
}

// Destruktor: Gibt den Speicher des NTP-Clients frei
WifiManager::~WifiManager() {
    if (timeClient != nullptr) {
        delete timeClient;
    }
}

// Hauptinitialisierungsmethode für den WiFi-Manager
bool WifiManager::begin() {
    Serial.println("\n=== WLAN Initialisierung ===");
    
    // ESP32 als Station-Modus konfigurieren (nicht als Access Point)
    WiFi.mode(WIFI_STA);
    // Automatische Wiederverbindung bei Verbindungsverlust aktivieren
    WiFi.setAutoReconnect(true);
    
    // Verbindungsaufbau starten
    return connect();
}

// Stellt die Verbindung zum WLAN her
bool WifiManager::connect() {
    Serial.print("Verbinde mit WLAN: ");
    Serial.println(WIFI_SSID);
    
    // Verbindungsversuch mit SSID und Passwort aus config.h
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Zeitstempel für Timeout-Überwachung
    unsigned long startAttempt = millis();
    
    // Wartet auf Verbindung bis Timeout erreicht ist
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_TIMEOUT_MS) {
        Serial.print(".");  // Fortschrittsanzeige
        delay(500);
    }
    Serial.println();
    
    // Prüfung ob Verbindung erfolgreich
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("✅ WLAN verbunden!");
        printNetworkInfo();  // Netzwerkdetails ausgeben
        
        // NTP-Zeitsynchronisation initialisieren
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

// Gibt den aktuellen WLAN-Verbindungsstatus zurück
bool WifiManager::isConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

// Trennt die WLAN-Verbindung
void WifiManager::disconnect() {
    WiFi.disconnect();
    wifiConnected = false;
    Serial.println("WLAN getrennt");
}

// Initialisiert den NTP-Client für Zeitsynchronisation
bool WifiManager::initNTP() {
    Serial.print("Initialisiere NTP... ");
    
    // NTP-Client mit Server, Zeitzone-Offset und Update-Intervall erstellen
    timeClient = new NTPClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SECONDS, NTP_UPDATE_INTERVAL_MS);
    timeClient->begin();
    
    // Bis zu 5 Versuche für erfolgreiche Zeitsynchronisation
    int attempts = 0;
    while (!timeClient->update() && attempts < 5) {
        Serial.print(".");
        timeClient->forceUpdate();  // Erzwinge Update-Versuch
        delay(1000);
        attempts++;
    }
    Serial.println();
    
    // Prüfung ob Synchronisation erfolgreich
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

// Aktualisiert die Zeit vom NTP-Server
void WifiManager::updateTime() {
    if (ntpInitialized && timeClient != nullptr) {
        timeClient->update();
    }
}

// Gibt die aktuelle Zeit als Unix-Timestamp (Sekunden seit 1.1.1970) zurück
unsigned long WifiManager::getEpochTime() {
    if (ntpInitialized && timeClient != nullptr) {
        return timeClient->getEpochTime();
    }
    return 0;  // Fallback wenn NTP nicht initialisiert
}

// Gibt die aktuelle Zeit als formatierten String (HH:MM:SS) zurück
String WifiManager::getFormattedTime() {
    if (ntpInitialized && timeClient != nullptr) {
        return timeClient->getFormattedTime();
    }
    return "00:00:00";  // Fallback wenn NTP nicht initialisiert
}

// Gibt detaillierte Netzwerkinformationen auf der seriellen Konsole aus
void WifiManager::printNetworkInfo() {
    Serial.println("\n--- Netzwerk Informationen ---");
    Serial.print("  IP Adresse:    ");
    Serial.println(WiFi.localIP());           // Zugewiesene IP-Adresse
    Serial.print("  Subnet Mask:   ");
    Serial.println(WiFi.subnetMask());        // Subnetzmaske
    Serial.print("  Gateway:       ");
    Serial.println(WiFi.gatewayIP());         // Gateway (Router)
    Serial.print("  DNS:           ");
    Serial.println(WiFi.dnsIP());             // DNS-Server
    Serial.print("  MAC Adresse:   ");
    Serial.println(WiFi.macAddress());        // Hardware-Adresse des ESP32
    Serial.print("  RSSI:          ");
    Serial.print(WiFi.RSSI());                // Signalstärke in dBm
    Serial.println(" dBm");
    Serial.println("-------------------------------\n");
}

// Überwacht die Verbindung und stellt sie bei Bedarf wieder her
void WifiManager::handleReconnect() {
    if (!isConnected()) {
        unsigned long now = millis();
        
        // Reconnect nur alle RECONNECT_INTERVAL Millisekunden versuchen
        if (now - lastReconnectAttempt >= RECONNECT_INTERVAL) {
            lastReconnectAttempt = now;
            
            Serial.println("⚠️  WLAN Verbindung verloren - Reconnect...");
            
            // Bestehende Verbindung sauber trennen
            WiFi.disconnect();
            delay(1000);
            
            // Neuer Verbindungsversuch
            if (connect()) {
                Serial.println("✅ Reconnect erfolgreich!");
                // NTP nach erfolgreicher Wiederverbindung neu initialisieren
                if (!ntpInitialized) {
                    initNTP();
                }
            } else {
                Serial.println("❌ Reconnect fehlgeschlagen, versuche später erneut...");
            }
        }
    }
}