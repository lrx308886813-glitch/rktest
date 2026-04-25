#include "core/state_machine.hpp"

namespace ect::core {

std::string toString(TerminalState state) {
    switch (state) {
    case TerminalState::BOOT:
        return "BOOT";
    case TerminalState::IDLE:
        return "IDLE";
    case TerminalState::REMINDER_ALERT:
        return "REMINDER_ALERT";
    case TerminalState::MESSAGE_ALERT:
        return "MESSAGE_ALERT";
    case TerminalState::CALLING:
        return "CALLING";
    case TerminalState::ERROR:
        return "ERROR";
    case TerminalState::FACTORY_TEST:
        return "FACTORY_TEST";
    }
    return "UNKNOWN";
}

StateMachine::StateMachine(Logger &logger) : logger_(logger) {
    logger_.log("STATE INIT -> BOOT");
}

TerminalState StateMachine::currentState() const { return state_; }

void StateMachine::transitionTo(TerminalState next, const std::string &reason) {
    logger_.log("STATE " + toString(state_) + " -> " + toString(next) + " reason=" + reason);
    state_ = next;
}

} // namespace ect::core
