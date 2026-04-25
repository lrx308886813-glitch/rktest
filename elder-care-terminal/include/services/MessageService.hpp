#pragma once

#include "core/StateMachine.hpp"
#include "core/Storage.hpp"
#include "services/AudioService.hpp"

namespace ect {

class MessageService {
public:
    MessageService(Storage& storage, AudioService& audio, StateMachine& stateMachine);
    std::vector<Message> list();
    Message add(const std::string& senderName, const std::string& content);
    bool markRead(int id);
    int unreadCount();
    bool playNextUnread();

private:
    Storage& storage_;
    AudioService& audio_;
    StateMachine& stateMachine_;
};

} // namespace ect
