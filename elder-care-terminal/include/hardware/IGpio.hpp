#pragma once

#include <string>

namespace ect {

class IGpio {
public:
    virtual ~IGpio() = default;
    virtual bool readButton(const std::string& name) = 0;
};

} // namespace ect
