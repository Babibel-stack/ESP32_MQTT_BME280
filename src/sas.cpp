#include "sas.h"
#include "mbedtls/base64.h"
#include "mbedtls/md.h"
#include <WiFi.h>
#include "wifi_setup.h"

SASToken::SASToken() {
}

// URL Encoding
String SASToken::urlEncode(const String& str) {
    String encoded = "";
    char c;
    char code0, code1;
    
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encoded += '+';
        } else if (isalnum(c)) {
            encoded += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }
    return encoded;
}

// Base64 Decode
String SASToken::base64Decode(const String& input) {
    size_t outputLen;
    
    // Berechne benötigte Größe
    mbedtls_base64_decode(NULL, 0, &outputLen, 
                         (const unsigned char*)input.c_str(), 
                         input.length());
    
    unsigned char* output = new unsigned char[outputLen + 1];
    
    int ret = mbedtls_base64_decode(output, outputLen, &outputLen,
                                    (const unsigned char*)input.c_str(),
                                    input.length());
    
    if (ret != 0) {
        delete[] output;
        return "";
    }
    
    output[outputLen] = '\0';
    String result = String((char*)output);
    delete[] output;
    
    return result;
}

// HMAC-SHA256
String SASToken::hmacSha256(const String& key, const String& data) {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    const size_t messageLength = data.length();
    const size_t keyLength = key.length();
    
    unsigned char hmacResult[32];
    
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key.c_str(), keyLength);
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)data.c_str(), messageLength);
    mbedtls_md_hmac_finish(&ctx, hmacResult);
    mbedtls_md_free(&ctx);
    
    // Base64 Encode
    size_t outputLen;
    mbedtls_base64_encode(NULL, 0, &outputLen, hmacResult, 32);
    
    unsigned char* encoded = new unsigned char[outputLen + 1];
    mbedtls_base64_encode(encoded, outputLen, &outputLen, hmacResult, 32);
    encoded[outputLen] = '\0';
    
    String result = String((char*)encoded);
    delete[] encoded;
    
    return result;
}

// Token generieren
String SASToken::generate(const char* hostname, 
                         const char* deviceId, 
                         const char* deviceKey,
                         unsigned long expiryInSeconds) {
    
    // String to Sign erstellen
    String resourceUri = String(hostname) + "/devices/" + String(deviceId);
    String stringToSign = resourceUri + "\n" + String(expiryInSeconds);
    
    // Key dekodieren
    String decodedKey = base64Decode(String(deviceKey));
    
    // Signatur erstellen
    String signature = hmacSha256(decodedKey, stringToSign);
    
    // URL-Encode Signatur
    String encodedSignature = urlEncode(signature);
    
    // SAS Token zusammenbauen
    String sasToken = "SharedAccessSignature sr=" + resourceUri + 
                     "&sig=" + encodedSignature + 
                     "&se=" + String(expiryInSeconds);
    
    return sasToken;
}

// Token mit Standardablauf (24 Stunden)
String SASToken::generateDefault(const char* hostname, 
                                const char* deviceId, 
                                const char* deviceKey,
                                unsigned long currentEpoch) {
    
    unsigned long expiry = currentEpoch + 86400; // +24 Stunden
    return generate(hostname, deviceId, deviceKey, expiry);
}