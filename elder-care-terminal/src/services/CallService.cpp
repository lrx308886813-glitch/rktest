#include "services/CallService.hpp"

#include "core/Logger.hpp"

namespace ect {

CallService::CallService(ContactService& contacts, StateMachine& stateMachine)
    : contacts_(contacts), stateMachine_(stateMachine) {}

bool CallService::callPrimary() {
    const auto contact = contacts_.primary();
    if (!contact) {
        Logger::instance().warn("Calling primary contact failed: no primary contact");
        return false;
    }
    stateMachine_.transitionTo(AppState::CALLING, "primary contact call");
    Logger::instance().info("Calling primary contact: " + contact->name + " phone=" + contact->phone);
    stateMachine_.transitionTo(AppState::IDLE, "mock call completed");
    return true;
}

} // namespace ect
