#pragma once

#include <mutex>
#include <string>

namespace ect::core {

class Logger {
public:
    explicit Logger(std::string log_path);
    void log(const std::string &message);

private:
    std::string log_path_;
    std::mutex mutex_;
};

} // namespace ect::core
