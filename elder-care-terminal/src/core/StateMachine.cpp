#include "core/StateMachine.hpp"

#include "core/Logger.hpp"

namespace ect {

std::string toString(AppState state) {
    switch (state) {
    case AppState::BOOT: return "BOOT";
    case AppState::IDLE: return "IDLE";
    case AppState::REMINDER_ALERT: return "REMINDER_ALERT";
    case AppState::MESSAGE_ALERT: return "MESSAGE_ALERT";
    case AppState::CALLING: return "CALLING";
    case AppState::ERROR: return "ERROR";
    case AppState::FACTORY_TEST: return "FACTORY_TEST";
    }
    return "UNKNOWN";
}

StateMachine::StateMachine() {
    Logger::instance().info("State initialized: BOOT");
}

AppState StateMachine::state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

void StateMachine::transitionTo(AppState next, const std::string& reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ == next) return;
    Logger::instance().info("State change: " + toString(state_) + " -> " + toString(next) + " (" + reason + ")");
    state_ = next;
}

} // namespace ect
