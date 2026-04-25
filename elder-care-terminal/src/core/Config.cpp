#include "core/Config.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace ect {

namespace {
std::string trim(const std::string& value) {
    auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
    auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    if (begin >= end) return "";
    return std::string(begin, end);
}

bool boolValue(const std::string& value, bool fallback) {
    if (value == "true" || value == "1" || value == "yes") return true;
    if (value == "false" || value == "0" || value == "no") return false;
    return fallback;
}

int intValue(const std::map<std::string, std::string>& values, const std::string& key, int fallback) {
    auto it = values.find(key);
    if (it == values.end()) return fallback;
    try {
        return std::stoi(it->second);
    } catch (...) {
        return fallback;
    }
}

std::vector<std::map<std::string, std::string>> readBlocks(const std::string& path, const std::string& marker) {
    std::ifstream in(path);
    std::vector<std::map<std::string, std::string>> blocks;
    std::map<std::string, std::string> current;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line == marker) {
            if (!current.empty()) {
                blocks.push_back(current);
                current.clear();
            }
            continue;
        }
        const auto pos = line.find('=');
        if (pos != std::string::npos) {
            current[trim(line.substr(0, pos))] = trim(line.substr(pos + 1));
        }
    }
    if (!current.empty()) {
        blocks.push_back(current);
    }
    return blocks;
}

std::string get(const std::map<std::string, std::string>& values, const std::string& key, const std::string& fallback) {
    auto it = values.find(key);
    return it == values.end() ? fallback : it->second;
}
} // namespace

bool Config::load(const std::string& configDir) {
    configDir_ = configDir;
    const auto deviceValues = readSimpleIni(configDir + "/device.ini");
    device_.deviceName = get(deviceValues, "device_name", device_.deviceName);
    device_.deviceId = get(deviceValues, "device_id", device_.deviceId);
    device_.platform = get(deviceValues, "platform", device_.platform);
    device_.mockMode = boolValue(get(deviceValues, "mock_mode", "true"), true);
    device_.displayWidth = intValue(deviceValues, "display_width", device_.displayWidth);
    device_.displayHeight = intValue(deviceValues, "display_height", device_.displayHeight);
    device_.httpPort = intValue(deviceValues, "http_port", device_.httpPort);
    device_.cameraDevice = get(deviceValues, "camera_device", device_.cameraDevice);
    device_.audioDevice = get(deviceValues, "audio_device", device_.audioDevice);
    device_.gpioConfirmButton = get(deviceValues, "gpio_confirm_button", device_.gpioConfirmButton);
    device_.gpioCallButton = get(deviceValues, "gpio_call_button", device_.gpioCallButton);
    device_.logLevel = get(deviceValues, "log_level", device_.logLevel);
    device_.mockPresence = boolValue(get(deviceValues, "mock_presence", "true"), true);

    reminders_.clear();
    for (const auto& block : readBlocks(configDir + "/reminders.ini", "[[reminder]]")) {
        ReminderConfig reminder;
        reminder.title = get(block, "title", "");
        reminder.type = get(block, "type", "CUSTOM");
        reminder.time = get(block, "time", "08:00");
        reminder.enabled = boolValue(get(block, "enabled", "true"), true);
        if (!reminder.title.empty()) reminders_.push_back(reminder);
    }

    contacts_.clear();
    for (const auto& block : readBlocks(configDir + "/contacts.ini", "[[contact]]")) {
        ContactConfig contact;
        contact.name = get(block, "name", "");
        contact.phone = get(block, "phone", "TODO");
        contact.relation = get(block, "relation", "");
        contact.primary = boolValue(get(block, "primary", "false"), false);
        if (!contact.name.empty()) contacts_.push_back(contact);
    }
    return !deviceValues.empty();
}

std::string Config::resolveConfigDir() {
    namespace fs = std::filesystem;
    if (fs::exists("config/device.ini")) return "config";
    if (fs::exists("../config/device.ini")) return "../config";
    if (fs::exists("elder-care-terminal/config/device.ini")) return "elder-care-terminal/config";
    return "config";
}

std::map<std::string, std::string> Config::readSimpleIni(const std::string& path) {
    std::ifstream in(path);
    std::map<std::string, std::string> values;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line.front() == '[') continue;
        const auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        values[trim(line.substr(0, pos))] = trim(line.substr(pos + 1));
    }
    return values;
}

} // namespace ect
