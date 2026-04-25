#pragma once

#include "hardware/IGpio.hpp"

namespace ect {

class MockGpio final : public IGpio {
public:
    bool readButton(const std::string& name) override;
};

} // namespace ect
