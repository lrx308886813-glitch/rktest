#include "services/DeviceService.hpp"

#include "core/Logger.hpp"
#include "core/Storage.hpp"

#include <sstream>

namespace ect {

DeviceService::DeviceService(const Config& config, CameraService& camera, StateMachine& stateMachine)
    : config_(config), camera_(camera), stateMachine_(stateMachine) {}

std::string DeviceService::statusJson() {
    const auto& d = config_.device();
    const bool presence = camera_.detectPresence();
    std::ostringstream out;
    out << "{"
        << "\"device_name\":\"" << escapeJson(d.deviceName) << "\","
        << "\"device_id\":\"" << escapeJson(d.deviceId) << "\","
        << "\"platform\":\"" << escapeJson(d.platform) << "\","
        << "\"mock_mode\":" << (d.mockMode ? "true" : "false") << ","
        << "\"display_width\":" << d.displayWidth << ","
        << "\"display_height\":" << d.displayHeight << ","
        << "\"state\":\"" << toString(stateMachine_.state()) << "\","
        << "\"presence\":" << (presence ? "true" : "false") << ","
        << "\"time\":\"" << nowIsoString() << "\""
        << "}";
    return out.str();
}

} // namespace ect
