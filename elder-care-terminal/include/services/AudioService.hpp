#pragma once

#include "hardware/IAudio.hpp"

#include <memory>
#include <string>

namespace ect {

class AudioService {
public:
    explicit AudioService(std::shared_ptr<IAudio> audio);
    bool playAlert(const std::string& label);
    bool playMessage(const std::string& text);
    void stop();

private:
    std::shared_ptr<IAudio> audio_;
};

} // namespace ect
