#include "services/MessageService.hpp"

#include "core/Logger.hpp"

namespace ect {

MessageService::MessageService(Storage& storage, AudioService& audio, StateMachine& stateMachine)
    : storage_(storage), audio_(audio), stateMachine_(stateMachine) {}

std::vector<Message> MessageService::list() {
    return storage_.messages();
}

Message MessageService::add(const std::string& senderName, const std::string& content) {
    Message message;
    message.senderName = senderName.empty() ? "family" : senderName;
    message.content = content;
    message.read = false;
    message.createdAt = nowIsoString();
    auto saved = storage_.addMessage(message);
    stateMachine_.transitionTo(AppState::MESSAGE_ALERT, "new message");
    Logger::instance().info("Message added: id=" + std::to_string(saved.id));
    return saved;
}

bool MessageService::markRead(int id) {
    const bool ok = storage_.markMessageRead(id);
    if (ok && storage_.unreadMessageCount() == 0) {
        stateMachine_.transitionTo(AppState::IDLE, "all messages read");
    }
    return ok;
}

int MessageService::unreadCount() {
    return storage_.unreadMessageCount();
}

bool MessageService::playNextUnread() {
    for (const auto& message : storage_.messages()) {
        if (!message.read) {
            audio_.playMessage(message.content);
            storage_.markMessageRead(message.id);
            return true;
        }
    }
    Logger::instance().info("No unread message to play");
    return false;
}

} // namespace ect
