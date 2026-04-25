#pragma once

#include <string>

namespace ect {

class Rk3566HardwarePlaceholder {
public:
    explicit Rk3566HardwarePlaceholder(std::string deviceConfigPath);
    void describeTodo() const;
    bool callPrimaryViaFutureBackend(const std::string& phoneOrEndpoint);

private:
    std::string deviceConfigPath_;
};

} // namespace ect
