#pragma once

#include "core/logger.hpp"

#include <string>

namespace ect::core {

enum class TerminalState {
    BOOT,
    IDLE,
    REMINDER_ALERT,
    MESSAGE_ALERT,
    CALLING,
    ERROR,
    FACTORY_TEST
};

std::string toString(TerminalState state);

class StateMachine {
public:
    explicit StateMachine(Logger &logger);

    TerminalState currentState() const;
    void transitionTo(TerminalState next, const std::string &reason);

private:
    TerminalState state_{TerminalState::BOOT};
    Logger &logger_;
};

} // namespace ect::core
