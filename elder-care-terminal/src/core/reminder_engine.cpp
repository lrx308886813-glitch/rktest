#include "core/reminder_engine.hpp"

#include <chrono>

namespace ect::core {

ReminderEngine::ReminderEngine(StateMachine &sm, Logger &logger, services::AudioService &audio, services::DataStore &store,
                               std::vector<services::Reminder> &reminders)
    : state_machine_(sm), logger_(logger), audio_(audio), store_(store), reminders_(reminders) {}

ReminderEngine::~ReminderEngine() { stop(); }

void ReminderEngine::start() {
    if (running_.exchange(true)) {
        return;
    }
    worker_ = std::thread(&ReminderEngine::run, this);
}

void ReminderEngine::stop() {
    if (!running_.exchange(false)) {
        return;
    }
    if (worker_.joinable()) {
        worker_.join();
    }
}

void ReminderEngine::run() {
    while (running_) {
        const auto now = std::chrono::system_clock::now();
        const auto now_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        for (auto &reminder : reminders_) {
            if (!reminder.acknowledged) {
                if (reminder.alert_started_epoch == 0) {
                    reminder.alert_started_epoch = now_epoch;
                    state_machine_.transitionTo(TerminalState::REMINDER_ALERT, "polling_due_reminder:" + reminder.id);
                    audio_.playAlert();
                    store_.saveReminders(reminders_);
                }
                if (now_epoch - reminder.alert_started_epoch >= 180) {
                    logger_.log("REMINDER timeout(3m) id=" + reminder.id + " title=" + reminder.title);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

} // namespace ect::core
