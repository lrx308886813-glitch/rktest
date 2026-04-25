#pragma once

#include "core/Storage.hpp"

#include <optional>

namespace ect {

class ContactService {
public:
    explicit ContactService(Storage& storage);
    std::vector<Contact> list();
    Contact add(const Contact& contact);
    std::optional<Contact> primary();

private:
    Storage& storage_;
};

} // namespace ect
