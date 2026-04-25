#pragma once

#include "core/StateMachine.hpp"
#include "core/Storage.hpp"
#include "services/AudioService.hpp"

#include <atomic>
#include <optional>
#include <thread>

namespace ect {

class ReminderService {
public:
    ReminderService(Storage& storage, AudioService& audio, StateMachine& stateMachine);
    ~ReminderService();

    void start();
    void stop();
    void checkDueOnce();
    std::vector<Reminder> list();
    Reminder add(const Reminder& reminder);
    bool acknowledge(int id);
    std::optional<Reminder> nextReminder();

private:
    void loop();
    void warnUnacknowledged(const Reminder& reminder);

    Storage& storage_;
    AudioService& audio_;
    StateMachine& stateMachine_;
    std::atomic<bool> running_{false};
    std::thread worker_;
};

} // namespace ect
