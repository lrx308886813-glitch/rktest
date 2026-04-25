#include "core/logger.hpp"
#include "core/reminder_engine.hpp"
#include "core/state_machine.hpp"
#include "hardware/mock_hardware.hpp"
#include "services/audio_service.hpp"
#include "services/data_store.hpp"
#include "web/http_server.hpp"

#include <atomic>
#include <csignal>
#include <filesystem>
#include <thread>

namespace {
std::atomic<bool> g_running{true};
void handleSignal(int) { g_running = false; }
} // namespace

int main() {
    const std::filesystem::path root = std::filesystem::current_path();
    const std::filesystem::path app_dir = root / "elder-care-terminal";
    const std::filesystem::path data_dir = app_dir / "data";
    const std::filesystem::path log_file = data_dir / "event_log.txt";

    ect::core::Logger logger(log_file.string());
    ect::services::DataStore store(data_dir);
    auto bundle = store.load();

    ect::hardware::MockAudio audio_hw;
    ect::services::AudioService audio_service(audio_hw);
    ect::core::StateMachine state_machine(logger);
    state_machine.transitionTo(ect::core::TerminalState::IDLE, "boot_complete");

    ect::core::ReminderEngine reminder_engine(state_machine, logger, audio_service, store, bundle.reminders);
    reminder_engine.start();

    ect::web::HttpServer server(8080, app_dir / "web", [&](const std::string &m) { logger.log(m); }, bundle.messages,
                                bundle.contacts, bundle.reminders);
    if (!server.start()) {
        logger.log("HTTP start failed");
        return 1;
    }

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    reminder_engine.stop();
    server.stop();
    store.saveMessages(bundle.messages);
    store.saveContacts(bundle.contacts);
    store.saveReminders(bundle.reminders);

    return 0;
}
