#include "services/audio_service.hpp"

namespace ect::services {

AudioService::AudioService(hardware::IAudio &audio) : audio_(audio) {}

bool AudioService::playAlert() { return audio_.playTone("reminder_alert"); }

} // namespace ect::services
