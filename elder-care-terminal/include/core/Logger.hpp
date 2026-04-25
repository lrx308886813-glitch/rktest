#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace ect {

enum class LogLevel { DEBUG = 0, INFO, WARN, ERROR };

class Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level);
    void setLogFile(const std::string& path);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    std::vector<std::string> recent(std::size_t maxLines) const;

private:
    Logger() = default;
    void log(LogLevel level, const std::string& message);

    mutable std::mutex mutex_;
    LogLevel level_{LogLevel::INFO};
    std::string logFile_;
};

LogLevel parseLogLevel(const std::string& value);
std::string nowIsoString();

} // namespace ect
