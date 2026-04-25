#include "core/Logger.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace ect {

namespace {
std::string levelName(LogLevel level) {
    switch (level) {
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO: return "INFO";
    case LogLevel::WARN: return "WARN";
    case LogLevel::ERROR: return "ERROR";
    }
    return "INFO";
}
} // namespace

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    logFile_ = path;
}

void Logger::debug(const std::string& message) { log(LogLevel::DEBUG, message); }
void Logger::info(const std::string& message) { log(LogLevel::INFO, message); }
void Logger::warn(const std::string& message) { log(LogLevel::WARN, message); }
void Logger::error(const std::string& message) { log(LogLevel::ERROR, message); }

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (static_cast<int>(level) < static_cast<int>(level_)) {
        return;
    }
    const std::string line = nowIsoString() + " [" + levelName(level) + "] " + message;
    std::cout << line << std::endl;
    if (!logFile_.empty()) {
        std::ofstream out(logFile_, std::ios::app);
        out << line << '\n';
    }
}

std::vector<std::string> Logger::recent(std::size_t maxLines) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> lines;
    if (logFile_.empty()) {
        return lines;
    }
    std::ifstream in(logFile_);
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
        if (lines.size() > maxLines) {
            lines.erase(lines.begin());
        }
    }
    return lines;
}

LogLevel parseLogLevel(const std::string& value) {
    if (value == "DEBUG") return LogLevel::DEBUG;
    if (value == "WARN") return LogLevel::WARN;
    if (value == "ERROR") return LogLevel::ERROR;
    return LogLevel::INFO;
}

std::string nowIsoString() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

} // namespace ect
