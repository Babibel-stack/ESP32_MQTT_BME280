#ifndef SAS_H
#define SAS_H

#include <Arduino.h>

class SASToken {
private:
    String urlEncode(const String& str);
    String base64Decode(const String& input);
    String hmacSha256(const String& key, const String& data);
    
public:
    SASToken();
    
    // Generiert SAS-Token für Azure IoT Hub
    String generate(const char* hostname, 
                   const char* deviceId, 
                   const char* deviceKey, 
                   unsigned long expiryInSeconds);
    
    // Generiert Token mit Standardablauf (24 Stunden)
    String generateDefault(const char* hostname, 
                          const char* deviceId, 
                          const char* deviceKey,
                          unsigned long currentEpoch);
};

#endif