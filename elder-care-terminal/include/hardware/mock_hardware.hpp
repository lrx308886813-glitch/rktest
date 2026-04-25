#pragma once

#include "hardware/interfaces.hpp"

namespace ect::hardware {

class MockAudio final : public IAudio {
public:
    bool playTone(const std::string &tone_name) override;
};

class MockCamera final : public ICamera {
public:
    bool captureFrame(const std::string &target_file) override;
};

class MockGpio final : public IGpio {
public:
    bool writePin(int pin, bool value) override;
    bool readPin(int pin) override;
};

} // namespace ect::hardware
