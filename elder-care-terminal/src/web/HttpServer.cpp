#include "web/HttpServer.hpp"

#include "core/Logger.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
using socklen_t = int;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace ect {

namespace {
void closeSocket(int socketFd) {
#if defined(_WIN32)
    closesocket(socketFd);
#else
    close(socketFd);
#endif
}

std::string reasonPhrase(int status) {
    if (status == 200) return "OK";
    if (status == 201) return "Created";
    if (status == 400) return "Bad Request";
    if (status == 404) return "Not Found";
    if (status == 500) return "Internal Server Error";
    return "OK";
}

std::string contentTypeFor(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html; charset=utf-8";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") return "text/css; charset=utf-8";
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") return "application/javascript; charset=utf-8";
    return "text/plain; charset=utf-8";
}

std::string readTextFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void sendAll(int socketFd, const std::string& data) {
    const char* buffer = data.data();
    std::size_t remaining = data.size();
    while (remaining > 0) {
#if defined(_WIN32)
        const int sent = send(socketFd, buffer, static_cast<int>(remaining), 0);
#else
        const auto sent = send(socketFd, buffer, remaining, 0);
#endif
        if (sent <= 0) return;
        buffer += sent;
        remaining -= static_cast<std::size_t>(sent);
    }
}

std::string trim(const std::string& value) {
    auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
    auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return begin >= end ? "" : std::string(begin, end);
}
} // namespace

HttpServer::HttpServer(int port, std::string webRoot) : port_(port), webRoot_(std::move(webRoot)) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::setHandler(Handler handler) {
    handler_ = std::move(handler);
}

bool HttpServer::start() {
    if (running_) return true;
#if defined(_WIN32)
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        Logger::instance().error("WSAStartup failed");
        return false;
    }
#endif
    serverSocket_ = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
    if (serverSocket_ < 0) {
        Logger::instance().error("Failed to create HTTP socket");
        return false;
    }
    int reuse = 1;
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<uint16_t>(port_));
    if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        Logger::instance().error("Failed to bind HTTP port " + std::to_string(port_));
        closeSocket(serverSocket_);
        serverSocket_ = -1;
        return false;
    }
    if (listen(serverSocket_, 16) < 0) {
        Logger::instance().error("Failed to listen HTTP port " + std::to_string(port_));
        closeSocket(serverSocket_);
        serverSocket_ = -1;
        return false;
    }
    running_ = true;
    worker_ = std::thread(&HttpServer::loop, this);
    Logger::instance().info("HTTP server listening on port " + std::to_string(port_));
    return true;
}

void HttpServer::stop() {
    running_ = false;
    if (serverSocket_ >= 0) {
        closeSocket(serverSocket_);
        serverSocket_ = -1;
    }
    if (worker_.joinable()) worker_.join();
#if defined(_WIN32)
    WSACleanup();
#endif
}

void HttpServer::loop() {
    while (running_) {
        sockaddr_in clientAddress{};
        socklen_t clientLength = sizeof(clientAddress);
        const int client = static_cast<int>(accept(serverSocket_, reinterpret_cast<sockaddr*>(&clientAddress), &clientLength));
        if (client < 0) {
            if (running_) Logger::instance().warn("HTTP accept failed");
            continue;
        }
        std::thread(&HttpServer::handleClient, this, client).detach();
    }
}

