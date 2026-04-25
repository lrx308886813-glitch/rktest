#include "core/logger.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace ect::core {

namespace {
std::string nowString() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
} // namespace

Logger::Logger(std::string log_path) : log_path_(std::move(log_path)) {}

void Logger::log(const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream out(log_path_, std::ios::app);
    out << "[" << nowString() << "] " << message << '\n';
}

} // namespace ect::core
