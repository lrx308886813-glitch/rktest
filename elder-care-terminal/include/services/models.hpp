#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ect::services {

struct Message {
    std::string from;
    std::string text;
    std::string created_at;
};

struct Contact {
    std::string name;
    std::string phone;
};

struct Reminder {
    std::string id;
    std::string title;
    bool acknowledged{false};
    std::int64_t alert_started_epoch{0};
};

struct DataBundle {
    std::vector<Message> messages;
    std::vector<Contact> contacts;
    std::vector<Reminder> reminders;
};

} // namespace ect::services
