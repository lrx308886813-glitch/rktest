#include "core/Storage.hpp"

#include "core/Logger.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

namespace ect {

namespace {
std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::trunc);
    out << content;
}

int nextIdReminder(const std::vector<Reminder>& items) {
    int id = 0;
    for (const auto& item : items) id = std::max(id, item.id);
    return id + 1;
}

int nextIdMessage(const std::vector<Message>& items) {
    int id = 0;
    for (const auto& item : items) id = std::max(id, item.id);
    return id + 1;
}

int nextIdContact(const std::vector<Contact>& items) {
    int id = 0;
    for (const auto& item : items) id = std::max(id, item.id);
    return id + 1;
}

std::string jsonStringField(const std::string& object, const std::string& key) {
    const std::string needle = "\"" + key + "\":\"";
    const auto start = object.find(needle);
    if (start == std::string::npos) return "";
    const auto valueStart = start + needle.size();
    std::string result;
    bool escape = false;
    for (std::size_t i = valueStart; i < object.size(); ++i) {
        const char c = object[i];
        if (escape) {
            result.push_back(c);
            escape = false;
        } else if (c == '\\') {
            escape = true;
        } else if (c == '"') {
            break;
        } else {
            result.push_back(c);
        }
    }
    return result;
}

int jsonIntField(const std::string& object, const std::string& key) {
    const std::string needle = "\"" + key + "\":";
    const auto start = object.find(needle);
    if (start == std::string::npos) return 0;
    try {
        return std::stoi(object.substr(start + needle.size()));
    } catch (...) {
        return 0;
    }
}

bool jsonBoolField(const std::string& object, const std::string& key) {
    const std::string needle = "\"" + key + "\":";
    const auto start = object.find(needle);
    if (start == std::string::npos) return false;
    return object.substr(start + needle.size(), 4) == "true";
}

std::vector<std::string> objectsInArray(const std::string& json) {
    std::vector<std::string> objects;
    std::size_t pos = 0;
    while ((pos = json.find('{', pos)) != std::string::npos) {
        int depth = 0;
        bool inString = false;
        bool escape = false;
        for (std::size_t i = pos; i < json.size(); ++i) {
            const char c = json[i];
            if (escape) {
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else if (c == '"') {
                inString = !inString;
            } else if (!inString && c == '{') {
                ++depth;
            } else if (!inString && c == '}') {
                --depth;
                if (depth == 0) {
                    objects.push_back(json.substr(pos, i - pos + 1));
                    pos = i + 1;
                    break;
                }
            }
        }
    }
    return objects;
}

std::string datePart(const std::string& iso) {
    return iso.size() >= 10 ? iso.substr(0, 10) : "";
}
} // namespace

Storage::Storage(std::string dataDir) : dataDir_(std::move(dataDir)) {}

bool Storage::initialize(const Config& config) {
    ensureDataDir();
    loadAll();
    seedFromConfig(config);
    return true;
}

void Storage::ensureDataDir() {
    std::filesystem::create_directories(dataDir_);
}

void Storage::seedFromConfig(const Config& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (reminders_.empty()) {
        int id = 1;
        for (const auto& cfg : config.reminders()) {
            reminders_.push_back(Reminder{id++, cfg.title, cfg.type, cfg.time, cfg.enabled, false, "", ""});
        }
        saveReminders();
    }
    if (contacts_.empty()) {
        int id = 1;
        for (const auto& cfg : config.contacts()) {
            contacts_.push_back(Contact{id++, cfg.name, cfg.phone, cfg.relation, cfg.primary});
        }
        saveContacts();
    }
}

void Storage::loadAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    reminders_.clear();
    for (const auto& object : objectsInArray(readFile(dataDir_ + "/reminders.db.json"))) {
        reminders_.push_back(Reminder{
            jsonIntField(object, "id"),
            jsonStringField(object, "title"),
            jsonStringField(object, "type"),
            jsonStringField(object, "time"),
            jsonBoolField(object, "enabled"),
            jsonBoolField(object, "acknowledged"),
            jsonStringField(object, "last_triggered_date"),
            jsonStringField(object, "alert_started_at")
        });
    }
    messages_.clear();
    for (const auto& object : objectsInArray(readFile(dataDir_ + "/messages.db.json"))) {
        messages_.push_back(Message{
            jsonIntField(object, "id"),
            jsonStringField(object, "sender_name"),
            jsonStringField(object, "content"),
            jsonBoolField(object, "read"),
            jsonStringField(object, "created_at")
        });
    }
    contacts_.clear();
    for (const auto& object : objectsInArray(readFile(dataDir_ + "/contacts.db.json"))) {
        contacts_.push_back(Contact{
            jsonIntField(object, "id"),
            jsonStringField(object, "name"),
            jsonStringField(object, "phone"),
            jsonStringField(object, "relation"),
            jsonBoolField(object, "primary")
        });
    }
}

