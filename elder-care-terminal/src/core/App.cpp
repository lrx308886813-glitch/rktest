#include "core/App.hpp"

#include "core/Logger.hpp"
#include "hardware/MockAudio.hpp"
#include "hardware/MockCamera.hpp"
#include "hardware/Rk3566HardwarePlaceholder.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <sstream>
#include <thread>

namespace ect {

namespace {
std::atomic<bool> g_running{true};

void handleSignal(int) {
    g_running = false;
}

std::string okJson(bool ok) {
    return std::string("{\"ok\":") + (ok ? "true" : "false") + "}";
}

std::string reminderJson(const Reminder& reminder) {
    std::ostringstream out;
    out << "{\"id\":" << reminder.id
        << ",\"title\":\"" << escapeJson(reminder.title)
        << "\",\"type\":\"" << escapeJson(reminder.type)
        << "\",\"time\":\"" << escapeJson(reminder.time)
        << "\",\"enabled\":" << (reminder.enabled ? "true" : "false")
        << ",\"acknowledged\":" << (reminder.acknowledged ? "true" : "false") << "}";
    return out.str();
}

std::string messageJson(const Message& message) {
    std::ostringstream out;
    out << "{\"id\":" << message.id
        << ",\"sender_name\":\"" << escapeJson(message.senderName)
        << "\",\"content\":\"" << escapeJson(message.content)
        << "\",\"read\":" << (message.read ? "true" : "false")
        << ",\"created_at\":\"" << escapeJson(message.createdAt) << "\"}";
    return out.str();
}

std::string contactJson(const Contact& contact) {
    std::ostringstream out;
    out << "{\"id\":" << contact.id
        << ",\"name\":\"" << escapeJson(contact.name)
        << "\",\"phone\":\"" << escapeJson(contact.phone)
        << "\",\"relation\":\"" << escapeJson(contact.relation)
        << "\",\"primary\":" << (contact.primary ? "true" : "false") << "}";
    return out.str();
}
} // namespace

App::App() = default;
App::~App() { stop(); }

bool App::initialize() {
    const auto configDir = Config::resolveConfigDir();
    if (!config_.load(configDir)) {
        Logger::instance().error("Failed to load config from " + configDir);
        return false;
    }
    const auto dataPath = dataDir();
    std::filesystem::create_directories(dataPath);
    Logger::instance().setLevel(parseLogLevel(config_.device().logLevel));
    Logger::instance().setLogFile(dataPath + "/event_log.txt");
    Logger::instance().info("Initializing elder-care-terminal");

    storage_ = std::make_unique<Storage>(dataPath);
    storage_->initialize(config_);

    if (!config_.device().mockMode) {
        Rk3566HardwarePlaceholder(configDir + "/device.ini").describeTodo();
    }
    audioHardware_ = std::make_shared<MockAudio>();
    cameraHardware_ = std::make_shared<MockCamera>(config_.device().mockPresence);
    audioService_ = std::make_unique<AudioService>(audioHardware_);
    cameraService_ = std::make_unique<CameraService>(cameraHardware_);
    contactService_ = std::make_unique<ContactService>(*storage_);
    messageService_ = std::make_unique<MessageService>(*storage_, *audioService_, stateMachine_);
    reminderService_ = std::make_unique<ReminderService>(*storage_, *audioService_, stateMachine_);
    callService_ = std::make_unique<CallService>(*contactService_, stateMachine_);
    deviceService_ = std::make_unique<DeviceService>(config_, *cameraService_, stateMachine_);
    httpServer_ = std::make_unique<HttpServer>(config_.device().httpPort, webRoot());
    httpServer_->setHandler([this](const HttpRequest& request) { return route(request); });
    stateMachine_.transitionTo(AppState::IDLE, "application initialized");
    return true;
}

bool App::start() {
    reminderService_->start();
    return httpServer_->start();
}

void App::stop() {
    if (reminderService_) reminderService_->stop();
    if (httpServer_) httpServer_->stop();
}

int App::run() {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);
    if (!initialize() || !start()) {
        stateMachine_.transitionTo(AppState::ERROR, "startup failed");
        return 1;
    }
    Logger::instance().info("Open http://127.0.0.1:" + std::to_string(config_.device().httpPort));
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    stop();
    return 0;
}

