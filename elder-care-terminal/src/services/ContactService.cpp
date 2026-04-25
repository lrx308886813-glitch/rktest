#include "services/ContactService.hpp"

namespace ect {

ContactService::ContactService(Storage& storage) : storage_(storage) {}

std::vector<Contact> ContactService::list() {
    return storage_.contacts();
}

Contact ContactService::add(const Contact& contact) {
    return storage_.addContact(contact);
}

std::optional<Contact> ContactService::primary() {
    return storage_.primaryContact();
}

} // namespace ect
