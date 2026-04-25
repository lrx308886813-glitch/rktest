#pragma once

#include <mutex>
#include <string>

namespace ect {

enum class AppState {
    BOOT,
    IDLE,
    REMINDER_ALERT,
    MESSAGE_ALERT,
    CALLING,
    ERROR,
    FACTORY_TEST
};

std::string toString(AppState state);

class StateMachine {
public:
    StateMachine();
    AppState state() const;
    void transitionTo(AppState next, const std::string& reason);

private:
    mutable std::mutex mutex_;
    AppState state_{AppState::BOOT};
};

} // namespace ect
