#pragma once

#include "core/Config.hpp"
#include "core/StateMachine.hpp"
#include "services/CameraService.hpp"

#include <string>

namespace ect {

class DeviceService {
public:
    DeviceService(const Config& config, CameraService& camera, StateMachine& stateMachine);
    std::string statusJson();

private:
    const Config& config_;
    CameraService& camera_;
    StateMachine& stateMachine_;
};

} // namespace ect
