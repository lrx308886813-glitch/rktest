#include "hardware/MockCamera.hpp"

#include "core/Logger.hpp"

#include <chrono>

namespace ect {

MockCamera::MockCamera(bool fixedPresence) : fixedPresence_(fixedPresence) {}

bool MockCamera::detectPresence() {
    if (fixedPresence_) {
        Logger::instance().debug("MockCamera detectPresence=true");
        return true;
    }
    const auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
    const bool present = (ticks % 2) == 0;
    Logger::instance().debug(std::string("MockCamera detectPresence=") + (present ? "true" : "false"));
    return present;
}

} // namespace ect
