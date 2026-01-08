#include "sas.h"
#include "mbedtls/base64.h"  // mbedTLS Bibliothek für Base64 En-/Dekodierung
#include "mbedtls/md.h"       // mbedTLS Bibliothek für HMAC-SHA256
#include <WiFi.h>
#include "wifi_setup.h"

// Konstruktor: Keine Initialisierung erforderlich
SASToken::SASToken() {
}

// ===== URL Encoding =====
// Konvertiert Sonderzeichen in URL-sichere Format (%XX)
// Notwendig für korrekte SAS-Token Formatierung
String SASToken::urlEncode(const String& str) {
    String encoded = "";
    char c;
    char code0, code1;  // Hexadezimale Zeichen für kodierung
    
    // Jedes Zeichen durchgehen
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        
        if (c == ' ') {
            // Leerzeichen wird zu '+' (URL-Standard)
            encoded += '+';
        } else if (isalnum(c)) {
            // Buchstaben und Zahlen bleiben unverändert
            encoded += c;
        } else {
            // Sonderzeichen werden in Hexadezimal kodiert (%XX)
            
            // Untere 4 Bits (Low Nibble)
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';  // A-F für 10-15
            }
            
            // Obere 4 Bits (High Nibble)
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';  // A-F für 10-15
            }
            
            // %XX Format zusammenbauen
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }
    return encoded;
}

// ===== Base64 Dekodierung =====
// Dekodiert Base64-kodierten Device Key zurück in Binärformat
// Azure IoT Hub Device Keys sind Base64-kodiert gespeichert
String SASToken::base64Decode(const String& input) {
    size_t outputLen;
    
    // ===== Schritt 1: Benötigte Ausgabegröße berechnen =====
    // Nullpointer-Aufruf um nur Größe zu ermitteln
    mbedtls_base64_decode(NULL, 0, &outputLen, 
                         (const unsigned char*)input.c_str(), 
                         input.length());
    
    // Speicher für dekodierte Daten allokieren (+1 für Null-Terminator)
    unsigned char* output = new unsigned char[outputLen + 1];
    
    // ===== Schritt 2: Tatsächliche Dekodierung durchführen =====
    int ret = mbedtls_base64_decode(output, outputLen, &outputLen,
                                    (const unsigned char*)input.c_str(),
                                    input.length());
    
    // Fehlerbehandlung
    if (ret != 0) {
        delete[] output;
        return "";  // Leerer String bei Fehler
    }
    
    // Null-Terminator hinzufügen
    output[outputLen] = '\0';
    
    // In String konvertieren und Speicher freigeben
    String result = String((char*)output);
    delete[] output;
    
    return result;
}

// ===== HMAC-SHA256 Signatur =====
// Erstellt kryptographische Signatur mit HMAC-SHA256 Algorithmus
// Diese Signatur beweist gegenüber Azure IoT Hub, dass wir den Device Key kennen
String SASToken::hmacSha256(const String& key, const String& data) {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;  // SHA256 Hash-Algorithmus
    const size_t messageLength = data.length();
    const size_t keyLength = key.length();
    
    unsigned char hmacResult[32];  // SHA256 erzeugt immer 32 Bytes (256 Bits)
    
    // ===== HMAC Berechnung =====
    mbedtls_md_init(&ctx);  // Kontext initialisieren
    
    // Setup mit SHA256 und HMAC-Modus (1 = HMAC aktiviert)
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    
    // HMAC starten mit Schlüssel
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key.c_str(), keyLength);
    
    // Daten verarbeiten (kann mehrfach aufgerufen werden für große Daten)
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)data.c_str(), messageLength);
    
    // HMAC abschließen und Resultat in hmacResult speichern
    mbedtls_md_hmac_finish(&ctx, hmacResult);
    
    // Kontext freigeben
    mbedtls_md_free(&ctx);
    
    // ===== Base64 Encoding des HMAC-Results =====
    // Azure IoT Hub erwartet die Signatur Base64-kodiert
    
    // Schritt 1: Benötigte Größe berechnen
    size_t outputLen;
    mbedtls_base64_encode(NULL, 0, &outputLen, hmacResult, 32);
    
    // Schritt 2: Speicher allokieren und kodieren
    unsigned char* encoded = new unsigned char[outputLen + 1];
    mbedtls_base64_encode(encoded, outputLen, &outputLen, hmacResult, 32);
    encoded[outputLen] = '\0';
    
    // In String konvertieren und Speicher freigeben
    String result = String((char*)encoded);
    delete[] encoded;
    
    return result;
}

// ===== SAS-Token Generierung =====
// Generiert ein vollständiges Shared Access Signature Token für Azure IoT Hub
// Format: SharedAccessSignature sr={resource}&sig={signature}&se={expiry}
String SASToken::generate(const char* hostname, 
                         const char* deviceId, 
                         const char* deviceKey,
                         unsigned long expiryInSeconds) {
    
    // ===== Schritt 1: String to Sign erstellen =====
    // Format: {resource}\n{expiry}
    // resource = hostname/devices/deviceId
    String resourceUri = String(hostname) + "/devices/" + String(deviceId);
    String stringToSign = resourceUri + "\n" + String(expiryInSeconds);
    
    // ===== Schritt 2: Device Key dekodieren =====
    // Der Device Key aus Azure IoT Hub ist Base64-kodiert
    String decodedKey = base64Decode(String(deviceKey));
    
    // ===== Schritt 3: Signatur erstellen =====
    // HMAC-SHA256 des "String to Sign" mit dekodiertem Key
    String signature = hmacSha256(decodedKey, stringToSign);
    
    // ===== Schritt 4: Signatur URL-kodieren =====
    // Signatur enthält Base64-Zeichen (+, /, =) die URL-kodiert werden müssen
    String encodedSignature = urlEncode(signature);
    
    // ===== Schritt 5: SAS Token zusammenbauen =====
    // Azure IoT Hub erwartet dieses spezifische Format:
    // SharedAccessSignature sr={resource}&sig={signature}&se={expiry}
    // - sr = Shared Resource (welches Device)
    // - sig = Signature (Beweis dass wir den Key kennen)
    // - se = Expiry (Unix-Timestamp wann Token ungültig wird)
    String sasToken = "SharedAccessSignature sr=" + resourceUri + 
                     "&sig=" + encodedSignature + 
                     "&se=" + String(expiryInSeconds);
    
    return sasToken;
}

// ===== SAS-Token mit Standardgültigkeit =====
// Convenience-Funktion: Generiert Token mit 24 Stunden Gültigkeit
// Dies ist die am häufigsten verwendete Funktion
String SASToken::generateDefault(const char* hostname, 
                                const char* deviceId, 
                                const char* deviceKey,
                                unsigned long currentEpoch) {
    
    // Ablaufzeit = Aktuelle Zeit + 86400 Sekunden (24 Stunden)
    unsigned long expiry = currentEpoch + 86400;
    
    // Token mit berechneter Ablaufzeit generieren
    return generate(hostname, deviceId, deviceKey, expiry);
}