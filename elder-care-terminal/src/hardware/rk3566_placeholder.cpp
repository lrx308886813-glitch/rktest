#include "hardware/rk3566_placeholder.hpp"

namespace ect::hardware {

bool Rk3566HardwarePlaceholder::playTone(const std::string &) {
    // TODO(RK3566): Integrate ALSA/aplay playback pipeline.
    return true;
}

bool Rk3566HardwarePlaceholder::captureFrame(const std::string &) {
    // TODO(RK3566): Integrate V4L2 + OpenCV camera capture.
    return true;
}

bool Rk3566HardwarePlaceholder::writePin(int, bool) {
    // TODO(RK3566): Integrate real GPIO controller abstraction.
    return true;
}

bool Rk3566HardwarePlaceholder::readPin(int) {
    // TODO(RK3566): Integrate real GPIO controller abstraction.
    return false;
}

std::string Rk3566HardwarePlaceholder::extensionNotes() const {
    return "TODO: add 4G modem, SIP stack, and WebRTC media pipeline integration points.";
}

} // namespace ect::hardware
