#pragma once

#include <string>

namespace ect::hardware {

class IAudio {
public:
    virtual ~IAudio() = default;
    virtual bool playTone(const std::string &tone_name) = 0;
};

class ICamera {
public:
    virtual ~ICamera() = default;
    virtual bool captureFrame(const std::string &target_file) = 0;
};

class IGpio {
public:
    virtual ~IGpio() = default;
    virtual bool writePin(int pin, bool value) = 0;
    virtual bool readPin(int pin) = 0;
};

} // namespace ect::hardware
