#pragma once

#include <map>
#include <string>
#include <vector>

namespace ect {

struct DeviceConfig {
    std::string deviceName{"Elder Care Terminal"};
    std::string deviceId{"ect-rk3566-001"};
    std::string platform{"rk3566"};
    bool mockMode{true};
    int displayWidth{1024};
    int displayHeight{600};
    int httpPort{8080};
    std::string cameraDevice{"TODO"};
    std::string audioDevice{"TODO"};
    std::string gpioConfirmButton{"TODO"};
    std::string gpioCallButton{"TODO"};
    std::string logLevel{"INFO"};
    bool mockPresence{true};
};

struct ReminderConfig {
    std::string title;
    std::string type;
    std::string time;
    bool enabled{true};
};

struct ContactConfig {
    std::string name;
    std::string phone;
    std::string relation;
    bool primary{false};
};

class Config {
public:
    bool load(const std::string& configDir);

    const DeviceConfig& device() const { return device_; }
    const std::vector<ReminderConfig>& reminders() const { return reminders_; }
    const std::vector<ContactConfig>& contacts() const { return contacts_; }
    const std::string& configDir() const { return configDir_; }

    static std::string resolveConfigDir();
    static std::map<std::string, std::string> readSimpleIni(const std::string& path);

private:
    DeviceConfig device_;
    std::vector<ReminderConfig> reminders_;
    std::vector<ContactConfig> contacts_;
    std::string configDir_;
};

} // namespace ect
