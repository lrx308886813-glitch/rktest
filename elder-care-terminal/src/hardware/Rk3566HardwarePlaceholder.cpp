#include "hardware/Rk3566HardwarePlaceholder.hpp"

#include "core/Logger.hpp"

#include <utility>

namespace ect {

Rk3566HardwarePlaceholder::Rk3566HardwarePlaceholder(std::string deviceConfigPath)
    : deviceConfigPath_(std::move(deviceConfigPath)) {}

void Rk3566HardwarePlaceholder::describeTodo() const {
    Logger::instance().warn("RK3566 hardware placeholder active, configure real devices in " + deviceConfigPath_);
    Logger::instance().warn("TODO: implement camera with V4L2/OpenCV after real camera_device is known");
    Logger::instance().warn("TODO: implement audio with ALSA/aplay after real audio_device is known");
    Logger::instance().warn("TODO: implement GPIO buttons after gpiochip and line numbers are known");
}

bool Rk3566HardwarePlaceholder::callPrimaryViaFutureBackend(const std::string& phoneOrEndpoint) {
    Logger::instance().warn("TODO: call backend placeholder for 4G/SIP/WebRTC target=" + phoneOrEndpoint);
    return false;
}

} // namespace ect
