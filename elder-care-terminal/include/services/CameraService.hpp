#pragma once

#include "hardware/ICamera.hpp"

#include <memory>

namespace ect {

class CameraService {
public:
    explicit CameraService(std::shared_ptr<ICamera> camera);
    bool detectPresence();

private:
    std::shared_ptr<ICamera> camera_;
};

} // namespace ect
