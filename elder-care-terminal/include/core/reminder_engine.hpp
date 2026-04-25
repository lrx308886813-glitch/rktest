#pragma once

#include "core/state_machine.hpp"
#include "services/audio_service.hpp"
#include "services/data_store.hpp"

#include <atomic>
#include <thread>
#include <vector>

namespace ect::core {

class ReminderEngine {
public:
    ReminderEngine(StateMachine &sm, Logger &logger, services::AudioService &audio, services::DataStore &store,
                   std::vector<services::Reminder> &reminders);
    ~ReminderEngine();

    void start();
    void stop();

private:
    void run();

    StateMachine &state_machine_;
    Logger &logger_;
    services::AudioService &audio_;
    services::DataStore &store_;
    std::vector<services::Reminder> &reminders_;
    std::atomic<bool> running_{false};
    std::thread worker_;
};

} // namespace ect::core
