#include "services/AudioService.hpp"

#include <utility>

namespace ect {

AudioService::AudioService(std::shared_ptr<IAudio> audio) : audio_(std::move(audio)) {}

bool AudioService::playAlert(const std::string& label) {
    return audio_ && audio_->playAlert(label);
}

bool AudioService::playMessage(const std::string& text) {
    return audio_ && audio_->playMessage(text);
}

void AudioService::stop() {
    if (audio_) audio_->stop();
}

} // namespace ect
