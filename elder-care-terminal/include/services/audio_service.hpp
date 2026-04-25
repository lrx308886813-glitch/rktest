#pragma once

#include "hardware/interfaces.hpp"

namespace ect::services {

class AudioService {
public:
    explicit AudioService(hardware::IAudio &audio);
    bool playAlert();

private:
    hardware::IAudio &audio_;
};

} // namespace ect::services
