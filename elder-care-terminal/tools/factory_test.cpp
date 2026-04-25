#include "core/Config.hpp"
#include "core/Logger.hpp"
#include "core/StateMachine.hpp"
#include "hardware/MockAudio.hpp"
#include "hardware/MockCamera.hpp"
#include "web/HttpServer.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {
void closeSocket(int socketFd) {
#if defined(_WIN32)
    closesocket(socketFd);
#else
    close(socketFd);
#endif
}

bool httpHealthCheck(int port) {
#if defined(_WIN32)
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return false;
#endif
    const int sock = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
    if (sock < 0) return false;
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<uint16_t>(port));
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        closeSocket(sock);
        return false;
    }
    const std::string req = "GET /api/health HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    send(sock, req.data(), static_cast<int>(req.size()), 0);
    char buffer[1024]{};
    const auto received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    closeSocket(sock);
#if defined(_WIN32)
    WSACleanup();
#endif
    return received > 0 && std::string(buffer).find("\"status\":\"ok\"") != std::string::npos;
}

void printResult(const std::string& name, bool ok, bool& allOk) {
    std::cout << (ok ? "PASS " : "FAIL ") << name << "\n";
    allOk = allOk && ok;
}
} // namespace

int main() {
    using namespace ect;
    bool allOk = true;
    const auto configDir = Config::resolveConfigDir();
    printResult("config/device.ini exists", std::filesystem::exists(configDir + "/device.ini"), allOk);
    printResult("config/reminders.ini exists", std::filesystem::exists(configDir + "/reminders.ini"), allOk);
    printResult("config/contacts.ini exists", std::filesystem::exists(configDir + "/contacts.ini"), allOk);

    MockAudio audio;
    printResult("mock audio", audio.playAlert("factory test"), allOk);

    MockCamera camera(true);
    printResult("mock camera", camera.detectPresence(), allOk);

    StateMachine sm;
    sm.transitionTo(AppState::FACTORY_TEST, "factory test");
    printResult("state machine", sm.state() == AppState::FACTORY_TEST, allOk);

    HttpServer server(18080, "web");
    server.setHandler([](const HttpRequest& request) {
        if (request.path == "/api/health") {
            return HttpResponse{200, "application/json; charset=utf-8", "{\"status\":\"ok\"}"};
        }
        return HttpResponse{404, "application/json; charset=utf-8", "{}"};
    });
    const bool started = server.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    printResult("http health", started && httpHealthCheck(18080), allOk);
    server.stop();

    std::cout << (allOk ? "FACTORY TEST PASS\n" : "FACTORY TEST FAIL\n");
    return allOk ? 0 : 1;
}