std::vector<Reminder> Storage::reminders() {
    std::lock_guard<std::mutex> lock(mutex_);
    return reminders_;
}

Reminder Storage::addReminder(const Reminder& reminder) {
    std::lock_guard<std::mutex> lock(mutex_);
    Reminder copy = reminder;
    copy.id = nextIdReminder(reminders_);
    reminders_.push_back(copy);
    saveReminders();
    return copy;
}

bool Storage::acknowledgeReminder(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& reminder : reminders_) {
        if (reminder.id == id) {
            reminder.acknowledged = true;
            reminder.alertStartedAt.clear();
            saveReminders();
            Logger::instance().info("Reminder acknowledged: id=" + std::to_string(id));
            return true;
        }
    }
    return false;
}

bool Storage::updateReminderAlert(int id, const std::string& date, const std::string& startedAt) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& reminder : reminders_) {
        if (reminder.id == id) {
            reminder.lastTriggeredDate = date;
            reminder.alertStartedAt = startedAt;
            reminder.acknowledged = false;
            saveReminders();
            return true;
        }
    }
    return false;
}

std::vector<Message> Storage::messages() {
    std::lock_guard<std::mutex> lock(mutex_);
    return messages_;
}

Message Storage::addMessage(const Message& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    Message copy = message;
    copy.id = nextIdMessage(messages_);
    if (copy.createdAt.empty()) copy.createdAt = nowIsoString();
    messages_.push_back(copy);
    saveMessages();
    return copy;
}

bool Storage::markMessageRead(int id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& message : messages_) {
        if (message.id == id) {
            message.read = true;
            saveMessages();
            return true;
        }
    }
    return false;
}

int Storage::unreadMessageCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int>(std::count_if(messages_.begin(), messages_.end(), [](const Message& m) { return !m.read; }));
}

std::vector<Contact> Storage::contacts() {
    std::lock_guard<std::mutex> lock(mutex_);
    return contacts_;
}

Contact Storage::addContact(const Contact& contact) {
    std::lock_guard<std::mutex> lock(mutex_);
    Contact copy = contact;
    copy.id = nextIdContact(contacts_);
    contacts_.push_back(copy);
    saveContacts();
    return copy;
}

std::optional<Contact> Storage::primaryContact() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(contacts_.begin(), contacts_.end(), [](const Contact& c) { return c.primary; });
    if (it == contacts_.end()) return std::nullopt;
    return *it;
}

void Storage::saveReminders() { writeFile(dataDir_ + "/reminders.db.json", remindersToJson(reminders_)); }
void Storage::saveMessages() { writeFile(dataDir_ + "/messages.db.json", messagesToJson(messages_)); }
void Storage::saveContacts() { writeFile(dataDir_ + "/contacts.db.json", contactsToJson(contacts_)); }

std::string escapeJson(const std::string& value) {
    std::string out;
    for (char c : value) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out.push_back(c);
    }
    return out;
}

std::string remindersToJson(const std::vector<Reminder>& reminders) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < reminders.size(); ++i) {
        const auto& r = reminders[i];
        if (i) out << ",";
        out << "{\"id\":" << r.id
            << ",\"title\":\"" << escapeJson(r.title)
            << "\",\"type\":\"" << escapeJson(r.type)
            << "\",\"time\":\"" << escapeJson(r.time)
            << "\",\"enabled\":" << (r.enabled ? "true" : "false")
            << ",\"acknowledged\":" << (r.acknowledged ? "true" : "false")
            << ",\"last_triggered_date\":\"" << escapeJson(r.lastTriggeredDate)
            << "\",\"alert_started_at\":\"" << escapeJson(r.alertStartedAt) << "\"}";
    }
    out << "]";
    return out.str();
}

std::string messagesToJson(const std::vector<Message>& messages) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < messages.size(); ++i) {
        const auto& m = messages[i];
        if (i) out << ",";
        out << "{\"id\":" << m.id
            << ",\"sender_name\":\"" << escapeJson(m.senderName)
            << "\",\"content\":\"" << escapeJson(m.content)
            << "\",\"read\":" << (m.read ? "true" : "false")
            << ",\"created_at\":\"" << escapeJson(m.createdAt) << "\"}";
    }
    out << "]";
    return out.str();
}

std::string contactsToJson(const std::vector<Contact>& contacts) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < contacts.size(); ++i) {
        const auto& c = contacts[i];
        if (i) out << ",";
        out << "{\"id\":" << c.id
            << ",\"name\":\"" << escapeJson(c.name)
            << "\",\"phone\":\"" << escapeJson(c.phone)
            << "\",\"relation\":\"" << escapeJson(c.relation)
            << "\",\"primary\":" << (c.primary ? "true" : "false") << "}";
    }
    out << "]";
    return out.str();
}

} // namespace ect
