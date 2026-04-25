#pragma once

#include "services/models.hpp"

#include <atomic>
#include <filesystem>
#include <functional>
#include <string>
#include <thread>
#include <vector>

namespace ect::web {

class HttpServer {
public:
    using LogFn = std::function<void(const std::string &)>;

    HttpServer(int port, std::filesystem::path static_dir, LogFn logger,
               std::vector<services::Message> &messages, std::vector<services::Contact> &contacts,
               std::vector<services::Reminder> &reminders);
    ~HttpServer();

    bool start();
    void stop();

private:
    void acceptLoop(int server_fd);
    std::string handleRequest(const std::string &request);
    std::string serveStatic(const std::string &path);

    int port_;
    std::filesystem::path static_dir_;
    LogFn logger_;
    std::vector<services::Message> &messages_;
    std::vector<services::Contact> &contacts_;
    std::vector<services::Reminder> &reminders_;

    std::atomic<bool> running_{false};
    int server_fd_{-1};
    std::thread worker_;
};

} // namespace ect::web
