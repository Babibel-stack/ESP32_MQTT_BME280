#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Log Levels
enum LogLevel {
    LOG_ERROR = 0,
    LOG_WARN = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3,
    LOG_VERBOSE = 4
};

class Logger {
private:
    static LogLevel currentLevel;
    static const char* getLevelString(LogLevel level);
    static const char* getColor(LogLevel level);
    
public:
    static void init(LogLevel level = LOG_INFO);
    static void setLevel(LogLevel level);
    
    static void error(const char* tag, const char* format, ...);
    static void warn(const char* tag, const char* format, ...);
    static void info(const char* tag, const char* format, ...);
    static void debug(const char* tag, const char* format, ...);
    static void verbose(const char* tag, const char* format, ...);
    
    static void printSeparator();
    static void printHeader(const char* text);
};

// Makros für einfache Nutzung
#define LOG_E(tag, format, ...) Logger::error(tag, format, ##__VA_ARGS__)
#define LOG_W(tag, format, ...) Logger::warn(tag, format, ##__VA_ARGS__)
#define LOG_I(tag, format, ...) Logger::info(tag, format, ##__VA_ARGS__)
#define LOG_D(tag, format, ...) Logger::debug(tag, format, ##__VA_ARGS__)
#define LOG_V(tag, format, ...) Logger::verbose(tag, format, ##__VA_ARGS__)

#endif