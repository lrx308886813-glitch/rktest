#pragma once

#include "services/models.hpp"

#include <filesystem>
#include <string>

namespace ect::services {

class DataStore {
public:
    explicit DataStore(std::filesystem::path data_dir);

    void ensureDataDir() const;
    DataBundle load() const;
    void saveMessages(const std::vector<Message> &messages) const;
    void saveContacts(const std::vector<Contact> &contacts) const;
    void saveReminders(const std::vector<Reminder> &reminders) const;

    std::filesystem::path messagesPath() const;
    std::filesystem::path contactsPath() const;
    std::filesystem::path remindersPath() const;

private:
    std::filesystem::path data_dir_;

    static std::string escape(const std::string &v);
};

} // namespace ect::services
