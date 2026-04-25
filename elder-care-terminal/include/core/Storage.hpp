#pragma once

#include "core/Config.hpp"

#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace ect {

struct Reminder {
    int id{0};
    std::string title;
    std::string type;
    std::string time;
    bool enabled{true};
    bool acknowledged{false};
    std::string lastTriggeredDate;
    std::string alertStartedAt;
};

struct Message {
    int id{0};
    std::string senderName;
    std::string content;
    bool read{false};
    std::string createdAt;
};

struct Contact {
    int id{0};
    std::string name;
    std::string phone;
    std::string relation;
    bool primary{false};
};

class Storage {
public:
    explicit Storage(std::string dataDir);

    bool initialize(const Config& config);
    const std::string& dataDir() const { return dataDir_; }

    std::vector<Reminder> reminders();
    Reminder addReminder(const Reminder& reminder);
    bool acknowledgeReminder(int id);
    bool updateReminderAlert(int id, const std::string& date, const std::string& startedAt);

    std::vector<Message> messages();
    Message addMessage(const Message& message);
    bool markMessageRead(int id);
    int unreadMessageCount();

    std::vector<Contact> contacts();
    Contact addContact(const Contact& contact);
    std::optional<Contact> primaryContact();

private:
    void ensureDataDir();
    void seedFromConfig(const Config& config);
    void loadAll();
    void saveReminders();
    void saveMessages();
    void saveContacts();

    std::string dataDir_;
    std::vector<Reminder> reminders_;
    std::vector<Message> messages_;
    std::vector<Contact> contacts_;
    std::mutex mutex_;
};

std::string escapeJson(const std::string& value);
std::string remindersToJson(const std::vector<Reminder>& reminders);
std::string messagesToJson(const std::vector<Message>& messages);
std::string contactsToJson(const std::vector<Contact>& contacts);

} // namespace ect
