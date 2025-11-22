#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

namespace VortexEngine {
    Logger& Logger::getInstance() {
        static Logger instance;
        return instance;
    }

    void Logger::log(LogLevel level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::cout << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] ";
        
        switch (level) {
            case LogLevel::DEBUG:
                std::cout << "[DEBUG] ";
                break;
            case LogLevel::INFO:
                std::cout << "[INFO] ";
                break;
            case LogLevel::WARNING:
                std::cout << "[WARNING] ";
                break;
            case LogLevel::ERROR:
                std::cout << "[ERROR] ";
                break;
        }
        
        std::cout << message << std::endl;
    }

    void Logger::debug(const std::string& message) {
        log(LogLevel::DEBUG, message);
    }

    void Logger::info(const std::string& message) {
        log(LogLevel::INFO, message);
    }

    void Logger::warning(const std::string& message) {
        log(LogLevel::WARNING, message);
    }

    void Logger::error(const std::string& message) {
        log(LogLevel::ERROR, message);
    }
}