void HttpServer::handleClient(int clientSocket) {
    std::string raw;
    std::vector<char> buffer(4096);
    int contentLength = 0;
    while (true) {
#if defined(_WIN32)
        const int received = recv(clientSocket, buffer.data(), static_cast<int>(buffer.size()), 0);
#else
        const auto received = recv(clientSocket, buffer.data(), buffer.size(), 0);
#endif
        if (received <= 0) break;
        raw.append(buffer.data(), static_cast<std::size_t>(received));
        const auto headerEnd = raw.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            const auto header = raw.substr(0, headerEnd);
            const auto contentPos = header.find("Content-Length:");
            if (contentPos != std::string::npos) {
                contentLength = std::stoi(trim(header.substr(contentPos + 15)));
            }
            if (raw.size() >= headerEnd + 4 + static_cast<std::size_t>(contentLength)) break;
        }
        if (raw.size() > 1024 * 1024) break;
    }

    HttpRequest request;
    std::istringstream stream(raw);
    std::string firstLine;
    std::getline(stream, firstLine);
    if (!firstLine.empty() && firstLine.back() == '\r') firstLine.pop_back();
    std::istringstream first(firstLine);
    std::string target;
    first >> request.method >> target;
    const auto queryPos = target.find('?');
    request.path = queryPos == std::string::npos ? target : target.substr(0, queryPos);
    request.query = queryPos == std::string::npos ? "" : target.substr(queryPos + 1);

    std::string line;
    while (std::getline(stream, line) && line != "\r") {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const auto colon = line.find(':');
        if (colon != std::string::npos) {
            request.headers[trim(line.substr(0, colon))] = trim(line.substr(colon + 1));
        }
    }
    const auto bodyStart = raw.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        request.body = raw.substr(bodyStart + 4);
    }

    HttpResponse response;
    if (handler_ && request.path.rfind("/api/", 0) == 0) {
        response = handler_(request);
    } else {
        response = serveStatic(request);
    }

    std::ostringstream out;
    out << "HTTP/1.1 " << response.status << " " << reasonPhrase(response.status) << "\r\n"
        << "Content-Type: " << response.contentType << "\r\n"
        << "Content-Length: " << response.body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << response.body;
    sendAll(clientSocket, out.str());
    closeSocket(clientSocket);
}

HttpResponse HttpServer::serveStatic(const HttpRequest& request) {
    std::string path = request.path;
    if (path == "/" || path.empty()) path = "/index.html";
    if (path == "/admin") path = "/admin.html";
    if (path.find("..") != std::string::npos) {
        return {400, "text/plain; charset=utf-8", "bad request"};
    }
    const std::string fullPath = webRoot_ + path;
    if (!std::filesystem::exists(fullPath)) {
        return {404, "text/plain; charset=utf-8", "not found"};
    }
    return {200, contentTypeFor(fullPath), readTextFile(fullPath)};
}

std::map<std::string, std::string> HttpServer::parseQuery(const std::string& query) {
    std::map<std::string, std::string> result;
    std::istringstream stream(query);
    std::string part;
    while (std::getline(stream, part, '&')) {
        const auto eq = part.find('=');
        if (eq != std::string::npos) {
            result[urlDecode(part.substr(0, eq))] = urlDecode(part.substr(eq + 1));
        }
    }
    return result;
}

std::string urlDecode(const std::string& value) {
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%' && i + 2 < value.size()) {
            const std::string hex = value.substr(i + 1, 2);
            result.push_back(static_cast<char>(std::stoi(hex, nullptr, 16)));
            i += 2;
        } else if (value[i] == '+') {
            result.push_back(' ');
        } else {
            result.push_back(value[i]);
        }
    }
    return result;
}

std::string extractJsonString(const std::string& body, const std::string& key) {
    const std::string jsonNeedle = "\"" + key + "\"";
    auto pos = body.find(jsonNeedle);
    if (pos != std::string::npos) {
        pos = body.find(':', pos);
        pos = body.find('"', pos);
        if (pos != std::string::npos) {
            std::string result;
            bool escape = false;
            for (std::size_t i = pos + 1; i < body.size(); ++i) {
                if (escape) {
                    result.push_back(body[i]);
                    escape = false;
                } else if (body[i] == '\\') {
                    escape = true;
                } else if (body[i] == '"') {
                    return result;
                } else {
                    result.push_back(body[i]);
                }
            }
        }
    }
    const auto formPos = body.find(key + "=");
    if (formPos != std::string::npos) {
        const auto start = formPos + key.size() + 1;
        const auto end = body.find('&', start);
        return urlDecode(body.substr(start, end == std::string::npos ? end : end - start));
    }
    return "";
}

bool extractJsonBool(const std::string& body, const std::string& key, bool defaultValue) {
    const std::string needle = "\"" + key + "\"";
    auto pos = body.find(needle);
    if (pos == std::string::npos) return defaultValue;
    pos = body.find(':', pos);
    if (pos == std::string::npos) return defaultValue;
    const std::string tail = body.substr(pos + 1);
    if (tail.find("true") == 0) return true;
    if (tail.find("false") == 0) return false;
    return defaultValue;
}

} // namespace ect
