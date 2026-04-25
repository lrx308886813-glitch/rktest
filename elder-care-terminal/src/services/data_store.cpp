#include "services/data_store.hpp"

#include <fstream>
#include <sstream>

namespace ect::services {

DataStore::DataStore(std::filesystem::path data_dir) : data_dir_(std::move(data_dir)) {}

void DataStore::ensureDataDir() const {
    std::filesystem::create_directories(data_dir_);
}

std::filesystem::path DataStore::messagesPath() const { return data_dir_ / "messages.db.json"; }
std::filesystem::path DataStore::contactsPath() const { return data_dir_ / "contacts.db.json"; }
std::filesystem::path DataStore::remindersPath() const { return data_dir_ / "reminders.db.json"; }

DataBundle DataStore::load() const {
    ensureDataDir();
    DataBundle bundle;
    if (!std::filesystem::exists(messagesPath())) {
        saveMessages(bundle.messages);
    }
    if (!std::filesystem::exists(contactsPath())) {
        saveContacts(bundle.contacts);
    }
    if (!std::filesystem::exists(remindersPath())) {
        saveReminders(bundle.reminders);
    }
    return bundle;
}

std::string DataStore::escape(const std::string &v) {
    std::string out;
    out.reserve(v.size());
    for (char c : v) {
        if (c == '"') {
            out += "\\\"";
        } else if (c == '\\') {
            out += "\\\\";
        } else {
            out += c;
        }
    }
    return out;
}

void DataStore::saveMessages(const std::vector<Message> &messages) const {
    std::ofstream out(messagesPath());
    out << "[\n";
    for (std::size_t i = 0; i < messages.size(); ++i) {
        const auto &m = messages[i];
        out << "  {\"from\":\"" << escape(m.from) << "\",\"text\":\"" << escape(m.text)
            << "\",\"created_at\":\"" << escape(m.created_at) << "\"}";
        out << (i + 1 < messages.size() ? ",\n" : "\n");
    }
    out << "]\n";
}

void DataStore::saveContacts(const std::vector<Contact> &contacts) const {
    std::ofstream out(contactsPath());
    out << "[\n";
    for (std::size_t i = 0; i < contacts.size(); ++i) {
        const auto &c = contacts[i];
        out << "  {\"name\":\"" << escape(c.name) << "\",\"phone\":\"" << escape(c.phone) << "\"}";
        out << (i + 1 < contacts.size() ? ",\n" : "\n");
    }
    out << "]\n";
}

void DataStore::saveReminders(const std::vector<Reminder> &reminders) const {
    std::ofstream out(remindersPath());
    out << "[\n";
    for (std::size_t i = 0; i < reminders.size(); ++i) {
        const auto &r = reminders[i];
        out << "  {\"id\":\"" << escape(r.id) << "\",\"title\":\"" << escape(r.title)
            << "\",\"acknowledged\":" << (r.acknowledged ? "true" : "false")
            << ",\"alert_started_epoch\":" << r.alert_started_epoch << "}";
        out << (i + 1 < reminders.size() ? ",\n" : "\n");
    }
    out << "]\n";
}

} // namespace ect::services
