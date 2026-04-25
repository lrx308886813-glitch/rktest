#pragma once

namespace ect {

class ICamera {
public:
    virtual ~ICamera() = default;
    virtual bool detectPresence() = 0;
};

} // namespace ect
