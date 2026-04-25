#pragma once

#include "hardware/ICamera.hpp"

namespace ect {

class MockCamera final : public ICamera {
public:
    explicit MockCamera(bool fixedPresence);
    bool detectPresence() override;

private:
    bool fixedPresence_;
};

} // namespace ect
