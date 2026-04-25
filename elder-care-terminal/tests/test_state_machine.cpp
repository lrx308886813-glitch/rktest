#include "core/StateMachine.hpp"

#include <cassert>

int main() {
    ect::StateMachine sm;
    assert(sm.state() == ect::AppState::BOOT);
    sm.transitionTo(ect::AppState::IDLE, "test");
    assert(sm.state() == ect::AppState::IDLE);
    sm.transitionTo(ect::AppState::REMINDER_ALERT, "test");
    assert(ect::toString(sm.state()) == "REMINDER_ALERT");
    return 0;
}
