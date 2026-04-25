#include "core/Config.hpp"

#include <cassert>

int main() {
    ect::Config config;
    assert(config.load(ect::Config::resolveConfigDir()));
    assert(config.device().httpPort == 8080);
    assert(config.device().mockMode);
    assert(!config.reminders().empty());
    assert(!config.contacts().empty());
    return 0;
}
