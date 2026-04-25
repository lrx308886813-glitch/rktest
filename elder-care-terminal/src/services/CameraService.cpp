#include "services/CameraService.hpp"

#include <utility>

namespace ect {

CameraService::CameraService(std::shared_ptr<ICamera> camera) : camera_(std::move(camera)) {}

bool CameraService::detectPresence() {
    return camera_ && camera_->detectPresence();
}

} // namespace ect
