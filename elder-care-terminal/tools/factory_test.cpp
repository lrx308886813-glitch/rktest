#include "core/logger.hpp"
#include "core/state_machine.hpp"
#include "hardware/mock_hardware.hpp"

#include <arpa/inet.h>
#include <filesystem>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
bool checkHttpHealth(const std::string &host, int port) {
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        close(fd);
        return false;
    }

    const std::string req = "GET /api/health HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    send(fd, req.data(), req.size(), 0);

    char buf[2048] = {0};
    const auto n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return false;
    const std::string resp(buf, static_cast<std::size_t>(n));
    return resp.find("200 OK") != std::string::npos && resp.find("\"status\":\"ok\"") != std::string::npos;
}
} // namespace

int main() {
    bool pass = true;
    const auto root = std::filesystem::current_path() / "elder-care-terminal";

    pass &= std::filesystem::exists(root / "config/device.ini");
    pass &= std::filesystem::exists(root / "config/reminders.ini");
    pass &= std::filesystem::exists(root / "config/contacts.ini");

    ect::hardware::MockAudio audio;
    ect::hardware::MockCamera camera;
    pass &= audio.playTone("factory_beep");
    pass &= camera.captureFrame("/tmp/factory_capture.txt");

    ect::core::Logger logger((root / "data/event_log.txt").string());
    ect::core::StateMachine machine(logger);
    machine.transitionTo(ect::core::TerminalState::FACTORY_TEST, "factory_test_start");
    machine.transitionTo(ect::core::TerminalState::IDLE, "factory_test_done");

    pass &= checkHttpHealth("127.0.0.1", 8080);

    std::cout << (pass ? "PASS" : "FAIL") << std::endl;
    return pass ? 0 : 1;
}
