#include "core/Config.hpp"
#include "core/StateMachine.hpp"
#include "core/Storage.hpp"
#include "hardware/MockAudio.hpp"
#include "services/AudioService.hpp"
#include "services/ReminderService.hpp"

#include <cassert>
#include <filesystem>
#include <memory>

int main() {
    ect::Config config;
    assert(config.load(ect::Config::resolveConfigDir()));
    const std::string dataDir = "test_data_reminder";
    std::filesystem::remove_all(dataDir);

    ect::Storage storage(dataDir);
    assert(storage.initialize(config));
    auto audioHw = std::make_shared<ect::MockAudio>();
    ect::AudioService audio(audioHw);
    ect::StateMachine sm;
    ect::ReminderService service(storage, audio, sm);

    ect::Reminder reminder;
    reminder.title = "test reminder";
    reminder.type = "CUSTOM";
    reminder.time = "23:59";
    reminder.enabled = true;
    auto saved = service.add(reminder);
    assert(saved.id > 0);
    assert(service.acknowledge(saved.id));
    auto reminders = service.list();
    bool found = false;
    for (const auto& item : reminders) {
        if (item.id == saved.id) {
            found = item.acknowledged;
        }
    }
    assert(found);
    std::filesystem::remove_all(dataDir);
    return 0;
}
