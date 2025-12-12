#include "logger.h"
#include <stdarg.h>

LogLevel Logger::currentLevel = LOG_INFO;

// ANSI Farbcodes für bessere Lesbarkeit
const char* Logger::getColor(LogLevel level) {
    switch(level) {
        case LOG_ERROR:   return "\033[1;31m"; // Rot
        case LOG_WARN:    return "\033[1;33m"; // Gelb
        case LOG_INFO:    return "\033[1;32m"; // Grün
        case LOG_DEBUG:   return "\033[1;36m"; // Cyan
        case LOG_VERBOSE: return "\033[1;37m"; // Weiß
        default:          return "\033[0m";    // Reset
    }
}

const char* Logger::getLevelString(LogLevel level) {
    switch(level) {
        case LOG_ERROR:   return "ERROR";
        case LOG_WARN:    return "WARN ";
        case LOG_INFO:    return "INFO ";
        case LOG_DEBUG:   return "DEBUG";
        case LOG_VERBOSE: return "VERB ";
        default:          return "?????";
    }
}

void Logger::init(LogLevel level) {
    Serial.begin(115200);
    while(!Serial && millis() < 3000); // Warte max 3s auf Serial
    currentLevel = level;
    
    printSeparator();
    Serial.println("\033[1;34m╔═══════════════════════════════════════╗");
    Serial.println("║   ESP32 IoT Wetterstation v1.0       ║");
    Serial.println("╚═══════════════════════════════════════╝\033[0m");
    printSeparator();
}

void Logger::setLevel(LogLevel level) {
    currentLevel = level;
}

void Logger::error(const char* tag, const char* format, ...) {
    if (currentLevel < LOG_ERROR) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    Serial.printf("%s[%s] [%10lu] [%s] %s\033[0m\n", 
                  getColor(LOG_ERROR),
                  getLevelString(LOG_ERROR),
                  millis(),
                  tag,
                  buffer);
}

void Logger::warn(const char* tag, const char* format, ...) {
    if (currentLevel < LOG_WARN) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    Serial.printf("%s[%s] [%10lu] [%s] %s\033[0m\n", 
                  getColor(LOG_WARN),
                  getLevelString(LOG_WARN),
                  millis(),
                  tag,
                  buffer);
}

void Logger::info(const char* tag, const char* format, ...) {
    if (currentLevel < LOG_INFO) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    Serial.printf("%s[%s] [%10lu] [%s] %s\033[0m\n", 
                  getColor(LOG_INFO),
                  getLevelString(LOG_INFO),
                  millis(),
                  tag,
                  buffer);
}

void Logger::debug(const char* tag, const char* format, ...) {
    if (currentLevel < LOG_DEBUG) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    Serial.printf("%s[%s] [%10lu] [%s] %s\033[0m\n", 
                  getColor(LOG_DEBUG),
                  getLevelString(LOG_DEBUG),
                  millis(),
                  tag,
                  buffer);
}

void Logger::verbose(const char* tag, const char* format, ...) {
    if (currentLevel < LOG_VERBOSE) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    Serial.printf("%s[%s] [%10lu] [%s] %s\033[0m\n", 
                  getColor(LOG_VERBOSE),
                  getLevelString(LOG_VERBOSE),
                  millis(),
                  tag,
                  buffer);
}

void Logger::printSeparator() {
    Serial.println("═══════════════════════════════════════════════════════");
}

void Logger::printHeader(const char* text) {
    printSeparator();
    Serial.printf("  %s\n", text);
    printSeparator();
}