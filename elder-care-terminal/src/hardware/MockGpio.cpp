#include "hardware/MockGpio.hpp"

#include "core/Logger.hpp"

namespace ect {

bool MockGpio::readButton(const std::string& name) {
    Logger::instance().debug("MockGpio readButton: " + name + "=false");
    return false;
}

} // namespace ect
