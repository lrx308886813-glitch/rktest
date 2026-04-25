#include "hardware/mock_hardware.hpp"

#include <fstream>

namespace ect::hardware {

bool MockAudio::playTone(const std::string &) { return true; }

bool MockCamera::captureFrame(const std::string &target_file) {
    std::ofstream out(target_file);
    out << "mock_frame";
    return true;
}

bool MockGpio::writePin(int, bool) { return true; }

bool MockGpio::readPin(int) { return false; }

} // namespace ect::hardware