HttpResponse App::route(const HttpRequest& request) {
    if (request.method == "GET" && request.path == "/api/health") {
        return {200, "application/json; charset=utf-8", "{\"status\":\"ok\",\"service\":\"elder-care-terminal\"}"};
    }
    if (request.method == "GET" && request.path == "/api/device/status") {
        std::ostringstream out;
        out << "{\"device\":" << deviceService_->statusJson()
            << ",\"unread_messages\":" << messageService_->unreadCount();
        const auto next = reminderService_->nextReminder();
        out << ",\"next_reminder\":" << (next ? reminderJson(*next) : "null") << "}";
        return {200, "application/json; charset=utf-8", out.str()};
    }
    if (request.method == "GET" && request.path == "/api/reminders") {
        return {200, "application/json; charset=utf-8", remindersToJson(reminderService_->list())};
    }
    if (request.method == "POST" && request.path == "/api/reminders") {
        Reminder reminder;
        reminder.title = extractJsonString(request.body, "title");
        reminder.type = extractJsonString(request.body, "type");
        reminder.time = extractJsonString(request.body, "time");
        reminder.enabled = extractJsonBool(request.body, "enabled", true);
        if (reminder.title.empty() || reminder.time.empty()) return {400, "application/json; charset=utf-8", okJson(false)};
        return {201, "application/json; charset=utf-8", reminderJson(reminderService_->add(reminder))};
    }
    if (request.method == "POST" && request.path == "/api/reminders/ack") {
        const auto q = HttpServer::parseQuery(request.query);
        const int id = q.count("id") ? std::stoi(q.at("id")) : 0;
        return {200, "application/json; charset=utf-8", okJson(reminderService_->acknowledge(id))};
    }
    if (request.method == "GET" && request.path == "/api/messages") {
        return {200, "application/json; charset=utf-8", messagesToJson(messageService_->list())};
    }
    if (request.method == "POST" && request.path == "/api/messages") {
        const auto sender = extractJsonString(request.body, "sender_name");
        const auto content = extractJsonString(request.body, "content");
        if (content.empty()) return {400, "application/json; charset=utf-8", okJson(false)};
        return {201, "application/json; charset=utf-8", messageJson(messageService_->add(sender, content))};
    }
    if (request.method == "POST" && request.path == "/api/messages/read") {
        const auto q = HttpServer::parseQuery(request.query);
        const int id = q.count("id") ? std::stoi(q.at("id")) : 0;
        return {200, "application/json; charset=utf-8", okJson(messageService_->markRead(id))};
    }
    if (request.method == "GET" && request.path == "/api/contacts") {
        return {200, "application/json; charset=utf-8", contactsToJson(contactService_->list())};
    }
    if (request.method == "POST" && request.path == "/api/contacts") {
        Contact contact;
        contact.name = extractJsonString(request.body, "name");
        contact.phone = extractJsonString(request.body, "phone");
        contact.relation = extractJsonString(request.body, "relation");
        contact.primary = extractJsonBool(request.body, "primary", false);
        if (contact.name.empty()) return {400, "application/json; charset=utf-8", okJson(false)};
        return {201, "application/json; charset=utf-8", contactJson(contactService_->add(contact))};
    }
    if (request.method == "POST" && request.path == "/api/call/primary") {
        return {200, "application/json; charset=utf-8", okJson(callService_->callPrimary())};
    }
    if (request.method == "POST" && request.path == "/api/messages/play-next") {
        return {200, "application/json; charset=utf-8", okJson(messageService_->playNextUnread())};
    }
    if (request.method == "GET" && request.path == "/api/logs/recent") {
        std::ostringstream out;
        out << "[";
        const auto lines = Logger::instance().recent(80);
        for (std::size_t i = 0; i < lines.size(); ++i) {
            if (i) out << ",";
            out << "\"" << escapeJson(lines[i]) << "\"";
        }
        out << "]";
        return {200, "application/json; charset=utf-8", out.str()};
    }
    return {404, "application/json; charset=utf-8", "{\"error\":\"not found\"}"};
}

std::string App::webRoot() const {
    if (std::filesystem::exists("web/index.html")) return "web";
    if (std::filesystem::exists("../web/index.html")) return "../web";
    return "web";
}

std::string App::dataDir() const {
    if (std::filesystem::exists("../config/device.ini")) return "../data";
    return "data";
}

} // namespace ect
