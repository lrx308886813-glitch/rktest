#pragma once

#include "hardware/IAudio.hpp"

namespace ect {

class MockAudio final : public IAudio {
public:
    bool playAlert(const std::string& label) override;
    bool playMessage(const std::string& text) override;
    void stop() override;
};

} // namespace ect
