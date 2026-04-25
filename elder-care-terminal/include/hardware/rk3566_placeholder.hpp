#pragma once

#include "hardware/interfaces.hpp"

#include <string>

namespace ect::hardware {

class Rk3566HardwarePlaceholder final : public IAudio, public ICamera, public IGpio {
public:
    bool playTone(const std::string &tone_name) override;
    bool captureFrame(const std::string &target_file) override;
    bool writePin(int pin, bool value) override;
    bool readPin(int pin) override;

    std::string extensionNotes() const;
};

} // namespace ect::hardware
