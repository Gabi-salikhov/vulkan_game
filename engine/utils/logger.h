#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>

namespace VortexEngine {

// Log level enumeration
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
    Off
};

// Log color codes (ANSI)
namespace LogColors {
    constexpr const char* Reset = "\033[0m";
    constexpr const char* Red = "\033[31m";
    constexpr const char* Green = "\033[32m";
    constexpr const char* Yellow = "\033[33m";
    constexpr const char* Blue = "\033[34m";
    constexpr const char* Magenta = "\033[35m";
    constexpr const char* Cyan = "\033[36m";
    constexpr const char* White = "\033[37m";
    constexpr const char* BrightRed = "\033[91m";
    constexpr const char* BrightGreen = "\033[92m";
    constexpr const char* BrightYellow = "\033[93m";
    constexpr const char* BrightBlue = "\033[94m";
    constexpr const char* BrightMagenta = "\033[95m";
    constexpr const char* BrightCyan = "\033[96m";
    constexpr const char* BrightWhite = "\033[97m";
}

// Log message structure
struct LogMessage {
    LogLevel level;
    std::string message;
    std::string timestamp;
    std::string source;
    uint32_t threadId;
    uint32_t line;
    std::string file;
    std::string function;
};

// Logger interface
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const LogMessage& message) = 0;
    virtual void setLogLevel(LogLevel level) = 0;
    virtual LogLevel getLogLevel() const = 0;
    virtual void setPattern(const std::string& pattern) = 0;
    virtual const std::string& getPattern() const = 0;
};

// Console logger
class ConsoleLogger : public ILogger {
public:
    ConsoleLogger();
    ~ConsoleLogger() override;

    // Logger interface
    void log(const LogMessage& message) override;
    void setLogLevel(LogLevel level) override { m_logLevel = level; }
    LogLevel getLogLevel() const override { return m_logLevel; }
    void setPattern(const std::string& pattern) override { m_pattern = pattern; }
    const std::string& getPattern() const override { return m_pattern; }

    // Console logger specific
    void enableColors(bool enable) { m_colorsEnabled = enable; }
    bool areColorsEnabled() const { return m_colorsEnabled; }
    void setFlushAfterLog(bool enable) { m_flushAfterLog = enable; }
    bool shouldFlushAfterLog() const { return m_flushAfterLog; }

private:
    LogLevel m_logLevel = LogLevel::Info;
    std::string m_pattern = "[%timestamp] [%level] [%source]: %message";
    bool m_colorsEnabled = true;
    bool m_flushAfterLog = true;

    // Color mapping
    std::string getLevelColor(LogLevel level) const;
    std::string formatMessage(const LogMessage& message) const;
};

// File logger
class FileLogger : public ILogger {
public:
    FileLogger();
    ~FileLogger() override;

    // Logger interface
    void log(const LogMessage& message) override;
    void setLogLevel(LogLevel level) override { m_logLevel = level; }
    LogLevel getLogLevel() const override { return m_logLevel; }
    void setPattern(const std::string& pattern) override { m_pattern = pattern; }
    const std::string& getPattern() const override { return m_pattern; }

    // File logger specific
    bool openFile(const std::string& filename);
    void closeFile();
    bool isOpen() const { return m_fileStream.is_open(); }
    void setMaxFileSize(size_t maxSize) { m_maxFileSize = maxSize; }
    size_t getMaxFileSize() const { return m_maxFileSize; }
    void enableRotation(bool enable) { m_rotationEnabled = enable; }
    bool isRotationEnabled() const { return m_rotationEnabled; }
    void setRotationCount(size_t count) { m_rotationCount = count; }
    size_t getRotationCount() const { return m_rotationCount; }

private:
    LogLevel m_logLevel = LogLevel::Info;
    std::string m_pattern = "[%timestamp] [%level] [%source]: %message";
    std::string m_filename;
    std::ofstream m_fileStream;
    size_t m_maxFileSize = 10 * 1024 * 1024; // 10 MB
    bool m_rotationEnabled = true;
    size_t m_rotationCount = 5;

    // Internal methods
    bool shouldRotate() const;
    void rotateFile();
    std::string getTimestamp() const;
    std::string formatMessage(const LogMessage& message) const;
};

// Multi-target logger
class MultiLogger : public ILogger {
public:
    MultiLogger();
    ~MultiLogger() override;

    // Logger interface
    void log(const LogMessage& message) override;
    void setLogLevel(LogLevel level) override;
    LogLevel getLogLevel() const override { return m_logLevel; }
    void setPattern(const std::string& pattern) override;
    const std::string& getPattern() const override { return m_pattern; }

