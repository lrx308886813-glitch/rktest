#include "web/http_server.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

namespace ect::web {

namespace {
std::string ok(const std::string &body, const std::string &content_type = "text/html; charset=utf-8") {
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: " << content_type << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return oss.str();
}

std::string notFound() {
    const std::string body = R"({"error":"not found"})";
    return "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(body.size()) +
           "\r\nConnection: close\r\n\r\n" + body;
}
} // namespace

HttpServer::HttpServer(int port, std::filesystem::path static_dir, LogFn logger,
                       std::vector<services::Message> &messages, std::vector<services::Contact> &contacts,
                       std::vector<services::Reminder> &reminders)
    : port_(port), static_dir_(std::move(static_dir)), logger_(std::move(logger)), messages_(messages), contacts_(contacts),
      reminders_(reminders) {}

HttpServer::~HttpServer() { stop(); }

bool HttpServer::start() {
    server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        return false;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        return false;
    }
    if (listen(server_fd_, 16) < 0) {
        return false;
    }
    running_ = true;
    worker_ = std::thread(&HttpServer::acceptLoop, this, server_fd_);
    logger_("HTTP server started on port " + std::to_string(port_));
    return true;
}

void HttpServer::stop() {
    if (!running_.exchange(false)) {
        return;
    }
    if (server_fd_ >= 0) {
        shutdown(server_fd_, SHUT_RDWR);
        close(server_fd_);
        server_fd_ = -1;
    }
    if (worker_.joinable()) {
        worker_.join();
    }
}

void HttpServer::acceptLoop(int server_fd) {
    while (running_) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        const int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
        if (client_fd < 0) {
            continue;
        }
        char buffer[4096] = {0};
        const auto n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            std::string response = handleRequest(std::string(buffer, static_cast<std::size_t>(n)));
            send(client_fd, response.data(), response.size(), 0);
        }
        close(client_fd);
    }
}

std::string HttpServer::serveStatic(const std::string &path) {
    const auto safe = path == "/" ? std::string("/index.html") : path;
    auto resolved = static_dir_ / safe.substr(1);
    if (resolved.extension() == ".css") {
        std::ifstream in(resolved);
        std::stringstream buf;
        buf << in.rdbuf();
        return ok(buf.str(), "text/css; charset=utf-8");
    }
    if (resolved.extension() == ".js") {
        std::ifstream in(resolved);
        std::stringstream buf;
        buf << in.rdbuf();
        return ok(buf.str(), "application/javascript; charset=utf-8");
    }
    if (resolved.extension() == ".html") {
        std::ifstream in(resolved);
        std::stringstream buf;
        buf << in.rdbuf();
        return ok(buf.str());
    }
    return notFound();
}

std::string HttpServer::handleRequest(const std::string &request) {
    std::istringstream iss(request);
    std::string method;
    std::string path;
    iss >> method >> path;

    if (method != "GET") {
        return notFound();
    }

    if (path.rfind("/api/", 0) == 0) {
        if (path == "/api/health") {
            return ok(R"({"status":"ok"})", "application/json");
        }
        if (path == "/api/messages") {
            return ok("{\"count\":" + std::to_string(messages_.size()) + "}", "application/json");
        }
        if (path == "/api/contacts") {
            return ok("{\"count\":" + std::to_string(contacts_.size()) + "}", "application/json");
        }
        if (path == "/api/reminders") {
            return ok("{\"count\":" + std::to_string(reminders_.size()) + "}", "application/json");
        }
        return ok(R"({"error":"unknown api"})", "application/json");
    }

    if (path == "/") {
        return serveStatic("/index.html");
    }
    if (path == "/admin") {
        return serveStatic("/admin.html");
    }
    return serveStatic(path);
}

} // namespace ect::web
