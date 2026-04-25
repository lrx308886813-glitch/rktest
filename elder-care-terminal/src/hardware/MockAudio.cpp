#include "hardware/MockAudio.hpp"

#include "core/Logger.hpp"

namespace ect {

bool MockAudio::playAlert(const std::string& label) {
    Logger::instance().info("MockAudio playAlert: " + label);
    return true;
}

bool MockAudio::playMessage(const std::string& text) {
    Logger::instance().info("MockAudio playMessage: " + text);
    return true;
}

void MockAudio::stop() {
    Logger::instance().info("MockAudio stop");
}

} // namespace ect
