#pragma once

#include "core/StateMachine.hpp"
#include "services/ContactService.hpp"

namespace ect {

class CallService {
public:
    CallService(ContactService& contacts, StateMachine& stateMachine);
    bool callPrimary();

private:
    ContactService& contacts_;
    StateMachine& stateMachine_;
};

} // namespace ect
