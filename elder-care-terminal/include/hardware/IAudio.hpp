#pragma once

#include <string>

namespace ect {

class IAudio {
public:
    virtual ~IAudio() = default;
    virtual bool playAlert(const std::string& label) = 0;
    virtual bool playMessage(const std::string& text) = 0;
    virtual void stop() = 0;
};

} // namespace ect