    // Multi logger specific
    void addLogger(std::shared_ptr<ILogger> logger);
    void removeLogger(std::shared_ptr<ILogger> logger);
    void clearLoggers();
    size_t getLoggerCount() const { return m_loggers.size(); }

private:
    LogLevel m_logLevel = LogLevel::Info;
    std::string m_pattern = "[%timestamp] [%level] [%source]: %message";
    std::vector<std::shared_ptr<ILogger>> m_loggers;
};

// Main logger class
class Logger {
public:
    Logger();
    ~Logger();

    // Logger lifecycle
    bool initialize();
    void shutdown();

    // Logging functions
    void trace(const std::string& message, const std::string& source = "", uint32_t line = 0, const std::string& file = "", const std::string& function = "");
    void debug(const std::string& message, const std::string& source = "", uint32_t line = 0, const std::string& file = "", const std::string& function = "");
    void info(const std::string& message, const std::string& source = "", uint32_t line = 0, const std::string& file = "", const std::string& function = "");
    void warning(const std::string& message, const std::string& source = "", uint32_t line = 0, const std::string& file = "", const std::string& function = "");
    void error(const std::string& message, const std::string& source = "", uint32_t line = 0, const std::string& file = "", const std::string& function = "");
    void critical(const std::string& message, const std::string& source = "", uint32_t line = 0, const std::string& file = "", const std::string& function = "");

    // Logger configuration
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const { return m_logLevel; }
    void setPattern(const std::string& pattern);
    const std::string& getPattern() const { return m_pattern; }

    // Logger targets
    void addConsoleLogger();
    void addFileLogger(const std::string& filename);
    void addMultiLogger();
    void setLogger(std::shared_ptr<ILogger> logger);
    std::shared_ptr<ILogger> getLogger() const { return m_logger; }

    // Thread safety
    void setThreadSafe(bool enable) { m_threadSafe = enable; }
    bool isThreadSafe() const { return m_threadSafe; }

    // Singleton access
    static Logger& getInstance();
    static void initializeSingleton();
    static void shutdownSingleton();

    // Utility functions
    static std::string levelToString(LogLevel level);
    static LogLevel stringToLevel(const std::string& level);
    static std::string getCurrentTimestamp();
    static uint32_t getCurrentThreadId();

private:
    // Logger state
    bool m_initialized = false;
    LogLevel m_logLevel = LogLevel::Info;
    std::string m_pattern = "[%timestamp] [%level] [%source]: %message";
    bool m_threadSafe = true;
    std::mutex m_mutex;

    // Logger implementation
    std::shared_ptr<ILogger> m_logger;

    // Internal methods
    void logInternal(LogLevel level, const std::string& message, const std::string& source, uint32_t line, const std::string& file, const std::string& function);
    LogMessage createLogMessage(LogLevel level, const std::string& message, const std::string& source, uint32_t line, const std::string& file, const std::string& function) const;
    std::string formatTimestamp(const std::string& timestamp) const;
};

// Macro definitions for easy logging
#define VORTEX_TRACE(message) Logger::getInstance().trace(message, __FILE__, __LINE__, __FUNCTION__)
#define VORTEX_DEBUG(message) Logger::getInstance().debug(message, __FILE__, __LINE__, __FUNCTION__)
#define VORTEX_INFO(message) Logger::getInstance().info(message, __FILE__, __LINE__, __FUNCTION__)
#define VORTEX_WARNING(message) Logger::getInstance().warning(message, __FILE__, __LINE__, __FUNCTION__)
#define VORTEX_ERROR(message) Logger::getInstance().error(message, __FILE__, __LINE__, __FUNCTION__)
#define VORTEX_CRITICAL(message) Logger::getInstance().critical(message, __FILE__, __LINE__, __FUNCTION__)

// Conditional logging macros
#define VORTEX_TRACE_IF(condition, message) if (condition) VORTEX_TRACE(message)
#define VORTEX_DEBUG_IF(condition, message) if (condition) VORTEX_DEBUG(message)
#define VORTEX_INFO_IF(condition, message) if (condition) VORTEX_INFO(message)
#define VORTEX_WARNING_IF(condition, message) if (condition) VORTEX_WARNING(message)
#define VORTEX_ERROR_IF(condition, message) if (condition) VORTEX_ERROR(message)
#define VORTEX_CRITICAL_IF(condition, message) if (condition) VORTEX_CRITICAL(message)

// Performance timing macros
#define VORTEX_SCOPE_TIMER(name) \
    VortexEngine::ScopeTimer timer(name)

class ScopeTimer {
public:
    ScopeTimer(const std::string& name);
    ~ScopeTimer();

private:
    std::string m_name;
    std::chrono::high_resolution_clock::time_point m_startTime;
};

} // namespace VortexEngine