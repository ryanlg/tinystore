#include <tinystore/store.hpp>

namespace tinystore {

void Store::put(std::string key, std::string value) {
    _backing.insert_or_assign(std::move(key), std::move(value));
}

std::optional<std::string> Store::get(const std::string& key) {
    const auto found = _backing.find(key);
    if (found == _backing.end())
        return std::nullopt;

    return found->second;
}

}  // namespace tinystore
