#include "services/ReminderService.hpp"

#include "core/Logger.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace ect {

namespace {
std::string today() {
    const std::string now = nowIsoString();
    return now.size() >= 10 ? now.substr(0, 10) : "";
}

std::string currentMinute() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%H:%M");
    return out.str();
}

long secondsBetweenNowAnd(const std::string& startedAt) {
    if (startedAt.empty()) return 0;
    std::tm tm{};
    std::istringstream in(startedAt);
    in >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (in.fail()) return 0;
    const auto started = std::mktime(&tm);
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return static_cast<long>(std::difftime(now, started));
}
} // namespace

ReminderService::ReminderService(Storage& storage, AudioService& audio, StateMachine& stateMachine)
    : storage_(storage), audio_(audio), stateMachine_(stateMachine) {}

ReminderService::~ReminderService() {
    stop();
}

void ReminderService::start() {
    if (running_) return;
    running_ = true;
    worker_ = std::thread(&ReminderService::loop, this);
}

void ReminderService::stop() {
    running_ = false;
    if (worker_.joinable()) worker_.join();
}

void ReminderService::loop() {
    while (running_) {
        checkDueOnce();
        for (int i = 0; i < 10 && running_; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void ReminderService::checkDueOnce() {
    const auto date = today();
    const auto minute = currentMinute();
    for (const auto& reminder : storage_.reminders()) {
        if (!reminder.enabled) continue;
        if (!reminder.acknowledged && !reminder.alertStartedAt.empty() && secondsBetweenNowAnd(reminder.alertStartedAt) >= 180) {
            warnUnacknowledged(reminder);
        }
        if (reminder.time == minute && reminder.lastTriggeredDate != date) {
            Logger::instance().info("Reminder due: " + reminder.title);
            storage_.updateReminderAlert(reminder.id, date, nowIsoString());
            stateMachine_.transitionTo(AppState::REMINDER_ALERT, "reminder due");
            audio_.playAlert(reminder.title);
        }
    }
}

std::vector<Reminder> ReminderService::list() {
    return storage_.reminders();
}

Reminder ReminderService::add(const Reminder& reminder) {
    return storage_.addReminder(reminder);
}

bool ReminderService::acknowledge(int id) {
    const bool ok = storage_.acknowledgeReminder(id);
    if (ok) {
        audio_.stop();
        stateMachine_.transitionTo(AppState::IDLE, "reminder acknowledged");
    }
    return ok;
}

std::optional<Reminder> ReminderService::nextReminder() {
    auto reminders = storage_.reminders();
    if (reminders.empty()) return std::nullopt;
    std::sort(reminders.begin(), reminders.end(), [](const Reminder& a, const Reminder& b) {
        return a.time < b.time;
    });
    return reminders.front();
}

void ReminderService::warnUnacknowledged(const Reminder& reminder) {
    Logger::instance().warn("Reminder unacknowledged for 3 minutes: id=" + std::to_string(reminder.id) + " title=" + reminder.title);
}

} // namespace ect
